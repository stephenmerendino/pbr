#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message.hpp"
#include <memory>

NetPacket::NetPacket()
    :m_payload_bytes_used(0)
    ,m_msg_cursor(nullptr)
{
    memset(m_payload, 0, PACKET_MTU);

    packet_header_t* header = get_packet_header();
    header->from_conn_idx = 0xFF;
    header->packet_ack = 0xFFFF;
    header->reliable_bundle_count = 0;
    header->unreliable_bundle_count = 0;

    m_msg_cursor = m_payload + sizeof(packet_header_t);
    m_payload_bytes_used += sizeof(packet_header_t);
}

bool NetPacket::write(NetMessage* msg)
{
    // write this net msg into the payload buffer IF there is enough room
    uint bytes_remaining = get_free_byte_count();
    uint msg_total_size = msg->get_full_size();
    if(msg_total_size > bytes_remaining){
        return false;
    }

    // increment payload size
    u8* dest = m_payload + m_payload_bytes_used;
    msg->write_to(dest);

    // track packet usage
    m_payload_bytes_used += msg_total_size;
    increment_msg_count();

    return true;
}

bool NetPacket::read(NetMessage* out_msg)
{
    // make sure we stil have messages to read
    u8 msg_count = get_messages_count();
    if(msg_count == 0){
        return false;
    }

    // track using a temporary internal cursor
    u8* internal_cursor = m_msg_cursor;

    // get body size
    u16 body_size = *internal_cursor;
    internal_cursor += sizeof(body_size);

    // get msg id
    u8 msg_id = *(internal_cursor);
    out_msg->m_message_type_id = msg_id;
    internal_cursor += sizeof(msg_id);

    // get payload
    u16 payload_size = body_size - sizeof(msg_id);
    memcpy(out_msg->m_payload, internal_cursor, payload_size);
    out_msg->m_payload_bytes_used = payload_size;

    // get sender
    out_msg->m_udp_sender = m_sender;

    // update actual cursor
    internal_cursor += payload_size;
    m_msg_cursor = internal_cursor;

    // update header for number of msg's left
    decrement_msg_count();

    return true;
}

packet_header_t* NetPacket::get_packet_header()
{
    return (packet_header_t*)m_payload;
}

void NetPacket::set_messages_count(u8 msg_count)
{
    packet_header_t* packet_header = get_packet_header();
    packet_header->unreliable_bundle_count = msg_count;
}

u8 NetPacket::get_messages_count()
{
    packet_header_t* packet_header = get_packet_header();
    return packet_header->unreliable_bundle_count;
}

void NetPacket::increment_msg_count()
{
    u8 cur_msg_count = get_messages_count();
    set_messages_count(cur_msg_count + 1);
}

void NetPacket::decrement_msg_count()
{
    u8 cur_msg_count = get_messages_count();
    set_messages_count(cur_msg_count - 1);
}

uint NetPacket::get_free_byte_count()
{
    return (uint)(PACKET_MTU - m_payload_bytes_used);
}