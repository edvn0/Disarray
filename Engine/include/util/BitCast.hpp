#pragma once

namespace Disarray {

	template <class To> To bit_cast(auto in)
	{
#ifdef WIN32
		return reinterpret_cast<To>(in);
#else
		return std::bit_cast<To>(in);
#endif
	}

} // namespace Disarray
