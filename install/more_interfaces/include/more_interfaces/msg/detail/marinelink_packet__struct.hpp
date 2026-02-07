// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from more_interfaces:msg/MarinelinkPacket.idl
// generated code does not contain a copyright notice

#ifndef MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_HPP_
#define MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_HPP_

#include <rosidl_runtime_cpp/bounded_vector.hpp>
#include <rosidl_runtime_cpp/message_initialization.hpp>
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>


#ifndef _WIN32
# define DEPRECATED__more_interfaces__msg__MarinelinkPacket __attribute__((deprecated))
#else
# define DEPRECATED__more_interfaces__msg__MarinelinkPacket __declspec(deprecated)
#endif

namespace more_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct MarinelinkPacket_
{
  using Type = MarinelinkPacket_<ContainerAllocator>;

  explicit MarinelinkPacket_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->topic = 0;
    }
  }

  explicit MarinelinkPacket_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->topic = 0;
    }
  }

  // field types and members
  using _topic_type =
    uint8_t;
  _topic_type topic;
  using _payload_type =
    std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other>;
  _payload_type payload;

  // setters for named parameter idiom
  Type & set__topic(
    const uint8_t & _arg)
  {
    this->topic = _arg;
    return *this;
  }
  Type & set__payload(
    const std::vector<uint8_t, typename ContainerAllocator::template rebind<uint8_t>::other> & _arg)
  {
    this->payload = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> *;
  using ConstRawPtr =
    const more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__more_interfaces__msg__MarinelinkPacket
    std::shared_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__more_interfaces__msg__MarinelinkPacket
    std::shared_ptr<more_interfaces::msg::MarinelinkPacket_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const MarinelinkPacket_ & other) const
  {
    if (this->topic != other.topic) {
      return false;
    }
    if (this->payload != other.payload) {
      return false;
    }
    return true;
  }
  bool operator!=(const MarinelinkPacket_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct MarinelinkPacket_

// alias to use template instance with default allocator
using MarinelinkPacket =
  more_interfaces::msg::MarinelinkPacket_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace more_interfaces

#endif  // MORE_INTERFACES__MSG__DETAIL__MARINELINK_PACKET__STRUCT_HPP_
