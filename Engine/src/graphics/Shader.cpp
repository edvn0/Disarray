#include "DisarrayPCH.hpp"

#include "graphics/Shader.hpp"

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <streambuf>

#include "vulkan/Shader.hpp"

namespace fmt {

auto fmt::formatter<Disarray::ShaderType>::format(const Disarray::ShaderType& format, format_context& ctx) -> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", magic_enum::enum_name(format)), ctx);
}

} // namespace fmt

namespace Disarray {

auto Shader::construct(const Disarray::Device& device, ShaderProperties properties) -> Ref<Disarray::Shader>
{
	return make_ref<Vulkan::Shader>(device, std::move(properties));
}

auto Shader::compile(const Disarray::Device& device, const std::filesystem::path& path) -> Ref<Disarray::Shader>
{
	return make_ref<Vulkan::Shader>(device, path);
}

} // namespace Disarray
