#include "RosInterface.h"
#include "LinkPackage/UDPLink.h"
#include "more_interfaces/msg/mavlink_values.hpp"


void RosInterface::run()
{
    auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    
    
    printf("DEBUG: Creating SubWorker...\n");
    auto sub_worker = std::make_shared<SubscribeWorker>(
        [this](uint8_t type, const std::vector<uint8_t>& payload) {
            this->handleMavlinkAssembly(type, payload);
        }
    );
    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Creating PubWorker...");
    auto pub_worker = std::make_shared<PublishWorker>();
    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Adding nodes to executor...");
    executor->add_node(sub_worker);
    executor->add_node(pub_worker);
    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "--- NODES ADDED, SPINNING ---");
    sub_worker_ = sub_worker;
    pub_worker_ = pub_worker;

    pub_worker_->setTimerCallback([this]() {
        this->publishMavlinkValues();
    });

    executor->spin();
    

}

void RosInterface::onMavlinkToParse(LinkInterface *link, const mavlink_message_t &message)
{

    //RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received MAVLink message with ID: %d", message.msgid);
    if(message.msgid == MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER){
        mavlink_custom_legacy_wrapper_t wrapper;
        mavlink_msg_custom_legacy_wrapper_decode(&message, &wrapper);
        RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Decoded MAVLink message - Target System: %d, Target Component: %d, Length: %d, Topic: %d", 
            wrapper.target_system, wrapper.target_component, wrapper.length, wrapper.topic);
        auto msg = more_interfaces::msg::MarinelinkPacket();
        msg.topic = wrapper.topic;
        UDPLink* udpLink = dynamic_cast<UDPLink*>(link);
        if(udpLink){
            msg.address = udpLink->lastSenderAddress().toString().toStdString();
        }
        msg.payload = std::vector<uint8_t>(wrapper.payload, wrapper.payload + wrapper.length);
    // publish wrapper.payload if wrapper.topic == 1
        if(wrapper.topic == 1){
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Publishing payload of length %d to topic %d", wrapper.length, wrapper.topic);
            pub_worker_->publish_payload(msg);
        }else if(wrapper.topic == 2){
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Videomanager play command", wrapper.topic);
            pub_worker_->publish_payload(msg);
        }else if(wrapper.topic == 3){
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Videomanager quit command", wrapper.topic);
            pub_worker_->publish_payload(msg);
        }
    }else if(message.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT){
        mavlink_global_position_int_t pos;
        mavlink_msg_global_position_int_decode(&message, &pos);
        mavlink_values_.yaw = static_cast<uint16_t>(pos.hdg);
        //RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received GLOBAL_POSITION_INT -  Yaw: %d",  pos.hdg);
    }else if(message.msgid == MAVLINK_MSG_ID_ATTITUDE){
        mavlink_attitude_t att;
        mavlink_msg_attitude_decode(&message, &att);
        mavlink_values_.pitch = att.pitch;
        mavlink_values_.roll = att.roll;
        //RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received ATTITUDE - Pitch: %f, Roll: %f", att.pitch, att.roll);
    }

}
void RosInterface::publishMavlinkValues() {
    auto values = more_interfaces::msg::MavlinkValues();
    values.yaw = mavlink_values_.yaw;
    values.pitch = mavlink_values_.pitch;
    values.roll = mavlink_values_.roll;
    pub_worker_->publishMavlinkValues(values);
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
    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Assembled MAVLink message with topic type: %d and payload length: %d", topic_type, raw_payload.size());
    emit mavlinkToSend(mav_msg);
    
    RCLCPP_DEBUG(rclcpp::get_logger("RosInterface"), "MAVLink Packet assembled and emitted.");
}