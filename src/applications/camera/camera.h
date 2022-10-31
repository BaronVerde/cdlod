
// Terrain camera with double precision.

#pragma once

#include "base/event_handler.h"
#include "omath/view_frustum.h"
#include "omath/mat4.h"

namespace orf_n {

class glfw_window;
class Plane;

class camera : public event_handler {
public:
	// Terrain is a first person camera with double precision view matrix
	typedef enum {
		ORBITING = 0, FIRST_PERSON, TERRAIN
	} camera_mode;

	typedef enum {
		// Movement of first person camera
		FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN, ROTATE_LEFT, ROTATE_RIGHT,
		FAST_FORWARD, FAST_BACKWARD, FAST_RIGHT, FAST_LEFT, FAST_UP, FAST_DOWN,
		// Movemtn of orbiting camera
		CLOSE, RETREAT, FAST_CLOSE, FAST_RETREAT
	} movement_direction;

	/* Common camera object, will be stored in the scene for all renderable objects
	 * to access it. Needs pointer to window, position and target as double vectors,
	 * camera up vector (default y up), near and far plane as floats and the mode,
	 * if it is an orbiting camera around the target or a free moving first person type. */
	camera(
			glfw_window *win,
			omath::dvec3 position,
			omath::dvec3 target,
			omath::dvec3 up = omath::dvec3{ 0.0, 1.0, 0.0 },
			double neaPlane = 1.0,
			double farPlane = 100.0,
			camera_mode mode = ORBITING
	);

	virtual ~camera();

	const omath::dmat4 &get_view_matrix() const;

	const omath::dmat4 &get_view_perspective_matrix() const;

	const omath::dmat4 &get_untranslated_view_perspective_matrix() const;

	const omath::view_frustum &get_view_frustum() const;

	const omath::dvec3 &get_front() const;

	const omath::dvec3 &get_right() const;

	const omath::dvec3 &get_up() const;

	const omath::dmat4 &get_perspective_matrix() const;

	const omath::dmat4 &get_zoffset_perspective_matrix() const;

	const omath::dvec3 &get_position() const;

	const omath::dvec3 &get_target() const;

	void set_position_and_target( const omath::dvec3 &pos, const omath::dvec3 &target );

	void set_mode( const camera_mode mode );

	void set_up( const omath::dvec3 &up );

	const float &get_movement_speed() const;

	void set_movement_speed( const float &speed );

	// Zoom angle of camera in degrees
	void set_zoom( const double &zoom );

	// get zoom in degrees
	const double &get_zoom() const;

	const bool &get_wireframe_mode() const;

	// Must be called on near/far plane or angle change.
	void calculate_fov();

	// Update camera and movement. Must be called every frame for continous movemnt.
	void update_moving(const double delta_time);

	const double &get_near_plane() const;

	const double &get_far_plane() const;

	void set_near_plane( const double &np );

	void set_far_plane( const double &fp );

private:
	glfw_window *m_window{nullptr};

	omath::dvec3 m_position{ omath::dvec3{ 1.0 } };

	// Orbiting camera target.
	omath::dvec3 m_target{ omath::dvec3{ 0.0 } };

	double m_distanceToTarget;

	double m_nearPlane{ 10.0 };

	double m_farPlane{ 10000.0 };

	double m_zoom{ 45.0 };

	omath::view_frustum m_frustum;

	omath::dmat4 m_viewMatrix;

	/* The scene's projection matrix. A renderable object can have its own projection matrix.
	 * Moved here from the scene for better access for frustum calculation. */
	omath::dmat4 m_perspectiveMatrix;

	// Perspective projection * view Matrix pre-comp.
	omath::dmat4 m_viewPerspectiveMatrix;

	omath::dmat4 m_untranslatedViewPerspectiveMatrix;

	omath::dmat4 m_zOffsetPerspectiveMatrix;

	omath::dvec3 m_front;

	omath::dvec3 m_up{ omath::dvec3{ 0.0, 1.0, 0.0 } };

	// omath::vec3 m_worldUp;

	omath::dvec3 m_right;

	double m_yaw{ 0.0 };

	double m_pitch{ 0.0 };

	camera_mode m_mode{ ORBITING };

	float m_movementSpeed{ 30.0f };

	//  Moving starts with key press, ends with key release.
	bool m_isMoving{ false };

	// Movement direction if camera is moving in firstperson mode, relative to camera axes.
	movement_direction m_direction;

	float m_mouseSensitivity{ 0.02f };

	/* Center of screeen (width and height/2) for relative mouse movement.
	 * FIXME: this should be unnecessary to carry along. doublecheck glfw3 documentation. */
	omath::vec2 m_centerOfScreen;

	// Switch between wireframe and textured display. Used internally.
	bool m_wireframe{ false };

	bool on_key_pressed( int key, int scancode, int action, int mods ) override final;

	bool on_mouse_move( float x, float y ) override final;

	bool on_mouse_button( int button, int action, int mods ) override final;

	bool on_mouse_scroll( float xOffset, float yOffset ) override final;

	bool on_framebuffer_resize( int width, int height ) override final;

	/* Helper function to calculate distance, yaw and pitch on creation and
	 * when position or target have been changed. */
	void calculate_initial_angles();

	/* Helper func calculates cam vectors on key press or mous move, depending on mode.
	 * Depend on prior calculateInitialValues() on camera creation or change of position or target ! */
	void update_camera_vectors();

	// debug output
	void print_position() const;

};

}	// namespace
