#include <boost/test/unit_test.hpp>

#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "protocols/iec104/link.hpp"
using namespace IEC104;

namespace asio = boost::asio;
namespace aync = boost::cobalt;

class TestEnvironment
{
public:
	TestEnvironment() {
		async::this_thread::set_executor(ctx.get_executor());

		fakeTime = VRTU::ClockWrapper::UtcNow();
		VRTU::ClockWrapper::Override([this]() { return MockUtcNow(); });
	}

	~TestEnvironment() {
		VRTU::ClockWrapper::Restore();
	}

	std::chrono::milliseconds MockUtcNow() const {
		return fakeTime;
	}

	void AdvanceTime(std::chrono::milliseconds msec) {
		fakeTime += msec;
	}

	void Handshake() {
		auto addr = asio::ip::address::from_string("127.0.0.1");
		uint16_t serverPort = 2404;

		client.async_connect(asio::ip::tcp::endpoint{ addr, serverPort }, [](boost::system::error_code ec) {
			if (ec.failed()) BOOST_FAIL("tcp handshake failed");
		});

		asio::ip::tcp::acceptor listener(ctx, {addr, serverPort});

		listener.async_accept(server, [](boost::system::error_code ec) {
			if (ec.failed()) BOOST_FAIL("tcp handshake failed");
		});

		ctx.poll();
	}

	asio::io_context ctx;
	asio::ip::tcp::socket client = asio::ip::tcp::socket(ctx);
	asio::ip::tcp::socket server = asio::ip::tcp::socket(ctx);
	std::chrono::milliseconds fakeTime;
};

static std::unique_ptr<TestEnvironment> InitTest() {
	auto env = std::make_unique<TestEnvironment>();
	env->Handshake();
	return env;
}

BOOST_AUTO_TEST_CASE(link_startdt_stopdt_testfr)
{
	// TODO
	//static const uint8_t STARTDT_ACT[] = { 0x68, 0x04, 0x07, 0x00, 0x00, 0x00 };
	//static const uint8_t STARTDT_CON[] = { 0x68, 0x04, 0x0B, 0x00, 0x00, 0x00 };
	//static const uint8_t STOPDT_ACT[]  = { 0x68, 0x04, 0x13, 0x00, 0x00, 0x00 };
	//static const uint8_t STOPDT_CON[]  = { 0x68, 0x04, 0x23, 0x00, 0x00, 0x00 };
	//static const uint8_t TESTFT_ACT[]  = { 0x68, 0x04, 0x43, 0x00, 0x00, 0x00 };
	//static const uint8_t TESTFT_CON[]  = { 0x68, 0x04, 0x83, 0x00, 0x00, 0x00 };
	//std::vector<uint8_t> recvBuffer(1024, 0);

	//auto env = InitTest();

	//Link l(std::move(env->server), Link::Mode::Slave);
	//auto linkRun = l.Run();
	//env->client.send(boost::asio::buffer(STARTDT_ACT, 6));
	//env->client.async_read_some(boost::asio::buffer(recvBuffer.data(), recvBuffer.size()), [&recvBuffer](boost::system::error_code ec, size_t read) {
	//	BOOST_REQUIRE_EQUAL(read, 6);
	//	BOOST_REQUIRE_EQUAL(std::memcmp(STARTDT_CON, recvBuffer.data(), 6), 0);
	//});
	//env->ctx.poll();
}
