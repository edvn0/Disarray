#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

#include <cstring>
#include <string>

#include "core/Log.hpp"

namespace Disarray::UI {

void drag_drop(const std::filesystem::path& path)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		const auto item_path = path.string();
		const auto* item_c_str = item_path.c_str();
		ImGui::SetDragDropPayload("Disarray::DragDropItem", item_c_str, item_path.size());
		ImGui::EndDragDropSource();
	}
}

std::optional<std::filesystem::path> accept_drag_drop(const std::string& payload_id, const ExtensionSet& allowed_extensions)
{
	std::optional<std::filesystem::path> fp {};
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id.c_str())) {
			const auto* path = static_cast<const char*>(payload->Data);
			fp = path;
		}
		ImGui::EndDragDropTarget();
	}

	if (!fp)
		return {};

	if (allowed_extensions.contains("*"))
		return fp;
	const auto as_string = fp->extension().string();

	if (!allowed_extensions.contains(as_string.c_str()))
		return {};

	return fp;
}

} // namespace Disarray::UI
