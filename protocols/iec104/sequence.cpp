#include "protocols/iec104/sequence.hpp"
#include <ostream>

namespace IEC104
{
	std::ostream& operator<<(std::ostream& os, const Sequence& obj) {
		return os << obj.Value();
	}
}
