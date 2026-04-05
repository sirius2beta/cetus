#include "RosInterface.h"
#include "LinkPackage/UDPLink.h"
#include "more_interfaces/msg/mavlink_values.hpp"

void RosInterface::run()
{
    // --- 1. 優先建立物件並賦值 (Critical Path) ---
    auto pub = std::make_shared<PublishWorker>();
    auto sub = std::make_shared<SubscribeWorker>(
        [this](uint8_t type, const std::vector<uint8_t> &payload) {
            this->handleMavlinkAssembly(type, payload);
        });

    // 這裡的賦值必須在 executor 啟動和任何 Slot 被呼叫前完成
    this->pub_worker_ = pub;
    this->sub_worker_ = sub;

    // --- 2. 配置功能 ---
    pub_worker_->setTimerCallback([this]() { 
        this->publishMavlinkValues(); 
    }, mavlinkMsgUpdateTimeout);

    // --- 3. 啟動 ROS 執行器 ---
    auto executor = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor->add_node(sub_worker_);
    executor->add_node(pub_worker_);

    RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "ROS Workers initialized.");

    // spin() 會阻塞在這裡，直到 thread 被要求停止
    executor->spin();
}

void RosInterface::onMavlinkToParse(LinkInterface *link, const mavlink_message_t &message)
{

    // RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received MAVLink message with ID: %d", message.msgid);
    switch (message.msgid)
    {
    case MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER:
    {
        mavlink_custom_legacy_wrapper_t wrapper;
        mavlink_msg_custom_legacy_wrapper_decode(&message, &wrapper);

        //RCLCPP_INFO(rclcpp::get_logger("RosInterface"),
        //            "Decoded MAVLink message - Target System: %d, Target Component: %d, Length: %d, Topic: %d",
        //            wrapper.target_system, wrapper.target_component, wrapper.length, wrapper.topic);

        auto msg = more_interfaces::msg::MarinelinkPacket();
        msg.topic = wrapper.topic;

        if (auto *udpLink = dynamic_cast<UDPLink *>(link))
        {
            msg.address = udpLink->lastSenderAddress().toString().toStdString();
        }

        msg.payload = std::vector<uint8_t>(wrapper.payload, wrapper.payload + wrapper.length);

        // 根據 topic 處理邏輯
        switch (wrapper.topic)
        {
        case 1:
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Video control command (Topic 1)");
            if (pub_worker_) {
                pub_worker_->publish_payload(msg);
            } else {
                RCLCPP_ERROR(rclcpp::get_logger("RosInterface"), "pub_worker_ is null!");
            }
            break;
        case 2: //未來video command整合
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Videomanager play command (Topic 2)");
            pub_worker_->publish_payload(msg);
            break;
        case 3:
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Videomanager quit command (Topic 3)");
            pub_worker_->publish_payload(msg);
            break;
        case 5:
            RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Control command (Topic 5)");
            pub_worker_->publishControl(msg);
            break;
        default:
            // 可選：處理未定義的 topic
            break;
        }
        break;
    }

    case MAVLINK_MSG_ID_GPS_RAW_INT:
    {
        mavlink_gps_raw_int_t pos;
        mavlink_msg_gps_raw_int_decode(&message, &pos);
        mavlink_values_.lon = pos.lon;
        mavlink_values_.lat = pos.lat;
        mavlink_values_.alt = pos.alt;
        mavlink_values_.fix_type = static_cast<uint8_t>(pos.fix_type);
        // RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Received GPS_RAW_INT - Yaw: %d", pos.hdg);
        break;
    }

    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
    {
        mavlink_global_position_int_t pos;
        mavlink_msg_global_position_int_decode(&message, &pos);
        mavlink_values_.yaw = static_cast<uint16_t>(pos.hdg);
        break;
    }

    case MAVLINK_MSG_ID_ATTITUDE:
    {
        mavlink_attitude_t att;
        mavlink_msg_attitude_decode(&message, &att);
        mavlink_values_.pitch = att.pitch;
        mavlink_values_.roll = att.roll;
        break;
    }

    case MAVLINK_MSG_ID_DISTANCE_SENSOR:
    {
        mavlink_distance_sensor_t dist;
        mavlink_msg_distance_sensor_decode(&message, &dist);
        mavlink_values_.depth = dist.current_distance;
        break;
    }

    case MAVLINK_MSG_ID_SYS_STATUS:
    {
        mavlink_sys_status_t sys_status;
        mavlink_msg_sys_status_decode(&message, &sys_status);
        mavlink_values_.voltage_battery = sys_status.voltage_battery;
        mavlink_values_.current_battery = sys_status.current_battery;
        mavlink_values_.battery_remaining = sys_status.battery_remaining;
        break;
    }

    case MAVLINK_MSG_ID_VFR_HUD:
    {
        mavlink_vfr_hud_t hud;
        mavlink_msg_vfr_hud_decode(&message, &hud);
        mavlink_values_.groundspeed = hud.groundspeed;
        break;
    }

    default:
        // 處理未知的 msgid
        break;
    }
}
void RosInterface::publishMavlinkValues()
{
    auto values = more_interfaces::msg::MavlinkValues();
    values.yaw = mavlink_values_.yaw;
    values.pitch = mavlink_values_.pitch;
    values.roll = mavlink_values_.roll;
    values.lon = mavlink_values_.lon;
    values.lat = mavlink_values_.lat;
    values.alt = mavlink_values_.alt;
    values.fix_type = mavlink_values_.fix_type;
    values.depth = mavlink_values_.depth;
    values.groundspeed = mavlink_values_.groundspeed;
    values.voltage_battery = mavlink_values_.voltage_battery;
    values.current_battery = mavlink_values_.current_battery;
    values.battery_remaining = mavlink_values_.battery_remaining;
    pub_worker_->publishMavlinkValues(values);
}

void RosInterface::handleMavlinkAssembly(uint8_t topic_type, const std::vector<uint8_t> &raw_payload)
{
    mavlink_message_t mav_msg;
    uint8_t target_sys = 1;
    uint8_t target_comp = 1;

    mavlink_msg_custom_legacy_wrapper_pack(
        target_sys, target_comp, &mav_msg,
        target_sys, target_comp,
        raw_payload.size(),
        topic_type,
        raw_payload.data());
    //RCLCPP_INFO(rclcpp::get_logger("RosInterface"), "Assembled MAVLink message with topic type: %d and payload length: %d", topic_type, raw_payload.size());
    emit mavlinkToSend(mav_msg);
}