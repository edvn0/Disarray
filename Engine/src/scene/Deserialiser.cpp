#include "DisarrayPCH.hpp"

#include "core/Collections.hpp"
#include "scene/Deserialiser.hpp"
#include "scene/Scene.hpp"

namespace Disarray {

void set_name_for_scene(Scene& scene, std::string_view name) { scene.set_name(name); }

} // namespace Disarray
