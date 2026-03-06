#include "bridge.h"
#include "linkmanager.h"
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    #include <QtCore/qapplicationstatic.h>
    Q_APPLICATION_STATIC(Bridge, _bridgeInstance)
#else
    #include <QtGlobal>
    Q_GLOBAL_STATIC(Bridge, _bridgeInstance)
#endif
#include <QtCore/QTimer>
#include <QtQml/qqml.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(BridgeLog, "hypex.comms.bridge", QtWarningMsg)


Bridge::Bridge(QObject *parent)
    : QObject{parent},
    _primaryUdpLink(nullptr),
    _secondaryUdpLink(nullptr),
    _pixhawkSerialLink(nullptr),
    _commLostCheckTimer(new QTimer(this)),
    _bridgeHearbeatTimer(new QTimer(this))
{
    connect(MAVLinkProtocol::instance(), &MAVLinkProtocol::messageReceived, this, &Bridge::mavlinkMessageReceived);
    (void) connect(_commLostCheckTimer, &QTimer::timeout, this, &Bridge::_commLostCheck);
    (void) connect(_bridgeHearbeatTimer, &QTimer::timeout, this, &Bridge::_sendGCSHeartbeat);

    _commLostCheckTimer->setSingleShot(false);
    _commLostCheckTimer->setInterval(_commLostCheckTimeoutMSecs);

    _bridgeHearbeatTimer->setSingleShot(false);
    _bridgeHearbeatTimer->setInterval(_heartbeatTimeoutMSecs);
}

Bridge* Bridge::instance()
{
    return _bridgeInstance;
}

void Bridge::init()
{

}

void Bridge::addUdpLinks(LinkInterface* primaryUdpLink, LinkInterface* secondaryUdpLink)
{

    _primaryUdpLink = primaryUdpLink;
    _primaryUdpLinkInfo.link = LinkManager::instance()->sharedLinkInterfacePointerForLink(primaryUdpLink);
    _primaryUdpLinkInfo.heartbeatElapsedTimer.start();

    _secondaryUdpLink = secondaryUdpLink;
    _secondaryUdpLinkInfo.link = LinkManager::instance()->sharedLinkInterfacePointerForLink(secondaryUdpLink);
    _secondaryUdpLinkInfo.heartbeatElapsedTimer.start();
    
    _commLostCheckTimer->start();
    _bridgeHearbeatTimer->start();
}

void Bridge::addPixhawkSerialLink(LinkInterface* pixhawkSerialLink)
{
    _pixhawkSerialLink = pixhawkSerialLink;
}

void Bridge::mavlinkMessageReceived(LinkInterface *link, const mavlink_message_t &message)
{
    uint8_t buf[MAVLINK_MAX_PACKET_LEN]{};
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &message);
    // Radio status messages come from Sik Radios directly. It doesn't indicate there is any life on the other end.
    if (message.msgid == MAVLINK_MSG_ID_RADIO_STATUS) {
        return;
    }

    if(link == _primaryUdpLink){
        _primaryUdpLinkInfo.heartbeatElapsedTimer.restart();
        if(_primaryUdpLinkInfo.commLost){
            _primaryUdpLinkInfo.commLost = false;
            _updatePrimaryLink();
        }

    }else if(link == _secondaryUdpLink){
        _secondaryUdpLinkInfo.heartbeatElapsedTimer.restart();
        if(_secondaryUdpLinkInfo.commLost){
            _secondaryUdpLinkInfo.commLost = false;
            _updatePrimaryLink();
        }
    }
    emit mavlinkToParse(link,message);
}

void Bridge::onMavlinkToSend(const mavlink_message_t &message)
{
    if (!_primaryUdpLink || !_secondaryUdpLink) {
        return; 
    }
    uint8_t buf[MAVLINK_MAX_PACKET_LEN]{};
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &message);

    SharedLinkInterfacePtr primaryLink = Bridge::instance()->primaryLink().lock();

    if(primaryLink){
        (void) primaryLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buf), len);
    }
}


