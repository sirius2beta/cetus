#pragma once
#include "ros_worker.h"
#include <QThread>
#include "mavlink.h"
class RosInterface : public QThread
{
    Q_OBJECT
public:
    explicit RosInterface(QObject *parent = nullptr) : QThread(parent) {}
    ~RosInterface() = default;
signals:
    void mavlinkToSend(const mavlink_message_t &message);
public slots:
    void onMavlinkToParse(const mavlink_message_t &message);
protected:
    void run() override;
private:
    void handleMavlinkAssembly(uint8_t topic_type, const std::vector<uint8_t>& raw_payload);
    std::shared_ptr<SubscribeWorker> sub_worker_;
    std::shared_ptr<PublishWorker> pub_worker_;
};