#pragma once

#include <string_view>

#include "core/exceptions/BaseException.hpp"

namespace Disarray {

class UnreachableException : public BaseException {
public:
	explicit UnreachableException(std::string_view msg)
		: BaseException("UnreachableException", msg)
	{
	}
};

class WindowingAPIException : public BaseException {
public:
	explicit WindowingAPIException(std::string_view msg)
		: BaseException("WindowingAPIException", msg)
	{
	}
};

class CouldNotInitialiseWindowingAPI : public BaseException {
public:
	explicit CouldNotInitialiseWindowingAPI(std::string_view msg)
		: BaseException("CouldNotInitialiseWindowingAPI", msg)
	{
	}
};

class MissingValueException : public BaseException {
public:
	explicit MissingValueException(std::string_view msg)
		: BaseException("MissingValueException", msg)
	{
	}
};

class CouldNotFormatException : public BaseException {
public:
	explicit CouldNotFormatException(std::string_view msg)
		: BaseException("CouldNotFormatException", msg)
	{
	}
};

class CouldNotLoadModelException : public BaseException {
public:
	explicit CouldNotLoadModelException(std::string_view msg)
		: BaseException("CouldNotLoadModelException", msg)
	{
	}
};

class CouldNotCompileShaderException : public BaseException {
public:
	explicit CouldNotCompileShaderException(std::string_view msg)
		: BaseException("CouldNotCompileShaderException", msg)
	{
	}
};

class CouldNotOpenStreamException : public BaseException {
public:
	explicit CouldNotOpenStreamException(std::string_view msg)
		: BaseException("CouldNotOpenStreamException", msg)
	{
	}
};

} // namespace Disarray
