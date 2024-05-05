#ifndef IEC104_SEQUENCE_HPP_
#define IEC104_SEQUENCE_HPP_

#include "core/bytestream.hpp"

namespace IEC104
{
	class Sequence
	{
	public:
		static constexpr int MAX_SEQ = 0x7FFF;

		explicit Sequence(ByteStream from) {
			int low = from.ReadByte();
			low >>= 1;

			int high = from.ReadByte();
			high <<= 7;

			mValue = low + high;
		}

		explicit Sequence() : mValue(0) {}

		Sequence(const Sequence&) = default;
		Sequence& operator=(const Sequence&) = default;
		Sequence(Sequence&&) = default;
		Sequence& operator=(Sequence&&) = default;

		inline int Value() const noexcept { 
			return mValue; 
		}
		
		inline uint8_t EncodedHighByte() const noexcept { 
			return static_cast<uint8_t>(mValue >> 7); 
		}
		
		inline uint8_t EncodedLowByte() const noexcept { 
			return static_cast<uint8_t>(((mValue & (0x7F)) << 1));
		}

		inline Sequence& operator++() noexcept {
			mValue >= MAX_SEQ ?	mValue = 0 : ++mValue;
			return *this;
		}

		inline Sequence& operator--() noexcept {
			mValue > 0 ? --mValue : mValue = 0;
			return *this;
		}

		inline Sequence operator++(int) noexcept {
			auto copy = *this;
			++*this;
			return copy;
		}

		inline Sequence operator--(int) noexcept {
			auto copy = *this;
			--*this;
			return copy;
		}

		inline bool operator==(Sequence other) {
			return mValue == other.mValue;
		}

		inline bool operator!=(Sequence other) {
			return !operator==(other);
		}

		inline bool operator<(Sequence other) {
			return Distance(other) > 0;
		}

		inline bool operator>=(Sequence other) {
			return !operator<(other);
		}

		inline bool operator>(Sequence other) {
			return other.operator<(other);
		}

		inline bool operator<=(Sequence other) {
			return !operator>(other);
		}

		// Determines how many hops in positive or negative direction
		// are needed to reach "to" from "this"
		// 
		// Because sequences can overflow, both cases are calculated
		// The smaller absolute value, meaning: the shorter path from "this" to "to" is then returned
		int Distance(Sequence to) const noexcept {
			int seq_from = mValue;
			int seq_to = to.mValue;

			int distance_normal = seq_to - seq_from;
			int distance_overflow;

			if (seq_from < seq_to)
				distance_overflow = seq_to - (seq_from + MAX_SEQ);
			else
				distance_overflow = (seq_to + MAX_SEQ) - seq_from;

			if (std::abs(distance_normal) < std::abs(distance_overflow))
				return distance_normal;
			else
				return distance_overflow;
		}

	private:
		int mValue;
	};
}


#endif
