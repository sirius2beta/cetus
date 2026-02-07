// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from more_interfaces:msg/MavlinkPacket.idl
// generated code does not contain a copyright notice

#ifndef MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__STRUCT_HPP_
#define MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__STRUCT_HPP_

#include <rosidl_runtime_cpp/bounded_vector.hpp>
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>


#ifndef _WIN32
# define DEPRECATED__more_interfaces__msg__MavlinkPacket __attribute__((deprecated))
#else
# define DEPRECATED__more_interfaces__msg__MavlinkPacket __declspec(deprecated)
#endif

namespace more_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct MavlinkPacket_
{
  using Type = MavlinkPacket_<ContainerAllocator>;

  explicit MavlinkPacket_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit MavlinkPacket_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _payload_type =
    std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other>;
  _payload_type payload;

  // setters for named parameter idiom
  Type & set__payload(
    const std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other> & _arg)
  {
    this->payload = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    more_interfaces::msg::MavlinkPacket_<ContainerAllocator> *;
  using ConstRawPtr =
    const more_interfaces::msg::MavlinkPacket_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      more_interfaces::msg::MavlinkPacket_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      more_interfaces::msg::MavlinkPacket_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__more_interfaces__msg__MavlinkPacket
    std::shared_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__more_interfaces__msg__MavlinkPacket
    std::shared_ptr<more_interfaces::msg::MavlinkPacket_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const MavlinkPacket_ & other) const
  {
    if (this->payload != other.payload) {
      return false;
    }
    return true;
  }
  bool operator!=(const MavlinkPacket_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct MavlinkPacket_

// alias to use template instance with default allocator
using MavlinkPacket =
  more_interfaces::msg::MavlinkPacket_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace more_interfaces

#endif  // MORE_INTERFACES__MSG__DETAIL__MAVLINK_PACKET__STRUCT_HPP_
