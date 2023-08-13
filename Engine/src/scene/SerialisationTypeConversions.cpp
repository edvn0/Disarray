#include "scene/SerialisationTypeConversions.hpp"

#include "graphics/ImageProperties.hpp"

namespace Disarray {

void to_json(json& j, const Extent& p)
{
	j = json {
		{ "width", p.width },
		{ "height", p.height },
	};
}
void from_json(const json& j, Extent& p)
{
	j.at("width").get_to(p.width);
	j.at("height").get_to(p.height);
}

void to_json(json& j, const FloatExtent& p)
{
	j = json {
		{ "width", p.width },
		{ "height", p.height },
	};
}
void from_json(const json& j, FloatExtent& p)
{
	j.at("width").get_to(p.width);
	j.at("height").get_to(p.height);
}

} // namespace Disarray
