#pragma once

#include "core/Panel.hpp"
#include "scene/Components.hpp"
#include "ui/UI.hpp"

#include <Disarray.hpp>

namespace Disarray::Client {

	class ScenePanel : public Disarray::Panel {
	public:
		ScenePanel(Disarray::Device&, Disarray::Window&, Disarray::Swapchain&, Disarray::Scene& s)
			: scene(s) {

			};

		void update(float ts, Renderer&) override { }

		void interface() override
		{
			UI::begin("Scene");
			if (ImGui::BeginTable("StatisticsTable", 2)) {
				scene.for_all_entities([&s = scene](const auto entity_id) mutable {
					Entity entity { s, entity_id };
					const auto& [tag, id] = entity.get_components<Tag, ID>();
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%s", tag.name.c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%llu", id.identifier);
				});
				ImGui::EndTable();
			}
			UI::end();
		}

	private:
		Scene& scene;
	};

} // namespace Disarray::Client
