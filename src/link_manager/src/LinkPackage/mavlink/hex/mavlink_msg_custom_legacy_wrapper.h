#pragma once
// MESSAGE CUSTOM_LEGACY_WRAPPER PACKING

#define MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER 51001


typedef struct __mavlink_custom_legacy_wrapper_t {
 uint8_t target_system; /*<  sys ID*/
 uint8_t target_component; /*<  target ID*/
 uint8_t length; /*<  data length*/
 uint8_t topic; /*<  topic ID*/
 uint8_t id; /*<  old protocol ID*/
 uint8_t payload[251]; /*<  data*/
} mavlink_custom_legacy_wrapper_t;

#define MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN 256
#define MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN 256
#define MAVLINK_MSG_ID_51001_LEN 256
#define MAVLINK_MSG_ID_51001_MIN_LEN 256

#define MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC 139
#define MAVLINK_MSG_ID_51001_CRC 139

#define MAVLINK_MSG_CUSTOM_LEGACY_WRAPPER_FIELD_PAYLOAD_LEN 251

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CUSTOM_LEGACY_WRAPPER { \
    51001, \
    "CUSTOM_LEGACY_WRAPPER", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_custom_legacy_wrapper_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_custom_legacy_wrapper_t, target_component) }, \
         { "length", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_custom_legacy_wrapper_t, length) }, \
         { "topic", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_custom_legacy_wrapper_t, topic) }, \
         { "id", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_custom_legacy_wrapper_t, id) }, \
         { "payload", NULL, MAVLINK_TYPE_UINT8_T, 251, 5, offsetof(mavlink_custom_legacy_wrapper_t, payload) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CUSTOM_LEGACY_WRAPPER { \
    "CUSTOM_LEGACY_WRAPPER", \
    6, \
    {  { "target_system", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_custom_legacy_wrapper_t, target_system) }, \
         { "target_component", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_custom_legacy_wrapper_t, target_component) }, \
         { "length", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_custom_legacy_wrapper_t, length) }, \
         { "topic", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_custom_legacy_wrapper_t, topic) }, \
         { "id", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_custom_legacy_wrapper_t, id) }, \
         { "payload", NULL, MAVLINK_TYPE_UINT8_T, 251, 5, offsetof(mavlink_custom_legacy_wrapper_t, payload) }, \
         } \
}
#endif

/**
 * @brief Pack a custom_legacy_wrapper message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  sys ID
 * @param target_component  target ID
 * @param length  data length
 * @param topic  topic ID
 * @param id  old protocol ID
 * @param payload  data
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t length, uint8_t topic, uint8_t id, const uint8_t *payload)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, length);
    _mav_put_uint8_t(buf, 3, topic);
    _mav_put_uint8_t(buf, 4, id);
    _mav_put_uint8_t_array(buf, 5, payload, 251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#else
    mavlink_custom_legacy_wrapper_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.length = length;
    packet.topic = topic;
    packet.id = id;
    mav_array_memcpy(packet.payload, payload, sizeof(uint8_t)*251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
}

/**
 * @brief Pack a custom_legacy_wrapper message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param target_system  sys ID
 * @param target_component  target ID
 * @param length  data length
 * @param topic  topic ID
 * @param id  old protocol ID
 * @param payload  data
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint8_t target_system, uint8_t target_component, uint8_t length, uint8_t topic, uint8_t id, const uint8_t *payload)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, length);
    _mav_put_uint8_t(buf, 3, topic);
    _mav_put_uint8_t(buf, 4, id);
    _mav_put_uint8_t_array(buf, 5, payload, 251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#else
    mavlink_custom_legacy_wrapper_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.length = length;
    packet.topic = topic;
    packet.id = id;
    mav_array_memcpy(packet.payload, payload, sizeof(uint8_t)*251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#endif
}

/**
 * @brief Pack a custom_legacy_wrapper message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param target_system  sys ID
 * @param target_component  target ID
 * @param length  data length
 * @param topic  topic ID
 * @param id  old protocol ID
 * @param payload  data
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t target_system,uint8_t target_component,uint8_t length,uint8_t topic,uint8_t id,const uint8_t *payload)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, length);
    _mav_put_uint8_t(buf, 3, topic);
    _mav_put_uint8_t(buf, 4, id);
    _mav_put_uint8_t_array(buf, 5, payload, 251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#else
    mavlink_custom_legacy_wrapper_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.length = length;
    packet.topic = topic;
    packet.id = id;
    mav_array_memcpy(packet.payload, payload, sizeof(uint8_t)*251);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
}

/**
 * @brief Encode a custom_legacy_wrapper struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param custom_legacy_wrapper C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_custom_legacy_wrapper_t* custom_legacy_wrapper)
{
    return mavlink_msg_custom_legacy_wrapper_pack(system_id, component_id, msg, custom_legacy_wrapper->target_system, custom_legacy_wrapper->target_component, custom_legacy_wrapper->length, custom_legacy_wrapper->topic, custom_legacy_wrapper->id, custom_legacy_wrapper->payload);
}

/**
 * @brief Encode a custom_legacy_wrapper struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param custom_legacy_wrapper C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_custom_legacy_wrapper_t* custom_legacy_wrapper)
{
    return mavlink_msg_custom_legacy_wrapper_pack_chan(system_id, component_id, chan, msg, custom_legacy_wrapper->target_system, custom_legacy_wrapper->target_component, custom_legacy_wrapper->length, custom_legacy_wrapper->topic, custom_legacy_wrapper->id, custom_legacy_wrapper->payload);
}

/**
 * @brief Encode a custom_legacy_wrapper struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param custom_legacy_wrapper C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_custom_legacy_wrapper_t* custom_legacy_wrapper)
{
    return mavlink_msg_custom_legacy_wrapper_pack_status(system_id, component_id, _status, msg,  custom_legacy_wrapper->target_system, custom_legacy_wrapper->target_component, custom_legacy_wrapper->length, custom_legacy_wrapper->topic, custom_legacy_wrapper->id, custom_legacy_wrapper->payload);
}

/**
 * @brief Send a custom_legacy_wrapper message
 * @param chan MAVLink channel to send the message
 *
 * @param target_system  sys ID
 * @param target_component  target ID
 * @param length  data length
 * @param topic  topic ID
 * @param id  old protocol ID
 * @param payload  data
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_custom_legacy_wrapper_send(mavlink_channel_t chan, uint8_t target_system, uint8_t target_component, uint8_t length, uint8_t topic, uint8_t id, const uint8_t *payload)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN];
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, length);
    _mav_put_uint8_t(buf, 3, topic);
    _mav_put_uint8_t(buf, 4, id);
    _mav_put_uint8_t_array(buf, 5, payload, 251);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER, buf, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#else
    mavlink_custom_legacy_wrapper_t packet;
    packet.target_system = target_system;
    packet.target_component = target_component;
    packet.length = length;
    packet.topic = topic;
    packet.id = id;
    mav_array_memcpy(packet.payload, payload, sizeof(uint8_t)*251);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER, (const char *)&packet, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#endif
}

/**
 * @brief Send a custom_legacy_wrapper message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_custom_legacy_wrapper_send_struct(mavlink_channel_t chan, const mavlink_custom_legacy_wrapper_t* custom_legacy_wrapper)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_custom_legacy_wrapper_send(chan, custom_legacy_wrapper->target_system, custom_legacy_wrapper->target_component, custom_legacy_wrapper->length, custom_legacy_wrapper->topic, custom_legacy_wrapper->id, custom_legacy_wrapper->payload);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER, (const char *)custom_legacy_wrapper, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#endif
}

#if MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_custom_legacy_wrapper_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t target_system, uint8_t target_component, uint8_t length, uint8_t topic, uint8_t id, const uint8_t *payload)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 0, target_system);
    _mav_put_uint8_t(buf, 1, target_component);
    _mav_put_uint8_t(buf, 2, length);
    _mav_put_uint8_t(buf, 3, topic);
    _mav_put_uint8_t(buf, 4, id);
    _mav_put_uint8_t_array(buf, 5, payload, 251);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER, buf, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#else
    mavlink_custom_legacy_wrapper_t *packet = (mavlink_custom_legacy_wrapper_t *)msgbuf;
    packet->target_system = target_system;
    packet->target_component = target_component;
    packet->length = length;
    packet->topic = topic;
    packet->id = id;
    mav_array_memcpy(packet->payload, payload, sizeof(uint8_t)*251);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER, (const char *)packet, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_MIN_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_CRC);
#endif
}
#endif

#endif

// MESSAGE CUSTOM_LEGACY_WRAPPER UNPACKING


/**
 * @brief Get field target_system from custom_legacy_wrapper message
 *
 * @return  sys ID
 */
