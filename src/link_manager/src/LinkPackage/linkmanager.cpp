/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "linkmanager.h"
#include "mavlinkprotocol.h"

#include "UDPLink.h"
#include "SerialLink.h"
#include "UdpIODevice.h"
#include "bridge.h"


#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    #include <QtCore/qapplicationstatic.h>
    Q_APPLICATION_STATIC(LinkManager, _linkManagerInstance);

#else
    #include <QtGlobal>
    Q_GLOBAL_STATIC(LinkManager, _linkManagerInstance)
#endif
#include <QtCore/QTimer>
#include <QtQml/qqml.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(LinkManagerLog, "qgc.comms.linkmanager")
Q_LOGGING_CATEGORY(LinkManagerVerboseLog, "qgc.comms.linkmanager:verbose")


LinkManager::LinkManager(QObject *parent)
    : QObject(parent)
    , _portListTimer(new QTimer(this))
#ifndef QGC_NO_SERIAL_LINK
    , _nmeaSocket(new UdpIODevice(this))
#endif
{
    // qCDebug(LinkManagerLog) << Q_FUNC_INFO << this;
}

LinkManager::~LinkManager()
{
    // qCDebug(LinkManagerLog) << Q_FUNC_INFO << this;
}

LinkManager *LinkManager::instance()
{
    return _linkManagerInstance();
}


void LinkManager::init()
{

    (void) connect(_portListTimer, &QTimer::timeout, this, &LinkManager::_updateAutoConnectLinks);
    _portListTimer->start(_autoconnectUpdateTimerMSecs); // timeout must be long enough to get past bootloader on second pass
    Bridge::instance()->init();

}



void LinkManager::createConnectedLink(const LinkConfiguration *config)
{

    for (SharedLinkConfigurationPtr &sharedConfig : _rgLinkConfigs) {
        if (sharedConfig.get() == config) {
            createConnectedLink(sharedConfig);
        }
    }
}

bool LinkManager::createConnectedLink(SharedLinkConfigurationPtr &config)
{

    SharedLinkInterfacePtr link = nullptr;



    switch(config->type()) {
    case LinkConfiguration::TypeSerial:
        link = std::make_shared<SerialLink>(config);
        break;
    case LinkConfiguration::TypeUdp:
        link = std::make_shared<UDPLink>(config);
        break;
    case LinkConfiguration::TypeLast:
    default:
        break;
    }

    if (!link) {
        return false;
    }
    SharedLinkConfigurationPtr conf = link->linkConfiguration();

    if (!link->_allocateMavlinkChannel()) {
        qCWarning(LinkManagerLog) << "Link failed to setup mavlink channels";
        return false;
    }

    _rgLinks.append(link);
    config->setLink(link);

    (void) connect(link.get(), &LinkInterface::communicationError, this, &LinkManager::_communicationError);
    (void) connect(link.get(), &LinkInterface::bytesReceived, MAVLinkProtocol::instance(), &MAVLinkProtocol::receiveBytes);
    (void) connect(link.get(), &LinkInterface::disconnected, this, &LinkManager::_linkDisconnected);

    MAVLinkProtocol::instance()->resetMetadataForLink(link.get());

    if (!link->_connect()) {
        link->_freeMavlinkChannel();
        _rgLinks.removeAt(_rgLinks.indexOf(link));
        config->setLink(nullptr);
        return false;
    }

    return true;
}

void LinkManager::_communicationError(const QString &title, const QString &error)
{
    //qgcApp()->showAppMessage(error, title);
}

SharedLinkInterfacePtr LinkManager::mavlinkPixhawkLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if (linkConfig->type() == LinkConfiguration::TypeSerial) {
            return link;
        }
    }

    return nullptr;
}

SharedLinkInterfacePtr LinkManager::mavlinkForwardingLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _mavlinkForwardingLinkName)) {
            return link;
        }
    }

    return nullptr;
}

SharedLinkInterfacePtr LinkManager::mavlinkAutoconnectLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _defaultUDPLinkName)) {
            return link;
        }
    }

    return nullptr;
}

