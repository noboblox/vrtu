#include "rtutool.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "protocols/iec104/server.hpp"


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

RtuTool::RtuTool(int argc, char* argv[])
    : mIP(boost::asio::ip::make_address("127.0.0.1"))
{
    ReadArguments(argc, argv);
}

void RtuTool::Run()
{
    PrintWelcomeMessage();
    IEC104::Server server(mContext, mIP);
    server.Start();
    mContext.run();
}

void RtuTool::PrintWelcomeMessage() const
{
    std::cout << "Welcome to RTU tool\n"
              << "A tool to analyze and test IEC 60870-5-104 traffic" << std::endl;
}

void RtuTool::ReadArguments(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--ip") == 0)
        {
            ++i;
            if (i < argc)
            {
                boost::system::error_code ec;
                auto ip = boost::asio::ip::make_address(argv[i], ec);

                if (ec)
                {
                    std::cout << argv[i] << " is not a valid IP address" << std::endl;
                }
                else
                {
                    mIP = ip;
                }
            }
        }
    }
}