static inline uint8_t mavlink_msg_custom_legacy_wrapper_get_target_system(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field target_component from custom_legacy_wrapper message
 *
 * @return  target ID
 */
static inline uint8_t mavlink_msg_custom_legacy_wrapper_get_target_component(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field length from custom_legacy_wrapper message
 *
 * @return  data length
 */
static inline uint8_t mavlink_msg_custom_legacy_wrapper_get_length(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field topic from custom_legacy_wrapper message
 *
 * @return  topic ID
 */
static inline uint8_t mavlink_msg_custom_legacy_wrapper_get_topic(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Get field id from custom_legacy_wrapper message
 *
 * @return  old protocol ID
 */
static inline uint8_t mavlink_msg_custom_legacy_wrapper_get_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field payload from custom_legacy_wrapper message
 *
 * @return  data
 */
static inline uint16_t mavlink_msg_custom_legacy_wrapper_get_payload(const mavlink_message_t* msg, uint8_t *payload)
{
    return _MAV_RETURN_uint8_t_array(msg, payload, 251,  5);
}

/**
 * @brief Decode a custom_legacy_wrapper message into a struct
 *
 * @param msg The message to decode
 * @param custom_legacy_wrapper C-struct to decode the message contents into
 */
static inline void mavlink_msg_custom_legacy_wrapper_decode(const mavlink_message_t* msg, mavlink_custom_legacy_wrapper_t* custom_legacy_wrapper)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    custom_legacy_wrapper->target_system = mavlink_msg_custom_legacy_wrapper_get_target_system(msg);
    custom_legacy_wrapper->target_component = mavlink_msg_custom_legacy_wrapper_get_target_component(msg);
    custom_legacy_wrapper->length = mavlink_msg_custom_legacy_wrapper_get_length(msg);
    custom_legacy_wrapper->topic = mavlink_msg_custom_legacy_wrapper_get_topic(msg);
    custom_legacy_wrapper->id = mavlink_msg_custom_legacy_wrapper_get_id(msg);
    mavlink_msg_custom_legacy_wrapper_get_payload(msg, custom_legacy_wrapper->payload);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN? msg->len : MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN;
        memset(custom_legacy_wrapper, 0, MAVLINK_MSG_ID_CUSTOM_LEGACY_WRAPPER_LEN);
    memcpy(custom_legacy_wrapper, _MAV_PAYLOAD(msg), len);
#endif
}
