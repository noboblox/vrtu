#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <array>
#include <cstdint>
#include <functional>

#include <boost/asio/ip/tcp.hpp>

#include "protocols/iec104/apdu.hpp"

namespace IEC104
{
    class Connection
    {
    public:
        // Create an connection with an connected socket
        explicit Connection(boost::asio::ip::tcp::socket&& arSocket, const std::function<void(Connection&)>& arClosedHandler);
        ~Connection();

        // Start message processing 
        void Start();

    private:
        // Start an async read of the next APDU -> ReadPartial()
        void ReceiveNextMessage();

        // Read a partial message from the socket -> OnBytesReceived
        void ReadPartial(void* apDestination, size_t aTargetSize);
        
        // Check the APDU for completion
        //  - If the message is incompleted, another async read is requested via -> ReadPartial()
        //  - If the message is complete, it's proceesed via -> ProcessMessage() 
        //    and the next message is expected afterwards -> ReceiveNextMessage()
        void OnBytesReceived(const boost::system::error_code& arError, size_t aBytesReceived);
        
        // Decode the message and respond confirmations
        bool ProcessMessage();

        // Print the message on screen
        void PrintMessage(IEC104::Apdu& arMessage, bool aIsSend);

        // Verify that a message is expected and update sequence counters
        bool HandleSequences(const IEC104::Apdu& arReceived);
        
        // Acknowlegde received messages
        bool ConfirmReceive();

        // Close the connection with an error message
        void CloseError(const std::string& arErrorMsg);


    private:
        boost::asio::ip::tcp::socket mSocket;
        std::function<void(Connection&)> ClosedHandler;
        std::array<uint8_t, 256> mReadBuffer, mWriteBuffer;
        size_t mCurrentSize;
        int mNextReceiveId, mNextSendId, mLastAcknoledgedId;
        Apdu mReceived;
    };
}

#endif