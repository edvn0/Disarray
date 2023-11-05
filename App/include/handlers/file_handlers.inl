template <typename Child> class FileHandlerBase {
public:
	explicit FileHandlerBase(const Device& dev, Scene* scene, std::filesystem::path path)
		: device(dev)
		, base_scene(scene)
		, file_path(std::move(path))
	{
		if (valid_file()) {
			handle();
		}
	}

protected:
	auto child() -> auto& { return static_cast<Child&>(*this); }
	auto get_scene() -> auto* { return base_scene; }
	[[nodiscard]] auto get_device() const -> const auto& { return device; }
	[[nodiscard]] auto get_path() const -> const auto& { return file_path; }

private:
	void handle() { return child().handle_impl(); }
	auto valid_file() -> bool { return child().valid_file_impl(); }
	const Device& device;
	Scene* base_scene;
	std::filesystem::path file_path {};
	Collections::StringViewSet extensions {};
	bool valid { false };
};

class PNGHandler : public FileHandlerBase<PNGHandler> {
public:
	using FileHandlerBase<PNGHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto* scene = get_scene();
		const auto& path = get_path();
		auto entity = scene->create(path.filename().string());
		auto texture = Texture::construct(get_device(),
			{
				.path = path,
				.debug_name = path.filename().string(),
			});

		entity.add_component<Components::Texture>(texture);
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = { ".png"sv, ".jpg"sv, ".png"sv, ".jpeg"sv, ".bmp"sv };
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<PNGHandler>;
};

class MeshHandler : public FileHandlerBase<MeshHandler> {
public:
	using FileHandlerBase<MeshHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto* scene = get_scene();
		const auto& path = get_path();
		auto entity = scene->create(path.filename().string());
		MeshProperties properties {
			.path = path,
		};
		auto mesh = Mesh::construct(get_device(), properties);

		entity.add_component<Components::Mesh>(mesh);
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = {
			".mesh"sv,
			".obj"sv,
			".fbx"sv,
		};
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<MeshHandler>;
};

class SceneHandler : public FileHandlerBase<SceneHandler> {
public:
	using FileHandlerBase<SceneHandler>::FileHandlerBase;

private:
	void handle_impl()
	{
		auto& current_scene = *get_scene();
		current_scene.clear();
		Scene::deserialise_into(current_scene, get_device(), get_path());
	}

	auto valid_file_impl() -> bool
	{
		using namespace std::string_view_literals;
		const auto& path = get_path();
		static std::unordered_set valid_extensions = { ".scene"sv, ".json"sv };
		return valid_extensions.contains(path.extension().string());
	}

	friend class FileHandlerBase<SceneHandler>;
};

namespace {
template <class... T> struct HandlerGroup { };
using FileHandlers = HandlerGroup<PNGHandler, SceneHandler, MeshHandler>;
} // namespace
