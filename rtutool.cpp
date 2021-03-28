#include "rtutool.hpp"

#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "core/bytestream.hpp"
#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/asdu.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace IEC104
{
    class Connection
    {
    public:

        explicit Connection(boost::asio::ip::tcp::socket&& arSocket)
            : mSocket(std::move(arSocket)), mReadBuffer(), mWriteBuffer(),
              mNextReceiveId(0), mNextSendId(0), mLastAcknoledgedId(0)
        {
        }

        bool HandleSequences(const IEC104::Apdu& arReceived)
        {
            bool success = true;

            if (arReceived.HasSendCounter())
            {
                if (mNextReceiveId == arReceived.GetSendCounter())
                    IEC104::Apdu::IncrementSequence(mNextReceiveId);
                else
                    success = false;
            }

            if (arReceived.HasReceiveCounter())
            {
                mLastAcknoledgedId = arReceived.GetReceiveCounter();
            }
            return success;
        }

        bool ConfirmReceive()
        {
            ReceiveConfirmation send(mNextReceiveId);
            boost::system::error_code ec;
            PrintMessage(send, true);

            boost::asio::write(mSocket, boost::asio::buffer(send.GetData(), IEC104::Apdu::GetNeededSize()), ec);
            return !ec.failed();
        }

        bool ReceiveMessage()
        {
            boost::system::error_code ec;
            boost::asio::read(mSocket, 
                boost::asio::buffer(mReadBuffer, IEC104::Apdu::GetNeededSize()), ec);

            if (ec)
                return false;

            mReceived.Assign(mReadBuffer.data(), IEC104::Apdu::GetNeededSize());

            if (!mReceived.IsValid() || !HandleSequences(mReceived))
                return false;

            const int asdu_size = mReceived.GetAsduSize();
            
            if (asdu_size)
            {
                boost::asio::read(mSocket,
                    boost::asio::buffer(mReadBuffer, asdu_size), ec);

                if (ec)
                    return false;

                ByteStream asdu_data;
                asdu_data.WriteData(mReadBuffer.data(), asdu_size);

                if (!mReceived.ReadAsdu(asdu_data))
                    return false;
            }

            return true;
        }

        void PrintMessage(IEC104::Apdu& arMessage, bool aIsSend)
        {

            std::cout << "[" << (aIsSend ? "--> " : "<-- ") << mSocket.remote_endpoint().address().to_string() << ":" << std::setw(5) << mSocket.remote_endpoint().port() << "]" 
                      << " [" << std::setw(12) << std::left << arMessage.TypeToString() << "]";

            if (aIsSend)
            {
                std::cout << " r = " << std::setw(5) << (arMessage.HasReceiveCounter() ? std::to_string(arMessage.GetReceiveCounter()) : std::string(""))
                    << ", s = " << std::setw(5) << (arMessage.HasSendCounter() ? std::to_string(arMessage.GetSendCounter()) : std::string(""));
            }
            else
            {
                std::cout << " s = " << std::setw(5) << (arMessage.HasSendCounter() ? std::to_string(arMessage.GetSendCounter()) : std::string(""))
                    << ", r = " << std::setw(5) << (arMessage.HasReceiveCounter() ? std::to_string(arMessage.GetReceiveCounter()) : std::string(""));
            }

                
            std::cout << ", ASDU size = " << arMessage.GetAsduSize() << ";" << std::endl;
        }
        void Start()
        {
            std::cout << "Connected to " << mSocket.remote_endpoint().address().to_string() << ":" << mSocket.remote_endpoint().port() << std::endl;

            while (true)
            {
                if (!ReceiveMessage())
                    return; // Error

                PrintMessage(mReceived, false);

                if (mReceived.NeedsConfirmation())
                {
                    mReceived.ConvertToConfirmation();
                    PrintMessage(mReceived, true);
                    boost::asio::write(mSocket, 
                        boost::asio::buffer(mReceived.GetData(), IEC104::Apdu::GetNeededSize()));
                }
                else if (mReceived.HasAsdu())
                {
                    ConfirmReceive();
                }
            }
        }
         
    private:
        boost::asio::ip::tcp::socket mSocket;
        std::array<uint8_t, 256> mReadBuffer, mWriteBuffer;
        int mNextReceiveId, mNextSendId, mLastAcknoledgedId;
        Apdu mReceived;
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
    boost::system::error_code error;

    boost::asio::ip::tcp::endpoint server_endpoint(boost::asio::ip::tcp::v4(), 2404);
    boost::asio::ip::tcp::acceptor server_accept(mContext, server_endpoint);
    boost::asio::ip::tcp::socket server_socket(mContext);
    server_accept.accept(server_socket, error);
    
    IEC104::Connection slave(std::move(server_socket));
    slave.Start();

    // TODO Errors

    std::cout << "Disconnected" << std::endl;
}

void RtuTool::PrintWelcomeMessage() const
{
    std::cout << "Welcome to RTU tool\n"
              << "A tool to analyze and test IEC 60870-5-104 traffic" << std::endl;
}
