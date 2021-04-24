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


namespace IP = boost::asio::ip;

namespace IEC104
{
    class Connection
    {
    public:

        explicit Connection(boost::asio::ip::tcp::socket&& arSocket)
            : mSocket(std::move(arSocket)), mReadBuffer(), mWriteBuffer(),
              mCurrentSize(0),
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

            boost::asio::write(mSocket, boost::asio::buffer(send.GetData(), IEC104::Apdu::GetHeaderSize()), ec);
            return !ec.failed();
        }

        void ReceiveNextMessage()
        {
            mCurrentSize = 0;
            ReadPartial(mReadBuffer.data(), IEC104::Apdu::GetHeaderSize());
        }

        void ReadPartial(void* apDestination, size_t aTargetSize)
        {
            mSocket.async_read_some(boost::asio::buffer(apDestination, aTargetSize),
                [this](const boost::system::error_code& arError, size_t aBytesReceived) {this->OnBytesReceived(arError, aBytesReceived); });
        }

        void OnBytesReceived(const boost::system::error_code& arError, size_t aBytesReceived)
        {
            if (arError)
            {
                // TODO Report and disconnect
                return;
            }

            mCurrentSize += aBytesReceived;

            if (mCurrentSize < IEC104::Apdu::GetHeaderSize())
                ReadPartial(&mReadBuffer[mCurrentSize], IEC104::Apdu::GetHeaderSize() - mCurrentSize);
            else if (mCurrentSize < IEC104::Apdu::GetMessageSize(mReadBuffer.data()))
                ReadPartial(&mReadBuffer[mCurrentSize], IEC104::Apdu::GetMessageSize(mReadBuffer.data()) - mCurrentSize);
            else if (mCurrentSize == IEC104::Apdu::GetMessageSize(mReadBuffer.data()))
            {
                ProcessMessage();
                ReceiveNextMessage();
            }

            return; // Error
        }


        bool ProcessMessage()
        {
            ByteStream msg;
            msg.WriteData(mReadBuffer.data(), mCurrentSize);
            mReceived.ReadFrom(msg);

            if (!mReceived.IsValid() || !HandleSequences(mReceived))
                return false;

            PrintMessage(mReceived, false);

            if (mReceived.NeedsConfirmation())
            {
                mReceived.ConvertToConfirmation();
                PrintMessage(mReceived, true);
                boost::asio::write(mSocket,
                    boost::asio::buffer(mReceived.GetData(), mReceived.GetHeaderSize()));
            }
            else if (mReceived.HasAsdu())
            {
                ConfirmReceive();
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
            ReceiveNextMessage();
        }
         
    private:
        boost::asio::ip::tcp::socket mSocket;
        std::array<uint8_t, 256> mReadBuffer, mWriteBuffer;
        int mCurrentSize;
        int mNextReceiveId, mNextSendId, mLastAcknoledgedId;
        Apdu mReceived;
    };

    class Server
    {
    public:
        explicit Server(boost::asio::io_context& arContext, const IP::address& arIP, uint16_t aListeningPort = 2404)
            : mrContext(arContext), mpAcceptingSocket(nullptr)
        {
            // A tcp endpoint for a specific interface
            boost::asio::ip::tcp::endpoint server_endpoint(arIP, aListeningPort);
            mpAcceptingSocket.reset(new IP::tcp::acceptor(mrContext, server_endpoint));

            std::cout << "Waiting for connections on " << arIP << ":" << aListeningPort << std::endl;
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
                mConnections.emplace_back(Connection(std::move(arNewSocket)));
                mConnections.rbegin()->Start();
                StartAccept();
            }
            else
            {
                std::cout << aError.message() << std::endl;
            }
        }

    private:
        boost::asio::io_context& mrContext;
        std::unique_ptr<IP::tcp::acceptor> mpAcceptingSocket;

        std::vector<Connection> mConnections;
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
