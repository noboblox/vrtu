#include <boost/test/unit_test.hpp>

#include "protocols/iec104/infoobjects.hpp"

BOOST_AUTO_TEST_CASE(single_point_type_and_len)
{
	IEC104::DataSinglePoint elem;
	BOOST_REQUIRE_EQUAL(elem.TYPE_ID,  IEC104::Type::M_SP_NA_1);
	BOOST_REQUIRE_EQUAL(elem.DATA_SIZE, 1);
}

