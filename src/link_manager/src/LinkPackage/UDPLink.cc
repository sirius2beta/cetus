/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "UDPLink.h"


#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkDatagram>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QUdpSocket>
Q_LOGGING_CATEGORY(UDPLinkLog, "UDPLinkLog")


namespace {
    constexpr int BUFFER_TRIGGER_SIZE = 10 * 1024;
    constexpr int RECEIVE_TIME_LIMIT_MS = 50;

    bool containsTarget(const QList<std::shared_ptr<UDPClient>> &list, const QHostAddress &address, quint16 port)
    {
        for (const std::shared_ptr<UDPClient> &target : list) {
            if ((target->address == address) && (target->port == port)) {
                return true;
            }
        }

        return false;
    }
}

/*===========================================================================*/

UDPConfiguration::UDPConfiguration(const QString &name, QObject *parent)
    : LinkConfiguration(name, parent)
{
}

UDPConfiguration::UDPConfiguration(const UDPConfiguration *source, QObject *parent)
    : LinkConfiguration(source, parent)
{
    // qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;

    UDPConfiguration::copyFrom(source);
}

UDPConfiguration::~UDPConfiguration()
{
    _targetHosts.clear();

    // qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

void UDPConfiguration::copyFrom(const LinkConfiguration *source)
{
    Q_ASSERT(source);
    LinkConfiguration::copyFrom(source);

    const UDPConfiguration *const udpSource = qobject_cast<const UDPConfiguration*>(source);
    Q_ASSERT(udpSource);

    setLocalPort(udpSource->localPort());
    _targetHosts.clear();

    for (const std::shared_ptr<UDPClient> &target : udpSource->targetHosts()) {
        if (!containsTarget(_targetHosts, target->address, target->port)) {
            _targetHosts.append(std::make_shared<UDPClient>(target.get()));
        }
    }
}

void UDPConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);

    setLocalPort(static_cast<quint16>(settings.value("port", "5000").toUInt()));

    _targetHosts.clear();
    const qsizetype hostCount = settings.value("hostCount", 0).toUInt();
    for (qsizetype i = 0; i < hostCount; i++) {
        const QString hkey = QStringLiteral("host%1").arg(i);
        const QString pkey = QStringLiteral("port%1").arg(i);
        if (settings.contains(hkey) && settings.contains(pkey)) {
            addHost(settings.value(hkey).toString(), settings.value(pkey).toUInt());
        }
    }

    settings.endGroup();
}

void UDPConfiguration::saveSettings(QSettings &settings, const QString &root) const
{
    settings.beginGroup(root);

    settings.setValue(QStringLiteral("hostCount"), _targetHosts.size());
    settings.setValue(QStringLiteral("port"), _localPort);

    for (qsizetype i = 0; i < _targetHosts.size(); i++) {
        const std::shared_ptr<UDPClient> target = _targetHosts.at(i);
        const QString hkey = QStringLiteral("host%1").arg(i);
        settings.setValue(hkey, target->address.toString());
        const QString pkey = QStringLiteral("port%1").arg(i);
        settings.setValue(pkey, target->port);
    }

    settings.endGroup();
}

void UDPConfiguration::addHost(const QString &host)
{
    if (host.contains(":")) {
        const QStringList hostInfo = host.split(":");
        if (hostInfo.size() != 2) {
            qCWarning(UDPLinkLog) << "Invalid host format:" << host;
            return;
        }

        const QString address = hostInfo.constFirst();
        const quint16 port = hostInfo.constLast().toUInt();

        addHost(address, port);
    } else {
        addHost(host, _localPort);
    }
}

void UDPConfiguration::addHost(const QString &host, quint16 port)
{
    
    const QString ipAdd = _getIpAddress(host);
    if (ipAdd.isEmpty()) {
        qCWarning(UDPLinkLog) << "Could not resolve host:" << host << "port:" << port;
        return;
    }

    const QHostAddress address(ipAdd);
    // only add if it's not already in the list
    if(_targetHosts.size() >= 1){
        _targetHosts[0].get()->address = address;
        _targetHosts[0].get()->port = port;
        return;
    }else{
        _targetHosts.append(std::make_shared<UDPClient>(address, port));
    }
}

