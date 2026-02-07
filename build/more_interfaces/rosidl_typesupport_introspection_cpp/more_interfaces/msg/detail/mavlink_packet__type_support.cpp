// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from more_interfaces:msg/MavlinkPacket.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "more_interfaces/msg/detail/mavlink_packet__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace more_interfaces
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void MavlinkPacket_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) more_interfaces::msg::MavlinkPacket(_init);
}

void MavlinkPacket_fini_function(void * message_memory)
{
  auto typed_message = static_cast<more_interfaces::msg::MavlinkPacket *>(message_memory);
  typed_message->~MavlinkPacket();
}

size_t size_function__MavlinkPacket__payload(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<uint8_t> *>(untyped_member);
  return member->size();
}

const void * get_const_function__MavlinkPacket__payload(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<uint8_t> *>(untyped_member);
  return &member[index];
}

void * get_function__MavlinkPacket__payload(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<uint8_t> *>(untyped_member);
  return &member[index];
}

void resize_function__MavlinkPacket__payload(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<uint8_t> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember MavlinkPacket_message_member_array[1] = {
  {
    "payload",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(more_interfaces::msg::MavlinkPacket, payload),  // bytes offset in struct
    nullptr,  // default value
    size_function__MavlinkPacket__payload,  // size() function pointer
    get_const_function__MavlinkPacket__payload,  // get_const(index) function pointer
    get_function__MavlinkPacket__payload,  // get(index) function pointer
    resize_function__MavlinkPacket__payload  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers MavlinkPacket_message_members = {
  "more_interfaces::msg",  // message namespace
  "MavlinkPacket",  // message name
  1,  // number of fields
  sizeof(more_interfaces::msg::MavlinkPacket),
  MavlinkPacket_message_member_array,  // message members
  MavlinkPacket_init_function,  // function to initialize message memory (memory has to be allocated)
  MavlinkPacket_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t MavlinkPacket_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &MavlinkPacket_message_members,
  get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace more_interfaces


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<more_interfaces::msg::MavlinkPacket>()
{
  return &::more_interfaces::msg::rosidl_typesupport_introspection_cpp::MavlinkPacket_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, more_interfaces, msg, MavlinkPacket)() {
  return &::more_interfaces::msg::rosidl_typesupport_introspection_cpp::MavlinkPacket_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
