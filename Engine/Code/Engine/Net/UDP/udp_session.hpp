#pragma once

#include "Engine/Net/session.hpp"

class UDPSocket;
class NetMessage;

class UDPSession : public NetSession
{
    public:
        UDPSocket* m_socket;

    public:
        UDPSession();
        virtual ~UDPSession();

    public:
        virtual bool host(uint16_t port) override;
        virtual bool join(const net_address_t& address) override;
        virtual void leave() override;
        virtual void update() override;

        virtual bool start_listening() override;
        virtual void stop_listening() override;
        virtual bool is_listening() override;

        virtual void set_state(SessionState new_state);

    public:
        void process_message(NetMessage* msg);
        bool start(uint16_t port);
        void send_direct(const net_address_t& dest_addr, NetMessage* msg);
};