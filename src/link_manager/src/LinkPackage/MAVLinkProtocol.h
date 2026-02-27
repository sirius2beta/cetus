#ifndef MAVLINKPROTOCOL_H
#define MAVLINKPROTOCOL_H

#include <QObject>

#include "MAVLinkLib.h"
#include "LinkInterface.h"
class MAVLinkProtocol : public QObject
{
    Q_OBJECT
public:
    explicit MAVLinkProtocol(QObject *parent = nullptr);

    ~MAVLinkProtocol();

    static MAVLinkProtocol *instance();


    void receiveBytes(LinkInterface *link, const QByteArray &data);
    void resetMetadataForLink(LinkInterface *link);
    void forward(LinkInterface *link, const mavlink_message_t &message);
signals:
    void messageReceived(LinkInterface *link, const mavlink_message_t &message);
private:
    void _forward(const mavlink_message_t &message);
    void _forwardtoPixhawk(const mavlink_message_t &message);
};




#endif // MAVLINKPROTOCOL_H