SharedLinkInterfacePtr LinkManager::mavlinkPrimaryUDPLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _defaultPrimaryUDPLinkName)) {
            return link;
        }
    }

    return nullptr;
}

SharedLinkInterfacePtr LinkManager::mavlinkSecondaryUDPLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _defaultSecondaryUDPLinkName)) {
            return link;
        }
    }

    return nullptr;
}

SharedLinkInterfacePtr LinkManager::mavlinkForwardingSupportLink()
{
    for (SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _mavlinkForwardingSupportLinkName)) {
            return link;
        }
    }

    return nullptr;
}

void LinkManager::disconnectAll()
{
    const QList<SharedLinkInterfacePtr> links = _rgLinks;
    for (const SharedLinkInterfacePtr &sharedLink: links) {
        sharedLink->disconnect();
    }
}

void LinkManager::_linkDisconnected()
{
    LinkInterface* const link = qobject_cast<LinkInterface*>(sender());

    if (!link || !containsLink(link)) {
        return;
    }

    (void) disconnect(link, &LinkInterface::bytesReceived, MAVLinkProtocol::instance(), &MAVLinkProtocol::receiveBytes);
    //(void) disconnect(link, &LinkInterface::bytesSent, MAVLinkProtocol::instance(), &MAVLinkProtocol::logSentBytes);
    (void) disconnect(link, &LinkInterface::disconnected, this, &LinkManager::_linkDisconnected);

    link->_freeMavlinkChannel();

    for (auto it = _rgLinks.begin(); it != _rgLinks.end(); ++it) {
        if (it->get() == link) {
            qCDebug(LinkManagerLog) << Q_FUNC_INFO << it->get()->linkConfiguration()->name() << it->use_count();
            (void) _rgLinks.erase(it);
            return;
        }
    }
}

SharedLinkInterfacePtr LinkManager::sharedLinkInterfacePointerForLink(const LinkInterface *link)
{
    for (SharedLinkInterfacePtr &sharedLink: _rgLinks) {
        if (sharedLink.get() == link) {
            return sharedLink;
        }
    }

    qCWarning(LinkManagerLog) << "returning nullptr";
    return SharedLinkInterfacePtr(nullptr);
}

bool LinkManager::_connectionsSuspendedMsg() const
{
    if (_connectionsSuspended) {
        //qgcApp()->showAppMessage(tr("Connect not allowed: %1").arg(_connectionsSuspendedReason));
        return true;
    }

    return false;
}

void LinkManager::saveLinkConfigurationList()
{
    QSettings settings;
    settings.remove(LinkConfiguration::settingsRoot());

    int trueCount = 0;
    for (int i = 0; i < _rgLinkConfigs.count(); i++) {
        SharedLinkConfigurationPtr linkConfig = _rgLinkConfigs[i];
        if (!linkConfig) {
            qCWarning(LinkManagerLog) << "Internal error for link configuration in LinkManager";
            continue;
        }

        if (linkConfig->isDynamic()) {
            continue;
        }
        qDebug()<<linkConfig->name();
        const QString root = LinkConfiguration::settingsRoot() + QStringLiteral("/Link%1").arg(trueCount++);
        settings.setValue(root + "/name", linkConfig->name());
        settings.setValue(root + "/type", linkConfig->type());
        settings.setValue(root + "/auto", linkConfig->isAutoConnect());
        settings.setValue(root + "/high_latency", linkConfig->isHighLatency());
        linkConfig->saveSettings(settings, root);
    }

    const QString root = QString(LinkConfiguration::settingsRoot());
    settings.setValue(root + "/count", trueCount);
}

