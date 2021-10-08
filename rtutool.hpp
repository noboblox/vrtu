#ifndef RTUTOOL_HPP_
#define RTUTOOL_HPP_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>

class RtuTool
{
public:
    explicit RtuTool(int aArgc, char* aArgv[]);

    void Run();
private:
    void PrintWelcomeMessage() const;
    void ReadArguments(int argc, char* argv[]);

private:
    boost::asio::io_context mContext;
    boost::asio::ip::address mIP;
};

#endif
