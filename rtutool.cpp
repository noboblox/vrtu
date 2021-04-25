#include "rtutool.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "protocols/iec104/connection.hpp"


namespace IP = boost::asio::ip;

namespace IEC104
{
    class Server
    {
    public:
        explicit Server(boost::asio::io_context& arContext, const IP::address& arIP, uint16_t aListeningPort = 2404)
            : mrContext(arContext), mpAcceptingSocket(nullptr)
        {
            // A tcp endpoint for a specific interface
            boost::asio::ip::tcp::endpoint server_endpoint(arIP, aListeningPort);
            mpAcceptingSocket.reset(new IP::tcp::acceptor(mrContext, server_endpoint));

            std::cout << "Server socket listening on " << arIP << ":" << aListeningPort << std::endl;
            StartAccept();
        }


    private:
        void StartAccept()
        {
            mpAcceptingSocket->async_accept(
                [this](boost::system::error_code aError, IP::tcp::socket&& arNewSocket)
                {
                    this->FinishAccept(aError, std::move(arNewSocket));
                });
        }

        void FinishAccept(boost::system::error_code aError, IP::tcp::socket&& arNewSocket)
        {
            if (!aError)
            {
                mConnections.emplace_back(new Connection(std::move(arNewSocket), [this](Connection& arConnection) {this->OnConnectionClosed(arConnection); } ));
                (*mConnections.rbegin())->Start();
                StartAccept();
            }
            else
            {
                std::cout << aError.message() << std::endl;
            }
        }
        
        void OnConnectionClosed(Connection& arClosed)
        {
            // Safely delete later, not now!
            boost::asio::post([this, &arClosed]() {this->DeleteConnection(arClosed); });
        }

        void DeleteConnection(Connection& apClosed)
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

    private:
        boost::asio::io_context& mrContext;
        std::unique_ptr<IP::tcp::acceptor> mpAcceptingSocket;

        std::vector<std::unique_ptr<Connection>> mConnections;
    };

}
int main(int argc, char* argv[])
{
    try
    {
        RtuTool app(argc, argv);
        app.Run();
        return 0;
    }
    catch (std::exception& e) { std::cout << "An unhandled error occured: " << e.what() << std::endl; }
    catch (...)               { std::cout << "An unhandled and unknown error occured" << std::endl; }
    return -1;
}

RtuTool::RtuTool(int aArgc, char* aArgv[])
{
    // TODO Do something with args
}

void RtuTool::Run()
{
    PrintWelcomeMessage();
    IEC104::Server server(mContext, IP::make_address("127.0.0.1"));
    mContext.run();
}

void RtuTool::PrintWelcomeMessage() const
{
    std::cout << "Welcome to RTU tool\n"
              << "A tool to analyze and test IEC 60870-5-104 traffic" << std::endl;
}