bool Bridge::_updatePrimaryLink()
{
    SharedLinkInterfacePtr primaryLink = _primaryLink.lock();


    if(primaryLink.get() == _primaryUdpLink){
        if(!_primaryUdpLinkInfo.commLost){
            return false;
        }else{
            if(!_secondaryUdpLinkInfo.commLost){
                _primaryLink = _secondaryUdpLinkInfo.link;
                qCDebug(BridgeLog,"secondary link up");
                return true;
            }else{
                return false;
            }
        }
        return false;
    }else{
        if(!_secondaryUdpLinkInfo.commLost){
            if(!_primaryUdpLinkInfo.commLost){
                _primaryLink = _primaryUdpLinkInfo.link;
                qCDebug(BridgeLog,"primary link up");
                return true;
            }else{
                return false;
            }
        }else{
            if(!_primaryUdpLinkInfo.commLost){
                _primaryLink = _primaryUdpLinkInfo.link;
                qCDebug(BridgeLog,"primary link up");
                return true;
            }else{
                //need backup
                _primaryLink = _primaryUdpLinkInfo.link;
                qCDebug(BridgeLog,"primary link up");
                return true;

            }

        }
    }
}

void Bridge::_commLostCheck()
{

    const int heartbeatTimeout = _heartbeatMaxElpasedMSecs;
    bool linkStatusChange = false;

    if (!_primaryUdpLinkInfo.commLost &&  (_primaryUdpLinkInfo.heartbeatElapsedTimer.elapsed() > heartbeatTimeout)) {
        _primaryUdpLinkInfo.commLost = true;
        linkStatusChange = true;

        // Notify the user of individual link communication loss
        const bool isPrimaryLink = _primaryUdpLinkInfo.link.get() == _primaryLink.lock().get();
    }
    if (!_secondaryUdpLinkInfo.commLost &&  (_secondaryUdpLinkInfo.heartbeatElapsedTimer.elapsed() > heartbeatTimeout)) {
        _secondaryUdpLinkInfo.commLost = true;
        linkStatusChange = true;

        // Notify the user of individual link communication loss
        const bool isPrimaryLink = _secondaryUdpLinkInfo.link.get() == _primaryLink.lock().get();
    }
    qCDebug(BridgeLog) << "primary link heartbeat elapsed time:" << _primaryUdpLinkInfo.heartbeatElapsedTimer.elapsed() << "ms, commLost:" << _primaryUdpLinkInfo.commLost;
    qCDebug(BridgeLog) << "secondary link heartbeat elapsed time:" << _secondaryUdpLinkInfo.heartbeatElapsedTimer.elapsed() << "ms, commLost:" << _secondaryUdpLinkInfo.commLost;

    if (_updatePrimaryLink()) {
        qCDebug(BridgeLog, "update link");
    }

}

void Bridge::_sendGCSHeartbeat()
{
    if (!_primaryUdpLink || !_secondaryUdpLink) {
        return; 
    }
    uint8_t target_sys = 1;
    uint8_t target_comp = 1;
    uint8_t topic = 0; // heartbeat topic
    //empty payload for heartbeat
    QByteArray paddedMsg(251, 0);
    uint8_t payload[251];
    memcpy(payload, paddedMsg.data(), paddedMsg.size());
    uint8_t length = 0;


    if(!_primaryUdpLinkInfo.commLost){
        qDebug(BridgeLog, "send heartbeat from primary link");
        mavlink_message_t message{};
        (void) mavlink_msg_custom_legacy_wrapper_pack_chan(
            _systemID,
            _componentID,
            _primaryUdpLink->mavlinkChannel(),
            &message,
            target_sys,
            target_comp,
            length,
            topic,
            payload
        );

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        const uint16_t len = mavlink_msg_to_send_buffer(buffer, &message);
        (void) _primaryUdpLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buffer), len);
    }
    if(!_secondaryUdpLinkInfo.commLost){
        qDebug(BridgeLog, "send heartbeat from secondary link");
        mavlink_message_t message{};
        (void) mavlink_msg_custom_legacy_wrapper_pack_chan(
            _systemID,
            _componentID,
            _secondaryUdpLink->mavlinkChannel(),
            &message,
            target_sys,
            target_comp,
            length,
            topic,
            payload
        );

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        const uint16_t len = mavlink_msg_to_send_buffer(buffer, &message);
        (void) _secondaryUdpLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buffer), len);
    }
}
