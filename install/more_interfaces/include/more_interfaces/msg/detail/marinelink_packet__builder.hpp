// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from more_interfaces:msg/MarinelinkPacket.idl
// generated code does not contain a copyright notice

#ifndef MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__BUILDER_HPP_
#define MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__BUILDER_HPP_

#include "more_interfaces/msg/detail/marinelink_packet__struct.hpp"
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <utility>


namespace more_interfaces
{

namespace msg
{

namespace builder
{

class Init_MarinelinkPacket_payload
{
public:
  explicit Init_MarinelinkPacket_payload(::more_interfaces::msg::MarinelinkPacket & msg)
  : msg_(msg)
  {}
  ::more_interfaces::msg::MarinelinkPacket payload(::more_interfaces::msg::MarinelinkPacket::_payload_type arg)
  {
    msg_.payload = std::move(arg);
    return std::move(msg_);
  }

private:
  ::more_interfaces::msg::MarinelinkPacket msg_;
};

class Init_MarinelinkPacket_topic
{
public:
  Init_MarinelinkPacket_topic()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_MarinelinkPacket_payload topic(::more_interfaces::msg::MarinelinkPacket::_topic_type arg)
  {
    msg_.topic = std::move(arg);
    return Init_MarinelinkPacket_payload(msg_);
  }

private:
  ::more_interfaces::msg::MarinelinkPacket msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::more_interfaces::msg::MarinelinkPacket>()
{
  return more_interfaces::msg::builder::Init_MarinelinkPacket_topic();
}

}  // namespace more_interfaces

#endif  // MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__BUILDER_HPP_
