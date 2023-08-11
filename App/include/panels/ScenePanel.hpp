#pragma once

#include "core/Panel.hpp"
#include "scene/Components.hpp"
#include "ui/UI.hpp"

#include <Disarray.hpp>

namespace Disarray::Client {

	class ScenePanel : public Disarray::Panel {
	public:
		ScenePanel(Disarray::Device& dev, Disarray::Window&, Disarray::Swapchain&, Disarray::Scene& s)
			: device(dev)
			, scene(s) {

			};

		void interface() override;
		void for_all_components(Entity& entity);

	private:
		bool shader_drop_button(const std::string& button_name, ShaderType type, Ref<Shader>& shader);

		Device& device;
		Scene& scene;
	};

} // namespace Disarray::Client
