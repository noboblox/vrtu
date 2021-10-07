#ifndef IEC104_SERVER_HPP_
#define IEC104_SERVER_HPP_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdint>
#include <memory>
#include <vector>

#include "protocols/iec104/connection.hpp"

namespace IEC104
{
    class Apdu;

    class Server
    {
    public:
        explicit Server(boost::asio::io_context& arContext, const boost::asio::ip::address& arIP, uint16_t aListeningPort = 2404);
        void Start();

        CORE::SignalEveryone<void, Connection&> SignalConnected;
        CORE::SignalEveryone<void, Connection&> SignalDisconnected;
        CORE::SignalEveryone<void, Connection&> SignalStartDT;
        CORE::SignalEveryone<void, Connection&> SignalStopDT;

        CORE::SignalEveryone<void, Connection&, const Apdu&> SignalApdu;
        CORE::SignalEveryone<void, Connection&, const Asdu&> SignalAsdu;

    private:
        void StartAccept();
        void FinishAccept(boost::system::error_code aError, boost::asio::ip::tcp::socket&& arNewSocket);

        void OnConnectionClosed(Connection& arClosed);
        void DeleteConnection(Connection& apClosed);

    private:
        boost::asio::io_context& mrContext;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> mpAcceptingSocket;

        std::vector<std::unique_ptr<Connection>> mConnections;
    };
}
#endif

