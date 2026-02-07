#include "rclcpp/rclcpp.hpp"
#include "more_interfaces/msg/marinelink_packet.hpp"

#include <QThread>

class SubscribeWorker : public rclcpp::Node
{
public:
    using MarineCallback = std::function<void(uint8_t,const std::vector<uint8_t>&)>;
    SubscribeWorker(MarineCallback callback) : Node("link_manager_subscribe_worker"), callback_(callback)
    {
        marine_subscription_ = this->create_subscription<more_interfaces::msg::MarinelinkPacket>(
            "marinelink_tosend", 10, 
            std::bind(&SubscribeWorker::topic_callback, this, std::placeholders::_1));

    }
    ~SubscribeWorker() = default;
private:
    MarineCallback callback_;
    rclcpp::Subscription<more_interfaces::msg::MarinelinkPacket>::SharedPtr marine_subscription_;
    void topic_callback(const more_interfaces::msg::MarinelinkPacket::SharedPtr msg)
    {
        if(callback_) {
            callback_(msg->topic, msg->payload); // 假設 topic_type 為 1
        }
    }
};

class PublishWorker : public rclcpp::Node
{
public:
    PublishWorker() : Node("link_manager_publish_worker")
    {
        marinelink_publisher_ = this->create_publisher<more_interfaces::msg::MarinelinkPacket>("video/cmd", 10);
    }
    ~PublishWorker() = default;
    void publish_payload(const uint8_t* data, size_t length) {
        auto msg = more_interfaces::msg::MarinelinkPacket();
        // 將原始資料填入 msg.payload (std::vector<uint8_t>)
        msg.payload.assign(data, data + length);
        msg.topic = 1; // 假設 topic_type 為 1
        // 直接發布
        marinelink_publisher_->publish(msg);
        RCLCPP_DEBUG(this->get_logger(), "Forwarded payload to ROS topic");
    }
private:
    rclcpp::Publisher<more_interfaces::msg::MarinelinkPacket>::SharedPtr marinelink_publisher_;
};
