#include <boost/test/unit_test.hpp>

#include "core/bytestream.hpp"

BOOST_AUTO_TEST_CASE(read_bytestream)
{
	ByteStream data{ 0x01, 0x02, 0x03, 0x04 };
	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x01);
	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x02);
	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x03);
	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x04);
	BOOST_REQUIRE_THROW(data.ReadByte(), std::out_of_range);

	data.WriteByte(0x05);
	data.WriteByte(0x06);

	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x05);
	BOOST_REQUIRE_EQUAL(data.ReadByte(), 0x06);
	BOOST_REQUIRE_THROW(data.ReadByte(), std::out_of_range);

	std::vector<int> vec(1024, -25);
	auto size = sizeof(int) * 1024;
	data.WriteData(vec.data(), size);
	BOOST_REQUIRE_EQUAL(data.RemainingBytes(), size);
	data.Flush();
	BOOST_REQUIRE_EQUAL(data.RemainingBytes(), size);

	auto ptr = reinterpret_cast<const int*> (data.ReadData(size));
	auto vec2 = std::vector(ptr, ptr + 1024);
	BOOST_REQUIRE_EQUAL(vec == vec2, true);
	BOOST_REQUIRE_EQUAL(data.RemainingBytes(), 0);

	BOOST_REQUIRE_THROW(data.ReadByte(), std::out_of_range);
	BOOST_REQUIRE_THROW(data.ReadData(1), std::out_of_range);
	data.Flush();
	BOOST_REQUIRE_THROW(data.ReadByte(), std::out_of_range);
	BOOST_REQUIRE_THROW(data.ReadData(1), std::out_of_range);
}
BOOST_AUTO_TEST_CASE(contruct_bytestream)
{
	static constexpr size_t DEFAULT_CAPACITY = 1024; // hint: 1024 is the default -> use here, but change if the Bytestream changes
	ByteStream empty1; 
	BOOST_REQUIRE_EQUAL(empty1.RemainingBytes(), 0);
	BOOST_REQUIRE_EQUAL(empty1.WritableBytes(), DEFAULT_CAPACITY);
	BOOST_REQUIRE_EQUAL(empty1.Capacity(), DEFAULT_CAPACITY);
	BOOST_REQUIRE_EQUAL(empty1.WriteBegin() + DEFAULT_CAPACITY, empty1.WriteEnd());

	ByteStream empty2(32000);
	BOOST_REQUIRE_EQUAL(empty2.RemainingBytes(), 0);
	BOOST_REQUIRE_EQUAL(empty2.WritableBytes(), 32000);
	BOOST_REQUIRE_EQUAL(empty2.Capacity(), 32000);
	BOOST_REQUIRE_EQUAL(empty2.WriteBegin() + 32000, empty2.WriteEnd());

	ByteStream data1{0x01, 0x02, 0x03};
	BOOST_REQUIRE_EQUAL(data1.RemainingBytes(), 3);
	BOOST_REQUIRE_EQUAL(data1.WritableBytes(), 0);
	BOOST_REQUIRE_EQUAL(data1.Capacity(), 3);
	BOOST_REQUIRE_EQUAL(data1.WriteBegin(), data1.WriteEnd());

	uint8_t arr[] = { 0x04, 0x05, 0x06, 0x07 };
	ByteStream data2(arr, arr + 4);
	BOOST_REQUIRE_EQUAL(data2.RemainingBytes(), 4);
	BOOST_REQUIRE_EQUAL(data2.WritableBytes(), 0);
	BOOST_REQUIRE_EQUAL(data2.Capacity(), 4);
	BOOST_REQUIRE_EQUAL(data2.WriteBegin(), data2.WriteEnd());
}


BOOST_AUTO_TEST_CASE(peek_bytestream)
{
	ByteStream empty;
	BOOST_REQUIRE_THROW(empty.PeekAt(0), std::out_of_range);
	BOOST_REQUIRE_THROW(empty.PeekAt(1), std::out_of_range);
	BOOST_REQUIRE_THROW(empty.PeekAt(-1), std::out_of_range);

	ByteStream data1{ 0x01, 0x02 };
	BOOST_REQUIRE_EQUAL(data1.PeekAt(0), 0x01);
	BOOST_REQUIRE_EQUAL(data1.PeekAt(1), 0x02);
	BOOST_REQUIRE_THROW(data1.PeekAt(2), std::out_of_range);
	BOOST_REQUIRE_THROW(data1.PeekAt(-1), std::out_of_range);
}
