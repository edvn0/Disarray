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

		void interface() override;

	private:
		Scene& scene;
	};

} // namespace Disarray::Client
