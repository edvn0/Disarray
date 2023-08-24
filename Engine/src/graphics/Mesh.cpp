#include "DisarrayPCH.hpp"

#include "graphics/Mesh.hpp"

#include "vulkan/Mesh.hpp"

namespace Disarray {

Ref<Mesh> Mesh::construct(const Disarray::Device& device, const Disarray::MeshProperties& props) { return make_ref<Vulkan::Mesh>(device, props); }

} // namespace Disarray
