// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from more_interfaces:msg/MarinelinkPacket.idl
// generated code does not contain a copyright notice

#ifndef MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_H_
#define MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'payload'
#include "rosidl_runtime_c/primitives_sequence.h"

// Struct defined in msg/MarinelinkPacket in the package more_interfaces.
typedef struct more_interfaces__msg__MarinelinkPacket
{
  uint8_t topic;
  rosidl_runtime_c__uint8__Sequence payload;
} more_interfaces__msg__MarinelinkPacket;

// Struct for a sequence of more_interfaces__msg__MarinelinkPacket.
typedef struct more_interfaces__msg__MarinelinkPacket__Sequence
{
  more_interfaces__msg__MarinelinkPacket * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} more_interfaces__msg__MarinelinkPacket__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_H_
