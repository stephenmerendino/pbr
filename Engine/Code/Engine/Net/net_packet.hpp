#pragma once

#include "Engine/Net/net_address.hpp"
#include "Engine/Core/types.h"

class NetMessage;

#define PACKET_MTU 1452

struct packet_header_t
{
    u8 from_conn_idx;
    u16 packet_ack;
    u8 reliable_bundle_count;
    u8 unreliable_bundle_count;
};

class NetPacket
{
    public:
        net_address_t m_sender;

        byte m_payload[PACKET_MTU];
        uint m_payload_bytes_used;
        byte* m_msg_cursor;

    public:
        NetPacket(); 

        bool write(NetMessage* msg);
        bool read(NetMessage* out_msg);

        packet_header_t* get_packet_header();
        void set_messages_count(u8 msg_count);
        u8 get_messages_count();
        void increment_msg_count();
        void decrement_msg_count();

        uint get_free_byte_count();
};