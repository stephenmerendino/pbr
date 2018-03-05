#pragma once

#include "Engine/Net/net_address.hpp"

class NetMessage;
class NetSession;

class NetConnection
{
    public:
        NetSession* m_owner;
        net_address_t m_address;
        uint8_t m_connection_index;

    public:
        NetConnection();
        virtual ~NetConnection();

        virtual void send(NetMessage *msg) = 0;
        virtual bool receive(NetMessage **msg) = 0;
        virtual bool is_disconnected() const = 0;
};