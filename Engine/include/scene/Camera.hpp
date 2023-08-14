#pragma once

#include "graphics/ImageProperties.hpp"

#include <glm/glm.hpp>

namespace Disarray {

enum class CameraType { Perspective, Orthographic };

class Camera {
public:
	Camera() = default;
	Camera(const Camera&) = default;
	Camera(const glm::mat4& projection, const glm::mat4& unreversed_projection);
	Camera(const float degree_fov, const float width, const float height, const float near_plane, const float far_plane);

	virtual ~Camera() = default;

	virtual void focus(const glm::vec3&) {};
	virtual void on_update(float) {};

	virtual glm::vec3 get_position() const = 0;

	virtual const glm::mat4& get_projection_matrix() const { return projection_matrix; }
	virtual const glm::mat4& get_view_matrix() const { return view_matrix; }
	virtual const glm::mat4& get_unreversed_projection_matrix() const { return unreversed_projection_matrix; }
	glm::mat4 get_view_projection() const { return get_projection_matrix() * get_view_matrix(); }

	void set_projection_matrix(const glm::mat4& projection, const glm::mat4& unreversed_projection)
	{
		projection_matrix = projection;
		unreversed_projection_matrix = unreversed_projection;
	}

	void set_perspective_projection_matrix(
		const float radians_fov, const float width, const float height, const float near_plane, const float far_plane);

	void set_ortho_projection_matrix(const float width, const float height, const float near_plane, const float far_plane);

	float get_exposure() const { return exposure; }
	float& get_exposure() { return exposure; }

protected:
	float exposure = 0.8f;

private:
	glm::mat4 projection_matrix { 1.0f };
	glm::mat4 unreversed_projection_matrix { 1.0f };
	glm::mat4 view_matrix { 1.0f };
};

enum class CameraMode { None, Flycam, Arcball };

class EditorCamera : public Camera {
public:
	EditorCamera(const float degree_fov, const float width, const float height, const float near_plane, const float far_plane,
		EditorCamera* previous_camera = nullptr);
	void init(EditorCamera* previous_camera = nullptr);

	void focus(const glm::vec3& focus_point) final;
	void on_update(float ts) final;

	bool is_active() const { return this->active; }
	void set_active(bool in) { this->active = in; }

	CameraMode get_current_mode() const { return camera_mode; }

	float get_distance() const { return distance; }
	void set_distance(float in) { distance = in; }

	const glm::vec3& get_focal_point() const { return focal_point; }

	void set_viewport_size(std::uint32_t width, std::uint32_t height)
	{
		if (viewport_width == width && viewport_height == height)
			return;
		set_perspective_projection_matrix(vertical_fov, static_cast<float>(width), static_cast<float>(height), near_clip, far_clip);
		viewport_width = width;
		viewport_height = height;
	}

	void set_viewport_size(const Extent& extent)
	{
		const auto& [width, height] = extent;
		if (viewport_width == width && viewport_height == height)
			return;
		aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		set_perspective_projection_matrix(vertical_fov, static_cast<float>(width), static_cast<float>(height), near_clip, far_clip);
		viewport_width = width;
		viewport_height = height;
		// update_camera_view();
	}

	void set_viewport_size(const FloatExtent& extent)
	{
		const auto& [width, height] = extent;
		if (viewport_width == width && viewport_height == height)
			return;
		aspect_ratio = width / height;
		set_perspective_projection_matrix(vertical_fov, width, height, near_clip, far_clip);
		viewport_width = static_cast<std::uint32_t>(width);
		viewport_height = static_cast<std::uint32_t>(height);
		// update_camera_view();
	}

	template <class Ex>
		requires(std::is_same_v<Ex, Extent> || std::is_same_v<Ex, FloatExtent>)
	void set_viewport_size(const Ex& ex)
	{
		if constexpr (std::is_same_v<Ex, Extent>)
			set_viewport_size(Extent { ex.width, ex.height });
		else if constexpr (std::is_same_v<Ex, FloatExtent>)
			set_viewport_size(FloatExtent { ex.width, ex.height });
	}

	const glm::mat4& get_view_matrix() const override { return view_matrix; }
	glm::mat4 get_unreversed_view_projection() const { return get_unreversed_projection_matrix() * view_matrix; }

	glm::vec3 get_up_direction() const;
	glm::vec3 get_right_direction() const;
	glm::vec3 get_forward_direction() const;

	glm::vec3 get_position() const override { return position; }

	glm::quat get_orientation() const;

	[[nodiscard]] float get_vertical_fov() const { return vertical_fov; }
	[[nodiscard]] float get_aspect_ratio() const { return aspect_ratio; }
	[[nodiscard]] float get_near_clip() const { return near_clip; }
	[[nodiscard]] float get_far_clip() const { return far_clip; }
	[[nodiscard]] float get_pitch() const { return pitch; }
	[[nodiscard]] float get_yaw() const { return yaw; }
	[[nodiscard]] float get_camera_speed() const;

private:
	void update_camera_view();

	void mouse_pan(const glm::vec2& delta);
	void mouse_rotate(const glm::vec2& delta);
	void mouse_zoom(float delta);

	glm::vec3 calculate_position() const;

	std::pair<float, float> pan_speed() const;
	float rotation_speed() const;
	float zoom_speed() const;

	glm::mat4 view_matrix;
	glm::vec3 position = { 5, 5, -5 };
	glm::vec3 direction;
	glm::vec3 focal_point { 0.0f };

	float vertical_fov;
	float aspect_ratio;
	float near_clip;
	float far_clip;

	bool active = false;
	glm::vec2 initial_mouse_position {};

	float distance;
	float normal_speed { 0.002f };

	float pitch = glm::radians(-30.0f);
	float yaw = 0;
	float pitch_delta {};
	float yaw_delta {};
	glm::vec3 position_delta {};
	glm::vec3 right_direction {};

	CameraMode camera_mode { CameraMode::Arcball };

	float min_focus_distance { 100.0f };

	std::uint32_t viewport_width { 1280 };
	std::uint32_t viewport_height { 720 };

	constexpr static float min_speed { 0.002f };
	constexpr static float max_speed { 0.02f };
};

} // namespace Disarray
