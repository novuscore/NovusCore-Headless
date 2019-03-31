#pragma once

#include <cstdint>
#include <cstddef>
#include "../NovusTypes.h"

namespace StringUtils
{
	// FNV-1a 32bit hashing algorithm.
	constexpr u32 fnv1a_32(char const* s, std::size_t count)
	{
		return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
	}

	inline std::vector<std::string> SplitString(std::string string)
	{
		std::istringstream iss(string);
		std::vector<std::string> results(std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>());
		return results;
	}
}

constexpr u32 operator"" _h(char const* s, std::size_t count)
{
	return StringUtils::fnv1a_32(s, count);
}