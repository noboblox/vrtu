#ifndef IEC104_SERVER_HPP_
#define IEC104_SERVER_HPP_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdint>
#include <memory>
#include <vector>

#include "protocols/iec104/link.hpp"

namespace IEC104
{
    class Apdu;

    class Server
    {
    public:
        explicit Server(boost::asio::io_context& arContext, const boost::asio::ip::address& arIP, uint16_t aPort = 2404);
        void Start();

        CORE::SignalEveryone<void, Link&> SignalConnected;
        CORE::SignalEveryone<void, Link&> SignalDisconnected;
        CORE::SignalEveryone<void, Link&> SignalStartDT;
        CORE::SignalEveryone<void, Link&> SignalStopDT;

        CORE::SignalEveryone<void, Link&, const Apdu&> SignalApdu;
        CORE::SignalEveryone<void, Link&, const Asdu&> SignalAsdu;

    private:
        void StartAccept();
        void FinishAccept(boost::system::error_code aError, boost::asio::ip::tcp::socket&& arNewSocket);

        void OnConnectionClosed(Link& arClosed);
        void DeleteConnection(Link& apClosed);

    private:
        boost::asio::io_context& mrContext;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> mpAcceptingSocket;

        std::vector<std::unique_ptr<Link>> mConnections;
    };
}
#endif

