#ifndef RTUTOOL_HPP_
#define RTUTOOL_HPP_

#include <boost/asio/io_context.hpp>

class RtuTool
{
public:
    explicit RtuTool(int aArgc, char* aArgv[]);

    void Run();
private:
    void PrintWelcomeMessage() const;

private:
    boost::asio::io_context mContext;
};

#endif