void LinkManager::loadLinkConfigurationList()
{
    QSettings settings;
    // Is the group even there?
    if (settings.contains(LinkConfiguration::settingsRoot() + "/count")) {
        // Find out how many configurations we have
        const int count = settings.value(LinkConfiguration::settingsRoot() + "/count").toInt();
        for (int i = 0; i < count; i++) {
            const QString root = LinkConfiguration::settingsRoot() + QStringLiteral("/Link%1").arg(i);
            if (!settings.contains(root + "/type")) {
                qCWarning(LinkManagerLog) << "Link Configuration" << root << "has no type.";
                continue;
            }

            LinkConfiguration::LinkType type = static_cast<LinkConfiguration::LinkType>(settings.value(root + "/type").toInt());
            if (type >= LinkConfiguration::TypeLast) {
                qCWarning(LinkManagerLog) << "Link Configuration" << root << "an invalid type:" << type;
                continue;
            }

            if (!settings.contains(root + "/name")) {
                qCWarning(LinkManagerLog) << "Link Configuration" << root << "has no name.";
                continue;
            }

            const QString name = settings.value(root + "/name").toString();
            if (name.isEmpty()) {
                qCWarning(LinkManagerLog) << "Link Configuration" << root << "has an empty name.";
                continue;
            }

            LinkConfiguration* link = nullptr;
            switch(type) {
            case LinkConfiguration::TypeSerial:
                link = new SerialConfiguration(name);
                break;
            case LinkConfiguration::TypeUdp:
                link = new UDPConfiguration(name);
                break;
            case LinkConfiguration::TypeLast:
            default:
                break;
            }

            if (link) {
                const bool autoConnect = settings.value(root + "/auto").toBool();
                link->setAutoConnect(autoConnect);
                const bool highLatency = settings.value(root + "/high_latency").toBool();
                link->setHighLatency(highLatency);
                link->loadSettings(settings, root);
                addConfiguration(link);
            }
        }
    }

    // Enable automatic Serial PX4/3DR Radio hunting
    _configurationsLoaded = true;
}

void LinkManager::_addUDPAutoConnectLink()
{

    for (SharedLinkInterfacePtr &link : _rgLinks) {

        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();

        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _defaultPrimaryUDPLinkName)) {
            return;
        }
    }

    qCDebug(LinkManagerLog) << "New auto-connect UDP port added";
    UDPConfiguration* const udpConfig = new UDPConfiguration(_defaultPrimaryUDPLinkName);
    udpConfig->setDynamic(true);
    udpConfig->setAutoConnect(true);
    udpConfig->setLocalPort(14560);

    SharedLinkConfigurationPtr config = addConfiguration(udpConfig);
    createConnectedLink(config);

    UDPConfiguration* const udpConfig2= new UDPConfiguration(_defaultSecondaryUDPLinkName);
    udpConfig2->setDynamic(true);
    udpConfig2->setAutoConnect(true);
    udpConfig2->setLocalPort(14561);
    SharedLinkConfigurationPtr config2 = addConfiguration(udpConfig2);
    createConnectedLink(config2);
    Bridge::instance()->addUdpLinks(udpConfig->link(), udpConfig2->link());


}

void LinkManager::_addMAVLinkForwardingLink()
{


    for (const SharedLinkInterfacePtr &link : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = link->linkConfiguration();
        if ((linkConfig->type() == LinkConfiguration::TypeUdp) && (linkConfig->name() == _mavlinkForwardingLinkName)) {
            // TODO: should we check if the host/port matches the mavlinkForwardHostName setting and update if it does not match?
            return;
        }
    }

    const QString hostName = QString("localhost:14445");
    _createDynamicForwardLink(_mavlinkForwardingLinkName, hostName);
}



void LinkManager::_updateAutoConnectLinks()
{

    if (_connectionsSuspended) {
        return;
    }
    // disable autoconnect link and forward link
    _addUDPAutoConnectLink();

    //_addMAVLinkForwardingLink();
#ifdef QGC_ZEROCONF_ENABLED
    _addZeroConfAutoConnectLink();
#endif

    // check to see if nmea gps is configured for UDP input, if so, set it up to connect

        _nmeaSocket->close();


#ifndef QGC_NO_SERIAL_LINK
    _addSerialAutoConnectLink();
#endif

}

