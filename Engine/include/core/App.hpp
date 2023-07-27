#pragma once

#include "core/Layer.hpp"
#include "core/Types.hpp"

#include <vector>

namespace Disarray {

	class Window;

	class App {
	public:
		~App();
		App();
		void run();

		template <typename T, typename... Args>
		void add_layer(Args&&... args)
			requires std::is_base_of_v<Layer, T>
		{
			layers.emplace_back(make_ref<T>(std::forward(args)...));
		}

	private:
		Scope<Window> window { nullptr };
		std::vector<Ref<Layer>> layers {};
	};

} // namespace Disarray
