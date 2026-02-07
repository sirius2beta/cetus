#include "RosInterface.h"

void RosInterface::run()
{
    auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();

    auto sub_worker = std::make_shared<SubscribeWorker>(
        [this](uint8_t type, const std::vector<uint8_t>& payload) {
            this->handleMavlinkAssembly(type, payload);
        }
    );
    auto pub_worker = std::make_shared<PublishWorker>();

    executor->add_node(sub_worker);
    executor->add_node(pub_worker);

    sub_worker_ = sub_worker;
    pub_worker_ = pub_worker;

    executor->spin();

}

void RosInterface::onMavlinkToParse(const mavlink_message_t &message)
{

    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received MAVLink message with ID: %d", message.msgid);
    if(message.msgid == MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER){
        mavlink_custom_legacy_wrapper_t wrapper;
        mavlink_msg_custom_legacy_wrapper_decode(&message, &wrapper);
        RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Decoded MAVLink message - Target System: %d, Target Component: %d, Length: %d, Topic: %d", 
            wrapper.target_system, wrapper.target_component, wrapper.length, wrapper.topic);
        // publish wrapper.payload if wrapper.topic == 1
        if(wrapper.topic == 1){
            auto msg = more_interfaces::msg::MarinelinkPacket();
            msg.topic = wrapper.topic;
            msg.payload = std::vector<uint8_t>(wrapper.payload, wrapper.payload + wrapper.length);
            // publish the message using the publisher node
            pub_worker_->publish_payload(wrapper.payload, wrapper.length);
        }
    }
}

void RosInterface::handleMavlinkAssembly(uint8_t topic_type, const std::vector<uint8_t>& raw_payload) 
{
    mavlink_message_t mav_msg;
    uint8_t target_sys = 1;
    uint8_t target_comp = 1;

    mavlink_msg_custom_legacy_wrapper_pack(
        target_sys, target_comp, &mav_msg,
        target_sys, target_comp,
        raw_payload.size(),
        topic_type,
        raw_payload.data()
    );

    emit mavlinkToSend(mav_msg);
    
    RCLCPP_DEBUG(rclcpp::get_logger("RosInterface"), "MAVLink Packet assembled and emitted.");
}