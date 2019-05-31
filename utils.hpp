#ifndef UTILS_H
#define UTILS_H
#include "assert.h"
#include <array>

namespace utils {

	template<typename T, typename T1>
	inline void setBit(T &value, T1 bit) {
		assert((sizeof(T) * 8U) > bit);
		value |= static_cast<T>(static_cast<T>(1) << static_cast<T>(bit));
	};

	template<typename T, typename T1>
	inline void clearBit(T &value, T1 bit) {
		assert((sizeof(T) * 8U) > bit);
		value &= ~static_cast<T>(static_cast<T>(1) << static_cast<T>(bit));
	};

	template<typename T, typename T1>
	inline void toggleBit(T &value, T1 bit) {
		assert((sizeof(T) * 8U) > bit);
		value ^= static_cast<T>(static_cast<T>(1) << static_cast<T>(bit));
	};

	template<typename T, typename T1>
	inline bool checkBit(const T &value, T1 bit) {
		assert((sizeof(T) * 8U) > bit);
		return !((value & (static_cast<T>(1) << static_cast<T>(bit))) == static_cast<T>(0U));
	};
};

#endif