void UDPConfiguration::removeHost(const QString &host)
{
    if (host.contains(":")) {
        const QStringList hostInfo = host.split(":");
        if (hostInfo.size() != 2) {
            qCWarning(UDPLinkLog) << "Invalid host format:" << host;
            return;
        }

        const QHostAddress address = QHostAddress(_getIpAddress(hostInfo.constFirst()));
        const quint16 port = hostInfo.constLast().toUInt();

        if (!containsTarget(_targetHosts, address, port)) {
            qCWarning(UDPLinkLog) << "Could not remove unknown host:" << host << "port:" << port;
            return;
        }

        for (qsizetype i = 0; i < _targetHosts.size(); ++i) {
            const std::shared_ptr<UDPClient> &target = _targetHosts[i];
            if (target->address == address && target->port == port) {
                _targetHosts.removeAt(i);
                return;
            }
        }
    } else {
        removeHost(host, _localPort);
    }
}

void UDPConfiguration::removeHost(const QString &host, quint16 port)
{
    const QString ipAdd = _getIpAddress(host);
    if (ipAdd.isEmpty()) {
        qCWarning(UDPLinkLog) << "Could not resolve host:" << host << "port:" << port;
        return;
    }

    const QHostAddress address(ipAdd);
    if (!containsTarget(_targetHosts, address, port)) {
        qCWarning(UDPLinkLog) << "Could not remove unknown host:" << host << "port:" << port;
        return;
    }

    for (qsizetype i = 0; i < _targetHosts.size(); ++i) {
        const std::shared_ptr<UDPClient> &target = _targetHosts[i];
        if (target->address == address && target->port == port) {
            _targetHosts.removeAt(i);
            return;
        }
    }
}


QString UDPConfiguration::_getIpAddress(const QString &address)
{
    const QHostAddress host(address);
    if (!host.isNull()) {
        return address;
    }

    const QHostInfo info = QHostInfo::fromName(address);
    if (info.error() != QHostInfo::NoError) {
        return QString();
    }

    const QList<QHostAddress> hostAddresses = info.addresses();
    for (const QHostAddress &hostAddress : hostAddresses) {
        if (hostAddress.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
            return hostAddress.toString();
        }
    }

    return QString();
}

/*===========================================================================*/

const QHostAddress UDPWorker::_multicastGroup = QHostAddress(QStringLiteral("224.0.0.1"));

