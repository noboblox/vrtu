#include <boost/test/unit_test.hpp>

#include "protocols/iec104/sequence.hpp"

using namespace IEC104;

BOOST_AUTO_TEST_CASE(read_sequence_from_stream)
{
	// hint: low byte is left!

	{
		ByteStream data{ 0xFF, 0xFF };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x7FFF);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0xFE);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0xFF);
		BOOST_REQUIRE_EQUAL(Sequence(0xFF, 0xFF).Value(), 0x7FFF);
		BOOST_REQUIRE_EQUAL(Sequence(0xFF, 0xFF).EncodedLowByte(), 0xFE);
		BOOST_REQUIRE_EQUAL(Sequence(0xFF, 0xFF).EncodedHighByte(), 0xFF);
	}
	{
		ByteStream data{ 0x00, 0x00 };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x0000);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0x00).Value(), 0x0000);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0x00).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0x00).EncodedHighByte(), 0x00);
	}
	{
		ByteStream data{ 0x01, 0x00 };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x0000);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x01, 0x00).Value(), 0x0000);
		BOOST_REQUIRE_EQUAL(Sequence(0x01, 0x00).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x01, 0x00).EncodedHighByte(), 0x00);
	}
	{
		ByteStream data{ 0x03, 0x00 };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x0001);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x02);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x03, 0x00).Value(), 0x0001);
		BOOST_REQUIRE_EQUAL(Sequence(0x03, 0x00).EncodedLowByte(), 0x02);
		BOOST_REQUIRE_EQUAL(Sequence(0x03, 0x00).EncodedHighByte(), 0x00);
	}
	{
		ByteStream data{ 0x02, 0x00 };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x0001);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x02);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x02, 0x00).Value(), 0x0001);
		BOOST_REQUIRE_EQUAL(Sequence(0x02, 0x00).EncodedLowByte(), 0x02);
		BOOST_REQUIRE_EQUAL(Sequence(0x02, 0x00).EncodedHighByte(), 0x00);
	}
	{
		ByteStream data{ 0x05, 0x00 };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x0002);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x04);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x05, 0x00).Value(), 0x0002);
		BOOST_REQUIRE_EQUAL(Sequence(0x05, 0x00).EncodedLowByte(), 0x04);
		BOOST_REQUIRE_EQUAL(Sequence(0x05, 0x00).EncodedHighByte(), 0x00);
	}
	{
		ByteStream data{ 0x00, 0xFF };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x7F80);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0xFF);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0xFF).Value(), 0x7F80);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0xFF).EncodedLowByte(), 0x00);
		BOOST_REQUIRE_EQUAL(Sequence(0x00, 0xFF).EncodedHighByte(), 0xFF);
	}
	{
		ByteStream data{ 0xF0, 0x0F };
		BOOST_REQUIRE_EQUAL(Sequence(data).Value(), 0x07F8);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedLowByte(), 0xF0);
		BOOST_REQUIRE_EQUAL(Sequence(data).EncodedHighByte(), 0x0F);
		BOOST_REQUIRE_EQUAL(Sequence(0xF0, 0x0F).Value(), 0x07F8);
		BOOST_REQUIRE_EQUAL(Sequence(0xF0, 0x0F).EncodedLowByte(), 0xF0);
		BOOST_REQUIRE_EQUAL(Sequence(0xF0, 0x0F).EncodedHighByte(), 0x0F);
	}

}


