#pragma once

#include "Engine/Core/BinaryStream.hpp"
#include "Engine/Core/types.h"
#include "Engine/Net/net_address.hpp"

class NetConnection;

#define MAX_PAYLOAD_SIZE 1024

class NetMessage : public BinaryStream
{
    public:
        uint8_t m_message_type_id;
        NetConnection* m_sender;
        byte_t m_payload[MAX_PAYLOAD_SIZE];
        size_t m_payload_bytes_used;
        size_t m_payload_bytes_read;

        //#TODO, get rid of this once we get more UDP infrastructure
        net_address_t m_udp_sender;

    public:
        NetMessage();
        NetMessage(uint8_t msg_type_id);
        NetMessage(const NetMessage& copy);

    	virtual u32 write_bytes(void* bytes, size_t count) override;
    	virtual u32 read_bytes(void* out_bytes, size_t count) override;

        void write_string(const char* string);
        bool read_string(char* out_string, unsigned int max_size);
        bool has_read_all_data() const;

        void write_to(u8* dest);
        u16 get_body_size();
        u16 get_full_size();

        void reset();
};