#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "asdu.hpp"
#include "bytestream.hpp"
#include "data.hpp"
#include "infoaddress.hpp"
#include "infoobjects.hpp"
#include "namedenum.hpp"

int main()
{
    static constexpr uint8_t DUMMY_MSG[] = {
        0x03,             // TK == 3, DoublePoint
        0x05,             // 5 IOs, no sequence
        0x03, 0x00,       // Reason: spontaneous, positive, no test, default origin
        0x0A, 0x05,       // Station address: 10-5
                          // IO 1
        0xCB, 0x37, 0x12, // IOA 203-55-18
        0x01,             // DoublePoint: value=off, quality=good
                          // IO 2
        0xCB, 0x39, 0x12, // IOA 203-57-18
        0xF3,             // DoublePoint: value=faulty, quality= blocked, invalid, not-topical, substituted 
                          // IO 3
        0xCB, 0x41, 0x12, // IOA 203-59-18
        0x00,             // DoublePoint: value=intermediate, quality=good
                          // IO 4
        0xCB, 0x43, 0x12, // IOA 203-61-18
        0x02,             // DoublePoint: value=on, quality=good
                          // IO 5
        0xCB, 0x45, 0x12, // IOA 203-63-18
        0x02              // DoublePoint: value=on, quality=good
    };

    static constexpr uint8_t DUMMY_MSG2[] = {
    0x0D,                   // TK == 13, measurement float
    0x01,                   // 1 IOs, no sequence
    0x03, 0x00,             // Reason: spontaneous, positive, no test, default origin
    0x0A, 0x05,             // Station address: 10-5
                            // IO 1
    0xCB, 0x37, 0x12,       // IOA 203-55-18
    0x56, 0x0E, 0x49, 0x40, // floating point value= ~3.1415f
    0x01,                   // quality=overflow
    };

    static constexpr uint8_t DUMMY_MSG3[] = {
    0x0B,                   // TK == 11, measurement scaled
    0x01,                   // 1 IOs, no sequence
    0x03, 0x00,             // Reason: spontaneous, positive, no test, default origin
    0x0A, 0x05,             // Station address: 10-5
                            // IO 1
    0xCB, 0x37, 0x12,       // IOA 203-55-18
    0xA2, 0xFE,             // signed 16bit int: -350
    0x01,                   // quality=overflow
    };
    
    ByteStream source, destination;
    source.WriteData(DUMMY_MSG, sizeof(DUMMY_MSG));
    IEC104::Asdu msg(IEC104::AsduConfig::Defaults);
    try
    {
        msg.ReadFrom(source);
        msg.WriteTo(destination);

        bool success = true;
        source.ResetIterator();

        if (source.RemainingBytes() != destination.RemainingBytes())
            success = false;

        while (source.RemainingBytes() != 0)
        {
            auto left = source.ReadByte();
            auto right = destination.ReadByte();

            if (left != right)
                success = false;
        }

        if (!success)
            return -1;

        msg.Serialize(std::cout);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }

    return 0;
}