#include "protocols/iec104/server.hpp"

#include <iostream>

namespace IEC104
{
    Server::Server(boost::asio::io_context& arContext, const boost::asio::ip::address& arIP, uint16_t aListeningPort)
        : mrContext(arContext), mpAcceptingSocket(nullptr)
    {
        // A tcp endpoint for a specific interface
        boost::asio::ip::tcp::endpoint server_endpoint(arIP, aListeningPort);
        mpAcceptingSocket.reset(new boost::asio::ip::tcp::acceptor(mrContext, server_endpoint));

    }

    void Server::Start()
    {
        StartAccept();
    }


    void Server::StartAccept()
    {
        mpAcceptingSocket->async_accept(
            [this](boost::system::error_code aError, boost::asio::ip::tcp::socket&& arNewSocket)
            {
                this->FinishAccept(aError, std::move(arNewSocket));
            });
    }

    void Server::FinishAccept(boost::system::error_code aError, boost::asio::ip::tcp::socket&& arNewSocket)
    {
        if (!aError)
        {
            mConnections.emplace_back(new Link(mrContext, std::move(arNewSocket), ConnectionConfig::DefaultConnectionConfig, [this](Link& arConnection) {this->OnConnectionClosed(arConnection); }));

            Link& r_new_connection = **mConnections.rbegin();
            SignalConnected(r_new_connection);
            r_new_connection.Start();
            StartAccept();
        }
        else
        {
            std::cout << aError.message() << std::endl;
        }
    }

    void Server::OnConnectionClosed(Link& arClosed)
    {
        SignalDisconnected(arClosed);
        // Safely delete later, not now!
        boost::asio::post([this, &arClosed]() {this->DeleteConnection(arClosed); });
    }

    void Server::DeleteConnection(Link& apClosed)
    {
        auto it = mConnections.begin();

        for (; it != mConnections.end(); ++it)
        {
            if (it->get() == &apClosed)
            {
                mConnections.erase(it);
                break;
            }
        }
    }
}