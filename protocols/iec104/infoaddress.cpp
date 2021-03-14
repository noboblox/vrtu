#include "protocols/iec104/infoaddress.hpp"

#include <stdexcept>
#include "core/bytestream.hpp"

namespace IEC104
{
    InfoAddress::InfoAddress(const uint8_t* apSource, int aAddressSize)
        : mValue(), mSize(aAddressSize)
    {
        if (!apSource)
            throw std::invalid_argument("nullptr source");

        switch (mSize)
        {
        case 1:
            mValue = apSource[0];
            break;
        case 2:
            mValue = apSource[0];
            mValue |= (apSource[1] << 8);
            break;
        case 3:
            mValue = apSource[0];
            mValue |= (apSource[1] << 8);
            mValue |= (apSource[2] << 16);
            break;
        default:
            throw std::invalid_argument("Address size not in range [1,3]");
        }
    }

    void InfoAddress::WriteTo(ByteStream& arOutput) const
    {
        switch (mSize)
        {
        case 1:
            arOutput.WriteByte(mValue & 0xFF);
            break;
        case 2:
            arOutput.WriteByte( mValue & 0x00FF);
            arOutput.WriteByte((mValue & 0xFF00) >> 8);
            break;
        case 3:
            arOutput.WriteByte( mValue & 0x0000FF);
            arOutput.WriteByte((mValue & 0x00FF00) >> 8);
            arOutput.WriteByte((mValue & 0xFF0000) >> 16);
            break;

        case 0:
        default:
            break;
        }
    }
}