void LinkManager::shutdown()
{
    setConnectionsSuspended(tr("Shutdown"));
    disconnectAll();

    // Wait for all the vehicles to go away to ensure an orderly shutdown and deletion of all objects
    //while (MultiVehicleManager::instance()->vehicles()->count()) {
    //    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    //}
}

QStringList LinkManager::linkTypeStrings() const
{
    //-- Must follow same order as enum LinkType in LinkConfiguration.h
    static QStringList list;
    if (!list.isEmpty()) {
        return list;
    }

    list += tr("Serial");
    list += tr("UDP");


    if (list.size() != static_cast<int>(LinkConfiguration::TypeLast)) {
        qCWarning(LinkManagerLog) << "Internal error";
    }

    return list;
}

void LinkManager::endConfigurationEditing(LinkConfiguration *config, LinkConfiguration *editedConfig)
{
    if (!config || !editedConfig) {
        qCWarning(LinkManagerLog) << "Internal error";
        return;
    }

    config->copyFrom(editedConfig);
    saveLinkConfigurationList();
    emit config->nameChanged(config->name());
    // Discard temporary duplicate
    delete editedConfig;
}

void LinkManager::endCreateConfiguration(LinkConfiguration *config)
{
    if (!config) {
        qCWarning(LinkManagerLog) << "Internal error";
        return;
    }

    addConfiguration(config);
    saveLinkConfigurationList();
}

LinkConfiguration *LinkManager::createConfiguration(int type, const QString &name)
{
#ifndef QGC_NO_SERIAL_LINK
    if (static_cast<LinkConfiguration::LinkType>(type) == LinkConfiguration::TypeSerial) {
        _updateSerialPorts();
    }
#endif

    return LinkConfiguration::createSettings(type, name);
}

LinkConfiguration *LinkManager::startConfigurationEditing(LinkConfiguration *config)
{
    if (!config) {
        qCWarning(LinkManagerLog) << "Internal error";
        return nullptr;
    }

#ifndef QGC_NO_SERIAL_LINK
    if (config->type() == LinkConfiguration::TypeSerial) {
        _updateSerialPorts();
    }
#endif

    return LinkConfiguration::duplicateSettings(config);
}

void LinkManager::removeConfiguration(LinkConfiguration *config)
{
    if (!config) {
        qCWarning(LinkManagerLog) << "Internal error";
        return;
    }

    LinkInterface* const link = config->link();
    if (link) {
        link->disconnect();
    }

    _removeConfiguration(config);
    saveLinkConfigurationList();
}

void LinkManager::createMavlinkForwardingSupportLink()
{
    const QString hostName = "support.ardupilot.org:xxxx";
    _createDynamicForwardLink(_mavlinkForwardingSupportLinkName, hostName);
    _mavlinkSupportForwardingEnabled = true;
    emit mavlinkSupportForwardingEnabledChanged();
}


void LinkManager::_removeConfiguration(const LinkConfiguration *config)
{

    for (auto it = _rgLinkConfigs.begin(); it != _rgLinkConfigs.end(); ++it) {
        if (it->get() == config) {
            (void) _rgLinkConfigs.erase(it);
            return;
        }
    }

    qCWarning(LinkManagerLog) << Q_FUNC_INFO << "called with unknown config";
}


bool LinkManager::containsLink(const LinkInterface *link) const
{
    for (const SharedLinkInterfacePtr &sharedLink : _rgLinks) {
        if (sharedLink.get() == link) {
            return true;
        }
    }

    return false;
}

SharedLinkConfigurationPtr LinkManager::addConfiguration(LinkConfiguration *config)
{
    (void) _rgLinkConfigs.append(SharedLinkConfigurationPtr(config));

    return _rgLinkConfigs.last();
}

void LinkManager::startAutoConnectedLinks()
{
    for (SharedLinkConfigurationPtr &sharedConfig : _rgLinkConfigs) {
        if (sharedConfig->isAutoConnect()) {
            createConnectedLink(sharedConfig);
        }
    }
}

