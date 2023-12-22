#pragma once

#include "core/Log.hpp"
#include "core/exceptions/BaseException.hpp"

namespace Disarray {

class ResultException final : public BaseException {
public:
	explicit ResultException(std::string_view msg)
		: BaseException("ResultException", msg)
	{
	}
};

class NoSuitableDeviceException final : public BaseException {
public:
	explicit NoSuitableDeviceException(std::string_view msg)
		: BaseException("NoSuitableDeviceException", msg)
	{
	}
};

class SwapchainImageAcquisitionException final : public BaseException {
public:
	explicit SwapchainImageAcquisitionException(std::string_view msg)
		: BaseException("SwapchainImageAcquisitionException", msg)
	{
	}
};

class CouldNotPresentSwapchainException final : public BaseException {
public:
	explicit CouldNotPresentSwapchainException(std::string_view msg)
		: BaseException("CouldNotPresentSwapchainException", msg)
	{
	}
};

class CouldNotCreateValidationLayersException final : public BaseException {
public:
	explicit CouldNotCreateValidationLayersException(std::string_view msg)
		: BaseException("CouldNotCreateValidationLayersException", msg)
	{
	}
};

} // namespace Disarray