BOOST_AUTO_TEST_CASE(sequence_construction_in_range_enforced)
{
	BOOST_REQUIRE_EQUAL(Sequence().Value(), 0);
	BOOST_REQUIRE_EQUAL(Sequence(0).Value(), 0);
	BOOST_REQUIRE_EQUAL(Sequence(25000).Value(), 25000);
	BOOST_REQUIRE_EQUAL(Sequence(32767).Value(), 32767);
	BOOST_REQUIRE_THROW(Sequence(32768).Value(), std::invalid_argument);
	BOOST_REQUIRE_THROW(Sequence(-1).Value(), std::invalid_argument);
	BOOST_REQUIRE_THROW(Sequence(-99999999).Value(), std::invalid_argument);
	BOOST_REQUIRE_THROW(Sequence(99999999).Value(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(sequence_check_copy_move)
{
	{
		auto l = Sequence(25);
		auto r = l;

		BOOST_REQUIRE_EQUAL(l.Value(), 25);
		BOOST_REQUIRE_EQUAL(r.Value(), 25);
		BOOST_REQUIRE_EQUAL(l, r);

		l = l;
		BOOST_REQUIRE_EQUAL(l.Value(), 25);

		l = std::move(l);
		BOOST_REQUIRE_EQUAL(l.Value(), 25);

		auto copyAssing = Sequence(26);
		r = copyAssing;
		BOOST_REQUIRE_EQUAL(r.Value(), 26);
		
		r = Sequence(27); // move assign
		BOOST_REQUIRE_EQUAL(r.Value(), 27);
	}
	{
		auto l = Sequence(25);
		auto r = std::move(l);
		BOOST_REQUIRE_EQUAL(r.Value(), 25);
	}
}

BOOST_AUTO_TEST_CASE(sequence_increment_decrement)
{
	{
		auto seq = Sequence(0);
		BOOST_REQUIRE_EQUAL((--seq).Value(), 0x7FFF);
	}
	{
		auto seq = Sequence(1);
		BOOST_REQUIRE_EQUAL((--seq).Value(), 0);
	}
	{
		auto seq = Sequence(0x7FFF);
		BOOST_REQUIRE_EQUAL((++seq).Value(), 0);
	}
	{
		auto seq = Sequence(0x7FFE);
		BOOST_REQUIRE_EQUAL((++seq).Value(), 0x7FFF);
	}
	{
		auto seq = Sequence(0);
		BOOST_REQUIRE_EQUAL((seq--).Value(), 0);
		BOOST_REQUIRE_EQUAL(seq.Value(), 0x7FFF);
	}
	{
		auto seq = Sequence(1);
		BOOST_REQUIRE_EQUAL((seq--).Value(), 1);
		BOOST_REQUIRE_EQUAL(seq.Value(), 0);
	}
	{
		auto seq = Sequence(0x7FFF);
		BOOST_REQUIRE_EQUAL((seq++).Value(), 0x7FFF);
		BOOST_REQUIRE_EQUAL(seq.Value(), 0);
	}
	{
		auto seq = Sequence(0x7FFE);
		BOOST_REQUIRE_EQUAL((seq++).Value(), 0x7FFE);
		BOOST_REQUIRE_EQUAL(seq.Value(), 0x7FFF);
	}
}
BOOST_AUTO_TEST_CASE(sequence_distance_and_comparison)
{
	BOOST_REQUIRE_EQUAL(Sequence(25000).Distance(Sequence(25000)), 0);
	BOOST_REQUIRE_EQUAL(Sequence(25000).Distance(Sequence(26000)), 1000);
	BOOST_REQUIRE_EQUAL(Sequence(25000).Distance(Sequence(24000)), -1000);

	BOOST_REQUIRE_EQUAL(Sequence(32767).Distance(Sequence(0)), 1);
	BOOST_REQUIRE_EQUAL(Sequence(0).Distance(Sequence(32767)), -1);

	BOOST_REQUIRE_EQUAL(Sequence(32767).Distance(Sequence(1000)), 1001);
	BOOST_REQUIRE_EQUAL(Sequence(0).Distance(Sequence(31767)), -1001);

	BOOST_REQUIRE_EQUAL(Sequence(1000) == Sequence(1000), true);
	BOOST_REQUIRE_EQUAL(Sequence(1000) != Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(1000) <  Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(1000) >  Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(1000) >= Sequence(1000), true);
	BOOST_REQUIRE_EQUAL(Sequence(1000) <= Sequence(1000), true);

	BOOST_REQUIRE_EQUAL(Sequence(1000) == Sequence(999), false);
	BOOST_REQUIRE_EQUAL(Sequence(1000) != Sequence(999), true);
	BOOST_REQUIRE_EQUAL(Sequence(1000) <  Sequence(999), false);
	BOOST_REQUIRE_EQUAL(Sequence(1000) >  Sequence(999), true);
	BOOST_REQUIRE_EQUAL(Sequence(1000) >= Sequence(999), true);
	BOOST_REQUIRE_EQUAL(Sequence(1000) <= Sequence(999), false);

	// hint: 32000 < 1000, because 1000 is considered overflown (see: Sequence::Distance)
	BOOST_REQUIRE_EQUAL(Sequence(32000) == Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(32000) != Sequence(1000), true);
	BOOST_REQUIRE_EQUAL(Sequence(32000) <  Sequence(1000), true);
	BOOST_REQUIRE_EQUAL(Sequence(32000) >  Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(32000) >= Sequence(1000), false);
	BOOST_REQUIRE_EQUAL(Sequence(32000) <= Sequence(1000), true);
}

