#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <array>
#include <cstdint>
#include <functional>

#include <boost/asio/ip/tcp.hpp>

#include "core/watchdog.hpp"
#include "core/signal.hpp"

#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/connectionconfig.hpp"

namespace IEC104
{
    class BaseInfoObject;

    class Link
    {
    public:
        // Create an connection with an connected socket
        explicit Link(boost::asio::io_context& arContext, boost::asio::ip::tcp::socket&& arSocket, const ConnectionConfig& arConfig,
                            const std::function<void(Link&)>& arClosedHandler);
        ~Link();

        // Start message processing 
        void Start();

        bool ConfirmReceivedAsdus();

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
        void RespondTo(const Apdu& arReceived);
        void DeployMessage(const Apdu& arReceived);

        // Verify that a message is expected and update sequence counters
        bool HandleSequences(const IEC104::Apdu& arReceived);
        bool IsAsduConfirmThresholdReached() const noexcept;
        
        void ConfirmService(const Apdu& arReceived);

        // Close the connection with an error message
        void CloseError(const std::string& arErrorMsg);


    private:
        boost::asio::io_context& mrContext;
        boost::asio::ip::tcp::socket mSocket;
        std::function<void(Link&)> ClosedHandler;
        std::array<uint8_t, 256> mReadBuffer, mWriteBuffer;
        size_t mCurrentSize;
        int mNextReceiveId, mNextSendId;
        int mLastConfirmedByLocal, mLastConfirmedByRemote;
        Apdu mReceived;
        ConnectionConfig mConfig;
        CORE::Watchdog mAsduConfirmationTrigger;

        CORE::SignalEveryone<void, Link&, const Apdu&> SignalReceivedApdu;
    };
}

#endif