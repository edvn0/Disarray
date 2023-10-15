#pragma once

#include <glm/glm.hpp>

#include "core/events/Event.hpp"
#include "core/events/MouseEvent.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

enum class CameraType : std::uint8_t { Perspective, Orthographic };

class Camera {
public:
	Camera() = default;
	Camera(const Camera&) = default;
	Camera(const glm::mat4& projection, const glm::mat4& unreversed_projection);
	Camera(const float degree_fov, const float width, const float height, const float near_plane, const float far_plane);

	virtual ~Camera() = default;

	virtual void focus(const glm::vec3&);
	virtual void on_update(float);

	[[nodiscard]] virtual auto get_near_clip() const -> float = 0;
	[[nodiscard]] virtual auto get_far_clip() const -> float = 0;

	[[nodiscard]] virtual auto get_position() const -> glm::vec3 = 0;
	[[nodiscard]] virtual auto get_direction() const -> glm::vec3 = 0;

	[[nodiscard]] virtual auto get_projection_matrix() const -> const glm::mat4& { return projection_matrix; }
	[[nodiscard]] virtual auto get_view_matrix() const -> const glm::mat4& { return view_matrix; }
	[[nodiscard]] virtual auto get_unreversed_projection_matrix() const -> const glm::mat4& { return unreversed_projection_matrix; }
	[[nodiscard]] auto get_view_projection() const -> glm::mat4 { return get_projection_matrix() * get_view_matrix(); }

	void set_projection_matrix(const glm::mat4& projection, const glm::mat4& unreversed_projection)
	{
		projection_matrix = projection;
		unreversed_projection_matrix = unreversed_projection;
	}

	void set_perspective_projection_matrix(
		const float radians_fov, const float width, const float height, const float near_plane, const float far_plane);

	void set_ortho_projection_matrix(const float width, const float height, const float near_plane, const float far_plane);

	[[nodiscard]] auto get_exposure() const -> float { return exposure; }
	auto get_exposure() -> float& { return exposure; }

protected:
	float exposure = 0.8f;

private:
	glm::mat4 projection_matrix { 1.0F };
	glm::mat4 unreversed_projection_matrix { 1.0F };
	glm::mat4 view_matrix { 1.0F };
};

enum class CameraMode : std::uint8_t { None, Flycam, Arcball };

class EditorCamera : public Camera {
public:
	EditorCamera(const float degree_fov, const float width, const float height, const float near_plane, const float far_plane,
		EditorCamera* previous_camera = nullptr);
	void init(EditorCamera* previous_camera = nullptr);

	void focus(const glm::vec3& focus_point) final;
	void on_update(float time_step) final;

	[[nodiscard]] auto is_active() const -> bool { return this->active; }
	void set_active(bool in) { this->active = in; }

	[[nodiscard]] auto get_current_mode() const -> CameraMode { return camera_mode; }

	[[nodiscard]] auto get_distance() const -> float { return distance; }
	void set_distance(float in) { distance = in; }

	[[nodiscard]] auto get_focal_point() const -> const glm::vec3& { return focal_point; }

	void set_viewport_size(std::uint32_t width, std::uint32_t height)
	{
		if (viewport_width == width && viewport_height == height) {
			return;
		}
		set_perspective_projection_matrix(vertical_fov, static_cast<float>(width), static_cast<float>(height), near_clip, far_clip);
		viewport_width = width;
		viewport_height = height;
	}

	void set_viewport_size(const Extent& extent)
	{
		const auto& [width, height] = extent;
		if (viewport_width == width && viewport_height == height) {
			return;
		}
		aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		set_perspective_projection_matrix(vertical_fov, static_cast<float>(width), static_cast<float>(height), near_clip, far_clip);
		viewport_width = width;
		viewport_height = height;
		update_camera_view();
	}

	void set_viewport_size(const FloatExtent& extent)
	{
		const auto& [width, height] = extent;
		if (viewport_width == width && viewport_height == height) {
			return;
		}
		aspect_ratio = width / height;
		set_perspective_projection_matrix(vertical_fov, width, height, near_clip, far_clip);
		viewport_width = static_cast<std::uint32_t>(width);
		viewport_height = static_cast<std::uint32_t>(height);
		update_camera_view();
	}

	void on_event(Event& event);
	auto on_mouse_scroll(MouseScrolledEvent& e) -> bool;

	template <class Ex>
		requires(std::is_same_v<Ex, Extent> || std::is_same_v<Ex, FloatExtent>)
	void set_viewport_size(const Ex& ex)
	{
		if constexpr (std::is_same_v<Ex, Extent>) {
			set_viewport_size(Extent { ex.width, ex.height });
		} else if constexpr (std::is_same_v<Ex, FloatExtent>) {
			set_viewport_size(FloatExtent { ex.width, ex.height });
		}
	}

	[[nodiscard]] auto get_view_matrix() const -> const glm::mat4& override { return view_matrix; }
	[[nodiscard]] auto get_unreversed_view_projection() const { return get_unreversed_projection_matrix() * view_matrix; }

	[[nodiscard]] auto get_up_direction() const -> glm::vec3;
	[[nodiscard]] auto get_right_direction() const -> glm::vec3;
	[[nodiscard]] auto get_forward_direction() const -> glm::vec3;

	[[nodiscard]] auto get_position() const -> glm::vec3 override { return position; }
	[[nodiscard]] auto get_direction() const -> glm::vec3 override { return direction; }

	[[nodiscard]] auto get_orientation() const -> glm::quat;

	[[nodiscard]] auto get_vertical_fov() const -> float { return vertical_fov; }
	[[nodiscard]] auto get_aspect_ratio() const -> float { return aspect_ratio; }
	[[nodiscard]] auto get_near_clip() const -> float override { return near_clip; }
	[[nodiscard]] auto get_far_clip() const -> float override { return far_clip; }
	[[nodiscard]] auto get_pitch() const -> float { return pitch; }
	[[nodiscard]] auto get_yaw() const -> float { return yaw; }
	[[nodiscard]] auto get_camera_speed() const -> float;

private:
	void update_camera_view();

	void mouse_pan(const glm::vec2& delta);
	void mouse_rotate(const glm::vec2& delta);
	void mouse_zoom(float delta);

	[[nodiscard]] auto calculate_position() const -> glm::vec3;

	[[nodiscard]] auto pan_speed() const -> std::pair<float, float>;
	static auto rotation_speed() -> float;
	[[nodiscard]] auto zoom_speed() const -> float;

	glm::mat4 view_matrix { 1.0F };
	glm::vec3 position = { 5, 5, -5 };
	glm::vec3 direction { 0.F };
	glm::vec3 focal_point { 0.0F };

	float vertical_fov { 0 };
	float aspect_ratio { 0 };
	float near_clip { 0 };
	float far_clip { 0 };

	bool active = false;
	glm::vec2 initial_mouse_position {};

	float distance;
	float normal_speed { 0.002f };

	float pitch = glm::radians(-30.0F);
	float yaw = 0;
	float pitch_delta {};
	float yaw_delta {};
	glm::vec3 position_delta {};
	glm::vec3 right_direction {};

	CameraMode camera_mode { CameraMode::Arcball };

	float min_focus_distance { 100.0F };

	std::uint32_t viewport_width { 1280 };
	std::uint32_t viewport_height { 720 };

	constexpr static float min_speed { 0.002f };
	constexpr static float max_speed { 0.02f };
};

} // namespace Disarray
