#include "DisarrayPCH.hpp"

#include "ui/UI.hpp"

namespace Disarray::UI {

	void drag_drop(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			const auto* item_path = path.c_str();
			ImGui::SetDragDropPayload("Disarray::DragDropItem", item_path, (std::strlen(item_path) + 1) * sizeof(char));
			ImGui::EndDragDropSource();
		}
	}

	std::optional<std::filesystem::path> accept_drag_drop(const std::string& payload_id, const std::string& allowed_extension)
	{
		std::optional<std::filesystem::path> fp {};
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id.c_str())) {
				const auto* path = static_cast<const wchar_t*>(payload->Data);
				fp = path;
			}
			ImGui::EndDragDropTarget();
		}

		if (!fp)
			return {};

		if (allowed_extension == "*")
			return fp;

		if (fp.value().extension() != allowed_extension)
			return {};

		return fp;
	}

} // namespace Disarray::UI
