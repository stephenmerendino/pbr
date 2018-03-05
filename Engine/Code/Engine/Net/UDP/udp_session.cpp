#include "Engine/Net/UDP/udp_session.hpp"
#include "Engine/Net/UDP/udp_socket.hpp"
#include "Engine/Net/net_address.hpp"
#include "Engine/Net/net_packet.hpp"
#include "Engine/Net/message.hpp"
#include "Engine/Net/message_definition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define MAX_ADDRESS_ATTEMPTS 8

UDPSession::UDPSession()
    :m_socket(nullptr)
{
}

UDPSession::~UDPSession()
{
    SAFE_DELETE(m_socket);
}

bool UDPSession::host(uint16_t port)
{
    UNUSED(port);
    return false;
}

bool UDPSession::join(const net_address_t& address)
{
    UNUSED(address);
    return false;
}

void UDPSession::leave()
{
}

void UDPSession::update()
{
    // handle incoming messages appropriately
    NetPacket packet;

    while(m_socket->receive(&packet)){
        NetMessage msg;
        while(packet.read(&msg)){
            process_message(&msg);
        }
    }
}

void UDPSession::process_message(NetMessage* msg)
{
    if(nullptr == msg){
        return;
    }

    NetMessageDefinition* def = get_message_definition(msg->m_message_type_id);
    ASSERT_OR_DIE(nullptr != def, Stringf("Could not find a definition for message id %i", msg->m_message_type_id));

    def->handle(msg);
}

bool UDPSession::start_listening()
{
    return false;
}

void UDPSession::stop_listening()
{
}

bool UDPSession::is_listening()
{
    return false;
}

void UDPSession::set_state(SessionState new_state)
{
    UNUSED(new_state);
}

bool UDPSession::start(uint16_t port)
{
    if(nullptr != m_socket){
        return false;
    }

    UDPSocket* socket = new UDPSocket();

    unsigned int attempts_left = MAX_ADDRESS_ATTEMPTS;
    while(attempts_left > 0){
        if(socket->bind(port)){
            m_socket = socket;
            return true;
        }else{
            attempts_left--;
            port++;
        }
    }

    SAFE_DELETE(socket);
    return false;
}

void UDPSession::send_direct(const net_address_t& dest_addr, NetMessage* msg)
{
    // build a netpacket and send directly to address using socket
    NetPacket packet;
    packet.write(msg);
    m_socket->send(dest_addr, packet.m_payload, packet.m_payload_bytes_used);
}