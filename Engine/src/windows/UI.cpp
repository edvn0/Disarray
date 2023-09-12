#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

#include <string>

#include "core/Formatters.hpp"
#include "core/Log.hpp"
#include "imgui.h"

namespace Disarray::UI {

void drag_drop(const std::filesystem::path& path)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		const auto as_string = path.string();
		const auto size = as_string.size() * sizeof(char);
		ImGui::SetDragDropPayload("Disarray::DragDropItem", as_string.c_str(), size);
		ImGui::EndDragDropSource();
	}
}

auto accept_drag_drop(const std::string& payload_id, const ExtensionSet& allowed_extensions) -> std::optional<std::filesystem::path>
{
	std::optional<std::filesystem::path> fp {};
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id.c_str())) {
			const auto* base = static_cast<const char*>(payload->Data);
			std::string data { base, static_cast<std::size_t>(payload->DataSize) };
			fp = data;
		}
		ImGui::EndDragDropTarget();
	}

	if (!fp) {
		return {};
	}

	if (allowed_extensions.contains("*")) {
		return fp;
	}
	const auto as_string = fp->extension().string();

	if (!allowed_extensions.contains(as_string)) {
		return {};
	}

	return fp;
}

} // namespace Disarray::UI