uint8_t LinkManager::allocateMavlinkChannel()
{
    for (uint8_t mavlinkChannel = 0; mavlinkChannel < MAVLINK_COMM_NUM_BUFFERS; mavlinkChannel++) {
        if (_mavlinkChannelsUsedBitMask & (1 << mavlinkChannel)) {
            continue;
        }

        mavlink_reset_channel_status(mavlinkChannel);
        mavlink_status_t* const mavlinkStatus = mavlink_get_channel_status(mavlinkChannel);
        mavlinkStatus->flags |= 0;
        _mavlinkChannelsUsedBitMask |= (1 << mavlinkChannel);
        qCDebug(LinkManagerLog) << "allocateMavlinkChannel" << mavlinkChannel;
        return mavlinkChannel;
    }

    qWarning(LinkManagerLog) << "allocateMavlinkChannel: all channels reserved!";
    return invalidMavlinkChannel();
}

void LinkManager::freeMavlinkChannel(uint8_t channel)
{
    qCDebug(LinkManagerLog) << "freeMavlinkChannel" << channel;

    if (invalidMavlinkChannel() == channel) {
        return;
    }

    _mavlinkChannelsUsedBitMask &= ~(1 << channel);
}



void LinkManager::_createDynamicForwardLink(const char *linkName, const QString &hostName)
{
    UDPConfiguration* const udpConfig = new UDPConfiguration(linkName);

    udpConfig->setDynamic(true);
    udpConfig->setForwarding(true);
    udpConfig->addHost(hostName);

    SharedLinkConfigurationPtr config = addConfiguration(udpConfig);
    createConnectedLink(config);

    qCDebug(LinkManagerLog) << "New dynamic MAVLink forwarding port added:" << linkName << " hostname:" << hostName;
}

bool LinkManager::isLinkUSBDirect(const LinkInterface *link)
{
#ifndef QGC_NO_SERIAL_LINK
    const SerialLink* const serialLink = qobject_cast<const SerialLink*>(link);
    if (!serialLink) {
        return false;
    }

    const SharedLinkConfigurationPtr config = serialLink->linkConfiguration();
    if (!config) {
        return false;
    }

    const SerialConfiguration* const serialConfig = qobject_cast<const SerialConfiguration*>(config.get());
    if (serialConfig && serialConfig->usbDirect()) {
        return link;
    }
#endif

    return false;
}

void LinkManager::resetMavlinkSigning()
{
    for (const SharedLinkInterfacePtr &sharedLink: _rgLinks) {
        //sharedLink->initMavlinkSigning();
    }
}

void LinkManager::_filterCompositePorts(QList<QGCSerialPortInfo> &portList)
{
    typedef QPair<quint16, quint16> VidPidPair_t;

    QMap<VidPidPair_t, QStringList> seenSerialNumbers;

    for (auto it = portList.begin(); it != portList.end();) {
        const QGCSerialPortInfo &portInfo = *it;
        if (portInfo.hasVendorIdentifier() && portInfo.hasProductIdentifier() && !portInfo.serialNumber().isEmpty() && portInfo.serialNumber() != "0") {
            VidPidPair_t vidPid(portInfo.vendorIdentifier(), portInfo.productIdentifier());
            if (seenSerialNumbers.contains(vidPid) && seenSerialNumbers[vidPid].contains(portInfo.serialNumber())) {
                // Some boards are a composite USB device, with the first port being mavlink and the second something else. We only expose to first mavlink port.
                // However internal NMEA devices can present like this, so dont skip anything with NMEA in description
                if(!portInfo.description().contains("NMEA")) {
                    //qCDebug(LinkManagerVerboseLog) << QStringLiteral("Removing secondary port on same device - port:%1 vid:%2 pid%3 sn:%4").arg(portInfo.portName()).arg(portInfo.vendorIdentifier()).arg(portInfo.productIdentifier()).arg(portInfo.serialNumber()) << Q_FUNC_INFO;
                    it = portList.erase(it);
                    continue;
                }
            }
            seenSerialNumbers[vidPid].append(portInfo.serialNumber());
        }
        it++;
    }
}

#ifndef QGC_NO_SERIAL_LINK // Serial Only Functions

