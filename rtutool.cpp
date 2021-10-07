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

RtuTool::RtuTool(int, char*[])
{
    // TODO Do something with args
}

void RtuTool::Run()
{
    PrintWelcomeMessage();
    IEC104::Server server(mContext, boost::asio::ip::make_address("127.0.0.1"));
    mContext.run();
}

void RtuTool::PrintWelcomeMessage() const
{
    std::cout << "Welcome to RTU tool\n"
              << "A tool to analyze and test IEC 60870-5-104 traffic" << std::endl;
}
