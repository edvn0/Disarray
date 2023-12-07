#pragma once

#include "core/exceptions/BaseException.hpp"

namespace Disarray {

class ResultException : public BaseException {
public:
	explicit ResultException(std::string_view msg);
};

class NoSuitableDeviceException : public BaseException {
public:
	explicit NoSuitableDeviceException(std::string_view msg)
		: BaseException("NoSuitableDeviceException", msg)
	{
	}
};

class SwapchainImageAcquisitionException : public BaseException {
public:
	explicit SwapchainImageAcquisitionException(std::string_view msg)
		: BaseException("SwapchainImageAcquisitionException", msg)
	{
	}
};

class CouldNotPresentSwapchainException : public BaseException {
public:
	explicit CouldNotPresentSwapchainException(std::string_view msg)
		: BaseException("CouldNotPresentSwapchainException", msg)
	{
	}
};

class CouldNotCreateValidationLayersException : public BaseException {
public:
	explicit CouldNotCreateValidationLayersException(std::string_view msg)
		: BaseException("CouldNotCreateValidationLayersException", msg)
	{
	}
};

} // namespace Disarray
