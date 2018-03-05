#include "Engine/Net/message.hpp"
#include "Engine/Core/types.h"

NetMessage::NetMessage()
    :m_sender(nullptr)
    ,m_payload_bytes_used(0)
    ,m_payload_bytes_read(0)
{
    m_stream_order = LITTLE_ENDIAN;
    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
}

NetMessage::NetMessage(uint8_t msg_type_id)
    :m_message_type_id(msg_type_id)
    ,m_sender(nullptr)
    ,m_payload_bytes_used(0)
    ,m_payload_bytes_read(0)
{
    m_stream_order = LITTLE_ENDIAN;
    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
}

NetMessage::NetMessage(const NetMessage& copy)
    :m_message_type_id(copy.m_message_type_id)
    ,m_sender(copy.m_sender)
    ,m_payload_bytes_used(copy.m_payload_bytes_used)
    ,m_payload_bytes_read(0)
{
    m_stream_order = LITTLE_ENDIAN;
    memcpy(m_payload, copy.m_payload, m_payload_bytes_used);
}

u32 NetMessage::write_bytes(void* bytes, size_t count)
{
    if(m_payload_bytes_used + count > MAX_PAYLOAD_SIZE){
        return 0;
    }

    byte_t* write_location = m_payload + m_payload_bytes_used;
    memcpy(write_location, bytes, count);

    m_payload_bytes_used += count;

    return (u32)count;
}

u32 NetMessage::read_bytes(void* out_bytes, size_t count)
{
    memcpy(out_bytes, m_payload + m_payload_bytes_read, count);
    m_payload_bytes_read += count;
    return (u32)count;
}

void NetMessage::write_string(const char* string)
{
    if(nullptr == string){
        write<uint16_t>(0xFFFF);
        return;
    }

    size_t string_length = strlen(string);    
    write<uint16_t>((uint16_t)string_length);

    if(string_length > 0){
        write_endian_aware_bytes((void*)string, (unsigned int)string_length);
    }
}

bool NetMessage::read_string(char* out_string, unsigned int max_size)
{
    if(has_read_all_data()){
        return nullptr;
    }

    uint16_t string_length;
    read<uint16_t>(string_length);

    if(string_length > max_size){
        string_length = (uint16_t)max_size;
    }

    if(0xFFFF == string_length){
        memset(out_string, 0, max_size);
        return false;
    }else if(0 == string_length){
        memset(out_string, 0, max_size);
        return false;
    }else{
        read(out_string, string_length);
        out_string[string_length] = '\0';
        return true;
    }
}

bool NetMessage::has_read_all_data() const
{
    return (m_payload_bytes_read >= m_payload_bytes_used);
}

void NetMessage::write_to(u8* dest)
{
    u16 body_size = get_body_size();
    memcpy(dest, &body_size, sizeof(body_size));
    dest += sizeof(body_size);

    memcpy(dest, &m_message_type_id, sizeof(m_message_type_id));
    dest += sizeof(m_message_type_id);

    memcpy(dest, m_payload, m_payload_bytes_used);
}

u16 NetMessage::get_body_size()
{
    return (u16)(sizeof(m_message_type_id) + m_payload_bytes_used);
}

u16 NetMessage::get_full_size()
{
    u16 body_size = get_body_size();
    return body_size + sizeof(body_size); // the header of the message is the body size
}

void NetMessage::reset()
{
    m_sender = nullptr;
    m_payload_bytes_used = 0;
    m_payload_bytes_read = 0;
    memset(m_payload, 0, MAX_PAYLOAD_SIZE);
}