void LinkManager::_addSerialAutoConnectLink()
{
    QList<QGCSerialPortInfo> portList;
#ifdef Q_OS_ANDROID
    // Android builds only support a single serial connection. Repeatedly calling availablePorts after that one serial
    // port is connected leaks file handles due to a bug somewhere in android serial code. In order to work around that
    // bug after we connect the first serial port we stop probing for additional ports.
    if (!_isSerialPortConnected()) {
        portList = QGCSerialPortInfo::availablePorts();
    }
#else
    portList = QGCSerialPortInfo::availablePorts();
#endif

    _filterCompositePorts(portList);

    QStringList currentPorts;
    for (const QGCSerialPortInfo &portInfo: portList) {
        /*
        qCDebug(LinkManagerVerboseLog) << "-----------------------------------------------------";
        qCDebug(LinkManagerVerboseLog) << "portName:          " << portInfo.portName();
        qCDebug(LinkManagerVerboseLog) << "systemLocation:    " << portInfo.systemLocation();
        qCDebug(LinkManagerVerboseLog) << "description:       " << portInfo.description();
        qCDebug(LinkManagerVerboseLog) << "manufacturer:      " << portInfo.manufacturer();
        qCDebug(LinkManagerVerboseLog) << "serialNumber:      " << portInfo.serialNumber();
        qCDebug(LinkManagerVerboseLog) << "vendorIdentifier:  " << portInfo.vendorIdentifier();
        qCDebug(LinkManagerVerboseLog) << "productIdentifier: " << portInfo.productIdentifier();
*/

        currentPorts << portInfo.systemLocation();

        QGCSerialPortInfo::BoardType_t boardType;
        QString boardName;

        // check to see if nmea gps is configured for current Serial port, if so, set it up to connect
        if (portInfo.getBoardInfo(boardType, boardName)) {
            // Should we be auto-connecting to this board type?
            if (!_allowAutoConnectToBoard(boardType)) {
                continue;
            }

            if (portInfo.isBootloader()) {
                // Don't connect to bootloader
                qCDebug(LinkManagerLog) << "Waiting for bootloader to finish" << portInfo.systemLocation();
                continue;
            }
            if (_portAlreadyConnected(portInfo.systemLocation()) || (_autoConnectRTKPort == portInfo.systemLocation())) {
                //qCDebug(LinkManagerVerboseLog) << "Skipping existing autoconnect" << portInfo.systemLocation();
            } else if (!_autoconnectPortWaitList.contains(portInfo.systemLocation())) {
                // We don't connect to the port the first time we see it. The ability to correctly detect whether we
                // are in the bootloader is flaky from a cross-platform standpoint. So by putting it on a wait list
                // and only connect on the second pass we leave enough time for the board to boot up.
                //qCDebug(LinkManagerLog) << "Waiting for next autoconnect pass" << portInfo.systemLocation() << boardName;
                _autoconnectPortWaitList[portInfo.systemLocation()] = 1;
            } else if ((++_autoconnectPortWaitList[portInfo.systemLocation()] * _autoconnectUpdateTimerMSecs) > _autoconnectConnectDelayMSecs) {
                SerialConfiguration* pSerialConfig = nullptr;
                _autoconnectPortWaitList.remove(portInfo.systemLocation());
                switch (boardType) {
                case QGCSerialPortInfo::BoardTypePixhawk:
                    pSerialConfig = new SerialConfiguration(tr("%1 on %2 (AutoConnect)").arg(boardName, portInfo.portName().trimmed()));
                    pSerialConfig->setUsbDirect(true);
                    break;
                case QGCSerialPortInfo::BoardTypeSiKRadio:
                    //pSerialConfig = new SerialConfiguration(tr("%1 on %2 (AutoConnect)").arg(boardName, portInfo.portName().trimmed()));
                    break;
                case QGCSerialPortInfo::BoardTypeOpenPilot:
                    //pSerialConfig = new SerialConfiguration(tr("%1 on %2 (AutoConnect)").arg(boardName, portInfo.portName().trimmed()));
                    break;
                case QGCSerialPortInfo::BoardTypeRTKGPS:
                    //qCDebug(LinkManagerLog) << "RTK GPS auto-connected" << portInfo.portName().trimmed();
                    //_autoConnectRTKPort = portInfo.systemLocation();
                    //GPSManager::instance()->gpsRtk()->connectGPS(portInfo.systemLocation(), boardName);
                    break;
                default:
                    qCWarning(LinkManagerLog) << "Internal error: Unknown board type" << boardType;
                    continue;
                }

                if (pSerialConfig) {
                    qCDebug(LinkManagerLog) << "New auto-connect port added: " << pSerialConfig->name() << portInfo.systemLocation();
                    pSerialConfig->setBaud((boardType == QGCSerialPortInfo::BoardTypeSiKRadio) ? 57600 : 115200);
                    pSerialConfig->setDynamic(true);
                    pSerialConfig->setPortName(portInfo.systemLocation());
                    pSerialConfig->setAutoConnect(true);

                    SharedLinkConfigurationPtr sharedConfig(pSerialConfig);
                    createConnectedLink(sharedConfig);
                    Bridge::instance()->addPixhawkSerialLink(pSerialConfig->link());
                }
            }
        }
    }

    // Check for RTK GPS connection gone
    if (!_autoConnectRTKPort.isEmpty() && !currentPorts.contains(_autoConnectRTKPort)) {
        qCDebug(LinkManagerLog) << "RTK GPS disconnected" << _autoConnectRTKPort;
        //GPSManager::instance()->gpsRtk()->disconnectGPS();
        _autoConnectRTKPort.clear();
    }
}

