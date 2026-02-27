#include "mavlinkprotocol.h"
#include "linkmanager.h"
#include "bridge.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QtCore/qapplicationstatic.h>
    Q_APPLICATION_STATIC(MAVLinkProtocol, _mavlinkProtocolInstance);

#else
    #include <QtGlobal>
    Q_GLOBAL_STATIC(MAVLinkProtocol, _mavlinkProtocolInstance)
#endif
#include<QSettings>
#include<QLoggingCategory>

Q_LOGGING_CATEGORY(MAVLinkProtocolLog, "qgc.comms.mavlinkprotocol");


MAVLinkProtocol::MAVLinkProtocol(QObject *parent)
    : QObject{parent}
{
    qDebug()<<"create mavlink protocol";
    QSettings settings;
    settings.setValue("mavlinkVersion", "2");

}

MAVLinkProtocol::~MAVLinkProtocol()
{

}

MAVLinkProtocol *MAVLinkProtocol::instance()
{
    return _mavlinkProtocolInstance();
}

void MAVLinkProtocol::receiveBytes(LinkInterface *link, const QByteArray &data)
{
    const SharedLinkInterfacePtr linkPtr = LinkManager::instance()->sharedLinkInterfacePointerForLink(link);
    if (!linkPtr) {
        qCDebug(MAVLinkProtocolLog) << "receiveBytes: link gone!" << data.size() << "bytes arrived too late";
        return;
    }

    for(const uint8_t &byte: data){
        const uint8_t mavlinkChannel = link->mavlinkChannel();
        mavlink_message_t message{};
        mavlink_status_t status{};

        if (mavlink_parse_char(mavlinkChannel, byte, &message, &status) != MAVLINK_FRAMING_OK) {
            continue;
        }

        if(link->linkConfiguration()->type() == LinkConfiguration::TypeSerial){

            _forward(message);

        }else{
            _forwardtoPixhawk(message);
        }

        emit messageReceived(link, message);
        continue;
    }
}

void MAVLinkProtocol::_forwardtoPixhawk(const mavlink_message_t &message)
{



    SharedLinkInterfacePtr pixhawkLink = LinkManager::instance()->mavlinkPixhawkLink();
    if (!pixhawkLink) {
        return;
    }

    uint8_t buf[MAVLINK_MAX_PACKET_LEN]{};
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &message);
    (void) pixhawkLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buf), len);
}

void MAVLinkProtocol::_forward(const mavlink_message_t &message)
{
    uint8_t buf[MAVLINK_MAX_PACKET_LEN]{};
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &message);

    SharedLinkInterfacePtr primaryLink = Bridge::instance()->primaryLink().lock();

    if(primaryLink){
        (void) primaryLink->writeBytesThreadSafe(reinterpret_cast<const char*>(buf), len);
    }

}

void MAVLinkProtocol::resetMetadataForLink(LinkInterface *link)
{
    const uint8_t channel = link->mavlinkChannel();
    //_totalReceiveCounter[channel] = 0;
    //_totalLossCounter[channel] = 0;
    //_runningLossPercent[channel] = 0.f;

    //link->setDecodedFirstMavlinkPacket(false);
}

#ifdef MAVLINK_EXTERNAL_RX_STATUS
mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];
#endif

#ifdef MAVLINK_GET_CHANNEL_STATUS
mavlink_status_t* mavlink_get_channel_status(uint8_t channel)
{
    // 檢查邊界，防止陣列溢位
    if (channel >= MAVLINK_COMM_NUM_BUFFERS) {
        return nullptr;
    }

    // 回傳全域陣列中對應通道的地址
    return &m_mavlink_status[channel];
}
#endif
