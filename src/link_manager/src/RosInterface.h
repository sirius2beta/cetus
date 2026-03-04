#pragma once
#include "ros_worker.h"
#include <QThread>
#include "LinkPackage/MAVLinkLib.h"
#include "LinkPackage/linkinterface.h"
typedef struct mavlinkValues{
    uint16_t yaw;
    float pitch;
    float roll;
} mavlinkValues_t;

class RosInterface : public QThread
{
    Q_OBJECT
public:
    explicit RosInterface(QObject *parent = nullptr) : QThread(parent) {}
    ~RosInterface() = default;
    void publishMavlinkValues();
signals:
    void mavlinkToSend(const mavlink_message_t &message);
public slots:
    void onMavlinkToParse(LinkInterface *link, const mavlink_message_t &message);
protected:
    void run() override;
private:
    void handleMavlinkAssembly(uint8_t topic_type, const std::vector<uint8_t>& raw_payload);
    std::shared_ptr<SubscribeWorker> sub_worker_;
    std::shared_ptr<PublishWorker> pub_worker_;
    mavlinkValues_t mavlink_values_;
};