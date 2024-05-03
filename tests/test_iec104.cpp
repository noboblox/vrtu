/**
 *  @file   test_iec104.cpp
 *  @author noboblox
 *  @date   Feb 22, 2022
 */

#include <boost/test/unit_test.hpp>

#include <cstdint>

#include "core/bytestream.hpp"
#include "protocols/iec104/104enums.hpp"
#include "protocols/iec104/asdu.hpp"

using VerifyFunc =  std::function<void(const IEC104::Asdu&)>;
static void VerifyAsdu(const IEC104::Asdu& arAsdu, int aType, bool aIsSequence, int aObjectCount, int aReason, int aAddress)
{
    BOOST_CHECK_EQUAL(aType, arAsdu.GetType());
    BOOST_CHECK_EQUAL(aIsSequence, arAsdu.IsSequence());
    BOOST_CHECK_EQUAL(aObjectCount, arAsdu.GetObjectCount());
    BOOST_CHECK_EQUAL(aReason, (int) arAsdu.GetReason().GetValue());
    BOOST_CHECK_EQUAL(aAddress, arAsdu.GetAddress());
}

enum Result
{
    SUCCESS,
    FAILURE
};

struct TestCase
{
    std::vector<uint8_t> mData;
    Result               mResult;
    VerifyFunc           mVerfifyFunc;
};

BOOST_AUTO_TEST_CASE(test_asdu_read_from_memory)
{
    static const TestCase TEST_CASES[] = {
        {std::vector<uint8_t>{0x01, 0x94, 0x03, 0x00, 0xAF, 0xAF, 0x12, 0x13,               // header
                              0x14,                                                         // 1st ioa
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 20 x IO
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                SUCCESS, [](const IEC104::Asdu& in) {VerifyAsdu(in, IEC104::M_SP_NA_1, true, 20, (int) IEC104::ReasonCode::SPONTANEOUS, 0xAFAF);}}
    };

    for (int i = 0; i < (sizeof(TEST_CASES) / sizeof(TestCase)); ++i)
    {
        IEC104::Asdu asdu;
        ByteStream input(TEST_CASES[i].mData.data(), TEST_CASES[i].mData.data() + TEST_CASES[i].mData.size());

        if (TEST_CASES[i].mResult == FAILURE)
            BOOST_CHECK_THROW(asdu.ReadFrom(input), std::exception);
        else
        {
            BOOST_CHECK_NO_THROW(asdu.ReadFrom(input));
            TEST_CASES[i].mVerfifyFunc(asdu);
        }
    }

}

