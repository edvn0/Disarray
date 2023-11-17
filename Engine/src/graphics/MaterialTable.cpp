#include "DisarrayPCH.hpp"

#include "graphics/MaterialTable.hpp"

namespace Disarray {

MaterialTable::MaterialTable(const Ref<MaterialTable>& other)
	: materials(other->materials)
	, material_count(other->material_count)
{
}

MaterialTable::MaterialTable(std::uint32_t material_count)
	: material_count(material_count)
{
	materials.reserve(material_count);
}

} // namespace Disarray