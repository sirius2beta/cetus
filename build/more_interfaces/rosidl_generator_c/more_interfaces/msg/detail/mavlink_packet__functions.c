// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from more_interfaces:msg/MavlinkPacket.idl
// generated code does not contain a copyright notice
#include "more_interfaces/msg/detail/mavlink_packet__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `payload`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
more_interfaces__msg__MavlinkPacket__init(more_interfaces__msg__MavlinkPacket * msg)
{
  if (!msg) {
    return false;
  }
  // payload
  if (!rosidl_runtime_c__uint8__Sequence__init(&msg->payload, 0)) {
    more_interfaces__msg__MavlinkPacket__fini(msg);
    return false;
  }
  return true;
}

void
more_interfaces__msg__MavlinkPacket__fini(more_interfaces__msg__MavlinkPacket * msg)
{
  if (!msg) {
    return;
  }
  // payload
  rosidl_runtime_c__uint8__Sequence__fini(&msg->payload);
}

bool
more_interfaces__msg__MavlinkPacket__are_equal(const more_interfaces__msg__MavlinkPacket * lhs, const more_interfaces__msg__MavlinkPacket * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // payload
  if (!rosidl_runtime_c__uint8__Sequence__are_equal(
      &(lhs->payload), &(rhs->payload)))
  {
    return false;
  }
  return true;
}

bool
more_interfaces__msg__MavlinkPacket__copy(
  const more_interfaces__msg__MavlinkPacket * input,
  more_interfaces__msg__MavlinkPacket * output)
{
  if (!input || !output) {
    return false;
  }
  // payload
  if (!rosidl_runtime_c__uint8__Sequence__copy(
      &(input->payload), &(output->payload)))
  {
    return false;
  }
  return true;
}

more_interfaces__msg__MavlinkPacket *
more_interfaces__msg__MavlinkPacket__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  more_interfaces__msg__MavlinkPacket * msg = (more_interfaces__msg__MavlinkPacket *)allocator.allocate(sizeof(more_interfaces__msg__MavlinkPacket), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(more_interfaces__msg__MavlinkPacket));
  bool success = more_interfaces__msg__MavlinkPacket__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
more_interfaces__msg__MavlinkPacket__destroy(more_interfaces__msg__MavlinkPacket * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    more_interfaces__msg__MavlinkPacket__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
more_interfaces__msg__MavlinkPacket__Sequence__init(more_interfaces__msg__MavlinkPacket__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  more_interfaces__msg__MavlinkPacket * data = NULL;

  if (size) {
    data = (more_interfaces__msg__MavlinkPacket *)allocator.zero_allocate(size, sizeof(more_interfaces__msg__MavlinkPacket), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = more_interfaces__msg__MavlinkPacket__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        more_interfaces__msg__MavlinkPacket__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
more_interfaces__msg__MavlinkPacket__Sequence__fini(more_interfaces__msg__MavlinkPacket__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      more_interfaces__msg__MavlinkPacket__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

more_interfaces__msg__MavlinkPacket__Sequence *
more_interfaces__msg__MavlinkPacket__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  more_interfaces__msg__MavlinkPacket__Sequence * array = (more_interfaces__msg__MavlinkPacket__Sequence *)allocator.allocate(sizeof(more_interfaces__msg__MavlinkPacket__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = more_interfaces__msg__MavlinkPacket__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
more_interfaces__msg__MavlinkPacket__Sequence__destroy(more_interfaces__msg__MavlinkPacket__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    more_interfaces__msg__MavlinkPacket__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
more_interfaces__msg__MavlinkPacket__Sequence__are_equal(const more_interfaces__msg__MavlinkPacket__Sequence * lhs, const more_interfaces__msg__MavlinkPacket__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!more_interfaces__msg__MavlinkPacket__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
more_interfaces__msg__MavlinkPacket__Sequence__copy(
  const more_interfaces__msg__MavlinkPacket__Sequence * input,
  more_interfaces__msg__MavlinkPacket__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(more_interfaces__msg__MavlinkPacket);
    more_interfaces__msg__MavlinkPacket * data =
      (more_interfaces__msg__MavlinkPacket *)realloc(output->data, allocation_size);
    if (!data) {
      return false;
    }
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!more_interfaces__msg__MavlinkPacket__init(&data[i])) {
        /* free currently allocated and return false */
        for (; i-- > output->capacity; ) {
          more_interfaces__msg__MavlinkPacket__fini(&data[i]);
        }
        free(data);
        return false;
      }
    }
    output->data = data;
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!more_interfaces__msg__MavlinkPacket__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
