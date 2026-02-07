// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from more_interfaces:msg/MavlinkPacket.idl
// generated code does not contain a copyright notice

#ifndef MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__BUILDER_HPP_
#define MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__BUILDER_HPP_

#include "more_interfaces/msg/detail/mavlink_packet__struct.hpp"
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <utility>


namespace more_interfaces
{

namespace msg
{

namespace builder
{

class Init_MavlinkPacket_payload
{
public:
  Init_MavlinkPacket_payload()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::more_interfaces::msg::MavlinkPacket payload(::more_interfaces::msg::MavlinkPacket::_payload_type arg)
  {
    msg_.payload = std::move(arg);
    return std::move(msg_);
  }

private:
  ::more_interfaces::msg::MavlinkPacket msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::more_interfaces::msg::MavlinkPacket>()
{
  return more_interfaces::msg::builder::Init_MavlinkPacket_payload();
}

}  // namespace more_interfaces

#endif  // MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__BUILDER_HPP_