UDPWorker::UDPWorker(UDPConfiguration *config, QObject *parent)
    : QObject(parent)
    , _udpConfig(config)
{
    // qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

UDPWorker::~UDPWorker()
{
    disconnectLink();

    // qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

bool UDPWorker::isConnected() const
{
    return (_socket && _socket->isValid() && _isConnected);
}

void UDPWorker::setupSocket()
{
    Q_ASSERT(!_socket);
    _socket = new QUdpSocket(this);

    const QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        _localAddresses = QSet(localAddresses.constBegin(), localAddresses.constEnd());
    #else
        _localAddresses = localAddresses.toSet();
    #endif

    _socket->setProxy(QNetworkProxy::NoProxy);

    (void) connect(_socket, &QUdpSocket::connected, this, &UDPWorker::_onSocketConnected);
    (void) connect(_socket, &QUdpSocket::disconnected, this, &UDPWorker::_onSocketDisconnected);
    (void) connect(_socket, &QUdpSocket::readyRead, this, &UDPWorker::_onSocketReadyRead);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        (void) connect(_socket, &QUdpSocket::errorOccurred, this, &UDPWorker::_onSocketErrorOccurred);
    #else
        // Qt 5 语法：由于 error 是重载的，需要指定函数指针类型
        (void) connect(_socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &UDPWorker::_onSocketErrorOccurred);
    #endif
    (void) connect(_socket, &QUdpSocket::stateChanged, this, [this](QUdpSocket::SocketState state) {
        qCDebug(UDPLinkLog) << "UDP State Changed:" << state;
        switch (state) {
        case QAbstractSocket::BoundState:
            _onSocketConnected();
            break;
        case QAbstractSocket::ClosingState:
        case QAbstractSocket::UnconnectedState:
            _onSocketDisconnected();
            break;
        default:
            break;
        }
    });

    if (UDPLinkLog().isDebugEnabled()) {
        // (void) connect(_socket, &QUdpSocket::bytesWritten, this, &UDPWorker::_onSocketBytesWritten);

        (void) QObject::connect(_socket, &QUdpSocket::hostFound, this, []() {
            qCDebug(UDPLinkLog) << "UDP Host Found";
        });
    }
}

void UDPWorker::connectLink()
{
    if (isConnected()) {
        qCWarning(UDPLinkLog) << "Already connected to" << _udpConfig->localPort();
        return;
    }

    _errorEmitted = false;

    qCDebug(UDPLinkLog) << "Attempting to bind to port:" << _udpConfig->localPort();
    const bool bindSuccess = _socket->bind(QHostAddress::AnyIPv4, _udpConfig->localPort(), QAbstractSocket::ReuseAddressHint | QAbstractSocket::ShareAddress);
    if (!bindSuccess) {
        qCWarning(UDPLinkLog) << "Failed to bind UDP socket to port" << _udpConfig->localPort();

        if (!_errorEmitted) {
            emit errorOccurred(tr("Failed to bind UDP socket to port"));
            _errorEmitted = true;
        }

        // Disconnecting here on autoconnect will cause continuous error popups
        /*if (!_udpConfig->isAutoConnect()) {
            _onSocketDisconnected();
        }*/

        return;
    }

    qCDebug(UDPLinkLog) << "Attempting to join multicast group:" << _multicastGroup.toString();
    const bool joinSuccess = _socket->joinMulticastGroup(_multicastGroup);
    if (!joinSuccess) {
        qCWarning(UDPLinkLog) << "Failed to join multicast group" << _multicastGroup.toString();
    }else{
        qCDebug(UDPLinkLog) << "Joined multicast group:" << _multicastGroup.toString();
    }

#ifdef QGC_ZEROCONF_ENABLED
    _registerZeroconf(_udpConfig->localPort());
#endif
}

void UDPWorker::disconnectLink()
{
#ifdef QGC_ZEROCONF_ENABLED
    _deregisterZeroconf();
#endif

    if (isConnected()) {
        (void) _socket->leaveMulticastGroup(_multicastGroup);
        _socket->close();
    }

    _sessionTargets.clear();
}

void UDPWorker::writeData(const QByteArray &data)
{
    if (!isConnected()) {
        emit errorOccurred(tr("Could Not Send Data - Link is Disconnected!"));
        return;
    }

    QMutexLocker locker(&_sessionTargetsMutex);


    // Send to all connected systems
    for (const std::shared_ptr<UDPClient> &target: _sessionTargets) {
        if (_socket->writeDatagram(data, target->address, target->port) < 0) {
            qCWarning(UDPLinkLog) << "Could Not Send Data - Write Failed!";
        }
    }

    locker.unlock();

    emit dataSent(data);
}

void UDPWorker::_onSocketConnected()
{
    qCDebug(UDPLinkLog) << "UDP connected to" << _udpConfig->localPort();
    _isConnected = true;
    _errorEmitted = false;
    emit connected();
}

void UDPWorker::_onSocketDisconnected()
{
    qCDebug(UDPLinkLog) << "UDP disconnected from" << _udpConfig->localPort();
    _isConnected = false;
    _errorEmitted = false;
    emit disconnected();
}

void UDPWorker::_onSocketReadyRead()
{
    if (!isConnected()) {
        emit errorOccurred(tr("Could Not Read Data - Link is Disconnected!"));
        return;
    }

    const qint64 byteCount = _socket->pendingDatagramSize();

    if (byteCount <= 0) {
        _socket->receiveDatagram();
        //qDebug()<<"Could Not Read Data - No Data Available!";

        return;

    }

    while (_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagramIn = _socket->receiveDatagram();
        if (datagramIn.isNull() || datagramIn.data().isEmpty()) continue;

        // --- 核心修正：直接發送，不緩衝 ---
        emit dataReceived(datagramIn.data());

        // 更新 Session Targets
        const QHostAddress senderAddress = (datagramIn.senderAddress().isLoopback() || _localAddresses.contains(datagramIn.senderAddress()))
                                               ? QHostAddress(QHostAddress::LocalHost)
                                               : datagramIn.senderAddress();

        QMutexLocker locker(&_sessionTargetsMutex);
        if (!containsTarget(_sessionTargets, senderAddress, datagramIn.senderPort())) {
            if(_sessionTargets.size() >= 1){
                _sessionTargets[0].get()->address = senderAddress;
                _sessionTargets[0].get()->port = datagramIn.senderPort();
                _udpConfig->addHost(senderAddress.toString(), datagramIn.senderPort());
                return;
            }
            qCDebug(UDPLinkLog) << "UDP Adding target:" << senderAddress << datagramIn.senderPort();
            _sessionTargets.append(std::make_shared<UDPClient>(senderAddress, datagramIn.senderPort()));
            _udpConfig->addHost(senderAddress.toString(), datagramIn.senderPort());
        }
    }
}

void UDPWorker::_onSocketBytesWritten(qint64 bytes)
{
    qCDebug(UDPLinkLog) << "Wrote" << bytes << "bytes";
}

void UDPWorker::_onSocketErrorOccurred(QUdpSocket::SocketError error)
{
    // 如果是連線被拒絕（通常是因為對面關了），直接忽略
    if (error == QAbstractSocket::ConnectionRefusedError) {
        return;
    }
    const QString errorString = _socket->errorString();
    qCWarning(UDPLinkLog) << "UDP Link error:" << error << _socket->errorString();

    if (!_errorEmitted) {
        emit errorOccurred(errorString);
        _errorEmitted = true;
    }
}

/*===========================================================================*/

UDPLink::UDPLink(SharedLinkConfigurationPtr &config, QObject *parent)
    : LinkInterface(config, parent)
    , _udpConfig(qobject_cast<UDPConfiguration*>(config.get()))
    , _worker(new UDPWorker(_udpConfig))
    , _workerThread(new QThread(this))
{
    _workerThread->setObjectName(QStringLiteral("UDP_%1").arg(_udpConfig->name()));

    _worker->moveToThread(_workerThread);

    (void) connect(_workerThread, &QThread::started, _worker, &UDPWorker::setupSocket);
    (void) connect(_workerThread, &QThread::finished, _worker, &QObject::deleteLater);

    (void) connect(_worker, &UDPWorker::connected, this, &UDPLink::_onConnected, Qt::QueuedConnection);
    (void) connect(_worker, &UDPWorker::disconnected, this, &UDPLink::_onDisconnected, Qt::QueuedConnection);
    (void) connect(_worker, &UDPWorker::errorOccurred, this, &UDPLink::_onErrorOccurred, Qt::QueuedConnection);
    (void) connect(_worker, &UDPWorker::dataReceived, this, &UDPLink::_onDataReceived, Qt::QueuedConnection);
    (void) connect(_worker, &UDPWorker::dataSent, this, &UDPLink::_onDataSent, Qt::QueuedConnection);

    _workerThread->start();
}

UDPLink::~UDPLink()
{
    UDPLink::disconnect();

    _workerThread->quit();
    if (!_workerThread->wait()) {
        qCWarning(UDPLinkLog) << "Failed to wait for UDP Thread to close";
    }

    // qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

bool UDPLink::isConnected() const
{
    return _worker->isConnected();
}

bool UDPLink::_connect()
{
    return QMetaObject::invokeMethod(_worker, "connectLink", Qt::QueuedConnection);
}

void UDPLink::disconnect()
{
    (void) QMetaObject::invokeMethod(_worker, "disconnectLink", Qt::QueuedConnection);
}

void UDPLink::_onConnected()
{
    emit connected();
}

void UDPLink::_onDisconnected()
{
    emit disconnected();
}

void UDPLink::_onErrorOccurred(const QString &errorString)
{
    qDebug() << "Communication error:" << errorString;
    emit communicationError(tr("UDP Link Error"), tr("Link %1: %2").arg(_udpConfig->name(), errorString));
}

void UDPLink::_onDataReceived(const QByteArray &data)
{
    emit bytesReceived(this, data);
}

void UDPLink::_onDataSent(const QByteArray &data)
{
    emit bytesSent(this, data);
}

void UDPLink::_writeBytes(const QByteArray& bytes)
{
    (void) QMetaObject::invokeMethod(_worker, "writeData", Qt::QueuedConnection, Q_ARG(QByteArray, bytes));
}

