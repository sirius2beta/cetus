// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from more_interfaces:msg/MarinelinkPacket.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "more_interfaces/msg/detail/marinelink_packet__rosidl_typesupport_introspection_c.h"
#include "more_interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "more_interfaces/msg/detail/marinelink_packet__functions.h"
#include "more_interfaces/msg/detail/marinelink_packet__struct.h"


// Include directives for member types
// Member `payload`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  more_interfaces__msg__MarinelinkPacket__init(message_memory);
}

void MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_fini_function(void * message_memory)
{
  more_interfaces__msg__MarinelinkPacket__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_member_array[2] = {
  {
    "topic",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(more_interfaces__msg__MarinelinkPacket, topic),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "payload",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(more_interfaces__msg__MarinelinkPacket, payload),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_members = {
  "more_interfaces__msg",  // message namespace
  "MarinelinkPacket",  // message name
  2,  // number of fields
  sizeof(more_interfaces__msg__MarinelinkPacket),
  MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_member_array,  // message members
  MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_init_function,  // function to initialize message memory (memory has to be allocated)
  MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_type_support_handle = {
  0,
  &MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_more_interfaces
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, more_interfaces, msg, MarinelinkPacket)() {
  if (!MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_type_support_handle.typesupport_identifier) {
    MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &MarinelinkPacket__rosidl_typesupport_introspection_c__MarinelinkPacket_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
