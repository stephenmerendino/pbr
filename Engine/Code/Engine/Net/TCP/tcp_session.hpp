#pragma once

#include "Engine/Net/session.hpp"
#include "Engine/Net/TCP/tcp_connection.hpp"

class TCPSocket;

class TCPSession : public NetSession
{
    public:
        TCPSocket* m_listen_socket;

    public:
        TCPSession();
        virtual ~TCPSession();

    public:
        virtual bool host(uint16_t port) override;
        virtual bool join(const net_address_t& address) override;
        virtual void leave() override;
        virtual void update() override;

        virtual bool start_listening() override;
        virtual void stop_listening() override;
        virtual bool is_listening() override;

    public:
        void process_message(NetMessage* msg);
        void send_join_info(NetConnection* connection);
        void on_join_response(NetMessage* msg);

        unsigned int get_number_of_live_clients() const;
};