#include "RosInterface.h"
#include "LinkPackage/UDPLink.h"
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
        // publish wrapper.payload if wrapper.topic == 1
        if(wrapper.topic == 1){
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Publishing payload of length %d to topic %d", wrapper.length, wrapper.topic);
            auto msg = more_interfaces::msg::MarinelinkPacket();
            msg.topic = wrapper.topic;
            SharedLinkConfigurationPtr sharedLinConfig = link->linkConfiguration();
            if(sharedLinConfig){
                UDPConfiguration* udpConfig = qobject_cast<UDPConfiguration*>(sharedLinConfig.get());
                RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "target hosts size: %d", udpConfig->targetHosts().size());
                msg.address = udpConfig->targetHosts().size() > 0 ? udpConfig->targetHosts().constFirst()->address.toString().toStdString() : "0";
            }
            
            msg.payload = std::vector<uint8_t>(wrapper.payload, wrapper.payload + wrapper.length);
            // publish the message using the publisher node
            pub_worker_->publish_payload(msg);
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
    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Assembled MAVLink message with topic type: %d and payload length: %d", topic_type, raw_payload.size());
    emit mavlinkToSend(mav_msg);
    
    RCLCPP_DEBUG(rclcpp::get_logger("RosInterface"), "MAVLink Packet assembled and emitted.");
}