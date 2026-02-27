#ifndef BRIDGE_H
#define BRIDGE_H

#include "linkinterface.h"
#include "linkinterface.h"
#include "MAVLinkLib.h"
#include "mavlinkprotocol.h"

#include <QObject>
#include <QtCore/QElapsedTimer>
#include <QTimer>


class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(QObject *parent = nullptr);

    static Bridge *instance();

    void init();
    void addUdpLinks(LinkInterface* primaryUdpLink, LinkInterface* secondaryUdpLink);
    void addPixhawkSerialLink(LinkInterface* pixhawkSerialLink);

    WeakLinkInterfacePtr primaryLink() const { return _primaryLink; }
signals:
    void mavlinkToParse(const mavlink_message_t &message);

protected slots:
    void mavlinkMessageReceived(LinkInterface *link, const mavlink_message_t &message);





private slots:
    void _commLostCheck();
    void _sendGCSHeartbeat();

private:
    static constexpr int _heartbeatTimeoutMSecs = 1000; ///< Check for comm lost once a second
    static constexpr int _commLostCheckTimeoutMSecs = 1000; ///< Check for comm lost once a second
    static constexpr int _heartbeatMaxElpasedMSecs = 3500;  ///< No heartbeat for longer than this indicates comm loss

    bool _updatePrimaryLink();

    QTimer *_commLostCheckTimer = nullptr;
    QTimer *_bridgeHearbeatTimer = nullptr;
    WeakLinkInterfacePtr _primaryLink;

    LinkInterface* _primaryUdpLink;
    LinkInterface* _secondaryUdpLink;
    LinkInterface* _pixhawkSerialLink;
    struct LinkInfo_t {
        SharedLinkInterfacePtr link;
        bool commLost = false;
        QElapsedTimer heartbeatElapsedTimer;
    };
    LinkInfo_t _primaryUdpLinkInfo;
    LinkInfo_t _secondaryUdpLinkInfo;
    LinkInfo_t _pixhawkSerialLinkInfo;
    uint8_t _systemID = 1;
    uint8_t _componentID = 5;
};

#endif // BRIDGE_H