bool LinkManager::_allowAutoConnectToBoard(QGCSerialPortInfo::BoardType_t boardType) const
{
    switch (boardType) {
    case QGCSerialPortInfo::BoardTypePixhawk:
        return true;
        break;
    case QGCSerialPortInfo::BoardTypeSiKRadio:
        return true;
        break;
    case QGCSerialPortInfo::BoardTypeOpenPilot:
        return true;
        break;
    case QGCSerialPortInfo::BoardTypeRTKGPS:
        return true;
        break;
    default:
        qCWarning(LinkManagerLog) << "Internal error: Unknown board type" << boardType;
        return false;
    }

    return false;
}

bool LinkManager::_portAlreadyConnected(const QString &portName) const
{
    const QString searchPort = portName.trimmed();
    for (const SharedLinkInterfacePtr &linkInterface : _rgLinks) {
        const SharedLinkConfigurationPtr linkConfig = linkInterface->linkConfiguration();
        const SerialConfiguration* const serialConfig = qobject_cast<const SerialConfiguration*>(linkConfig.get());
        if (serialConfig && (serialConfig->portName() == searchPort)) {
            return true;
        }
    }

    return false;
}

void LinkManager::_updateSerialPorts()
{
    _commPortList.clear();
    _commPortDisplayList.clear();
    const QList<QGCSerialPortInfo> portList = QGCSerialPortInfo::availablePorts();
    for (const QGCSerialPortInfo &info: portList) {
        const QString port = info.systemLocation().trimmed();
        _commPortList += port;
        _commPortDisplayList += SerialConfiguration::cleanPortDisplayName(port);
    }
}

QStringList LinkManager::serialPortStrings()
{
    if (_commPortDisplayList.isEmpty()) {
        _updateSerialPorts();
    }

    return _commPortDisplayList;
}

QStringList LinkManager::serialPorts()
{
    if (_commPortList.isEmpty()) {
        _updateSerialPorts();
    }

    return _commPortList;
}

QStringList LinkManager::serialBaudRates()
{
    return SerialConfiguration::supportedBaudRates();
}

bool LinkManager::_isSerialPortConnected() const
{
    for (const SharedLinkInterfacePtr &link: _rgLinks) {
        if (qobject_cast<const SerialLink*>(link.get())) {
            return true;
        }
    }

    return false;
}

#endif // QGC_NO_SERIAL_LINK
