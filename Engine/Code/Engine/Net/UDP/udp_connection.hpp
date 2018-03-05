#pragma once

#include "Engine/Net/connection.hpp"

class UDPConnection : public NetConnection
{
    public:
        UDPConnection();
        virtual ~UDPConnection();

        virtual void send(NetMessage *msg) override;
        virtual bool receive(NetMessage **msg) override;
        virtual bool is_disconnected() const override;
};