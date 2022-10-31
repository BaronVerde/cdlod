
#include "camera.h"
#include "base/glfw_window.h"
#include "base/logbook.h"
#include "omath/mat4.h"
#include <sstream>
#include <iostream>
#include <string>
#include "imgui/imgui.h"

namespace orf_n {

camera::camera( glfw_window *win,
				omath::dvec3 position,
				omath::dvec3 target,
				omath::dvec3 up,
				double nearPlane,
				double farPlane,
				camera_mode mode ) :
						event_handler{ win },
						m_window{ win },
						m_position{ position },
						m_target{ target },
						m_nearPlane{ nearPlane },
						m_farPlane{ farPlane },
						m_up{ up },
						m_mode{ mode } {

	// center of the screen for reset of captured mouse coursor.
	m_centerOfScreen = { (float)m_window->get_width() / 2.0f, (float)m_window->get_height() / 2.0f };

	// register input events with the event handler
	unsigned int cb = event_handler::KEY |
					  event_handler::MOUSE_MOVE |
					  event_handler::MOUSE_BUTTON |
					  event_handler::MOUSE_SCROLL |
					  event_handler::FRAMEBUFFER_RESIZE;
	event_handler::registered_object_t o{ this, cb };
	register_object( o );

	calculate_initial_angles();
	calculate_fov();
	update_camera_vectors();
}

void camera::calculate_fov() {
	const double width = (double)m_window->get_width();
	const double height = (double)m_window->get_height();
	m_perspectiveMatrix = omath::perspective( omath::radians( m_zoom ), width / height, m_nearPlane, m_farPlane );
	const double zModMul = 1.001;
	const double zModAdd = 0.01;
	m_zOffsetPerspectiveMatrix = omath::perspective(
			omath::radians( m_zoom ), width / height, m_nearPlane * zModMul + zModAdd, m_farPlane * zModMul + zModAdd
	);
	m_frustum.set_fov( m_zoom, width / height, m_nearPlane, m_farPlane );
}

void camera::calculate_initial_angles() {
	// Calculate initial yaw and pitch on change of position or target
	// m_distanceToTarget is float, direction does not need the precision
	// @todo different cases for camera mode
	omath::vec3 direction{ m_target - m_position };
	m_distanceToTarget = omath::magnitude( m_target - m_position );
	std::ostringstream s;
	s << "Calculating new camera angles for position " << m_position << "; target " <<
			m_target << "; distance " << m_distanceToTarget << '.';
	logbook::log_msg( logbook::WINDOW,logbook::INFO,s.str() );
	m_yaw = omath::degrees( std::atan2( -direction.x, direction.z ) );
	m_pitch = omath::degrees( std::atan2( direction.y,
			std::sqrt( ( direction.x * direction.x) + ( direction.y * direction.y ) ) ) );
	update_camera_vectors();
}

void camera::update_camera_vectors() {
	if( ORBITING == m_mode ) {
		//m_distanceToTarget = omath::length( m_target - m_position );
		const double yaw{ -omath::radians( m_yaw ) };
		const double pitch{ -omath::radians( m_pitch ) };
		const double cp{ std::cos( pitch ) };
		// Calculate the camera position on the sphere around the target
		m_position = omath::dvec3{ m_distanceToTarget * std::sin( yaw ) * cp,
								   m_distanceToTarget * std::sin( pitch ),
								   -m_distanceToTarget * std::cos( yaw ) * cp };
		m_front = omath::normalize( m_target - m_position );
		// conversion to float because precision isn't needed here
		m_viewMatrix = omath::lookAt( m_position, m_target, m_up );
		m_frustum.set_camera_vectors( m_position, m_target, m_up );
	} else {
		const omath::dvec3 posf{ m_position };
		// flip yaw and pitch because intuition (at least mine ;-))
		const double yaw{ omath::radians( m_pitch ) };
		const double pitch{ omath::radians( m_yaw ) };
		const double cy{ std::cos( yaw ) };
		// Calculate the new front and right vectors. camera position is set by movement.
		omath::dvec3 front { -cy * std::sin( pitch ), std::sin( yaw ),  cy * std::cos( pitch ) };
		m_front = omath::normalize( front );
		m_viewMatrix = omath::lookAt( posf, posf + front, m_up );
		m_frustum.set_camera_vectors( posf, posf + front, m_up );
	}
	// Also re-calculate the right vector
	m_right = omath::normalize( omath::cross( m_front, m_up ) );
	// prefab projection * view
	m_viewPerspectiveMatrix = m_perspectiveMatrix * m_viewMatrix;
	// prefab proj * untranslated view matrix, stripped of translation
	m_untranslatedViewPerspectiveMatrix = m_perspectiveMatrix *
			omath::dmat4{ m_viewMatrix[0], m_viewMatrix[1], m_viewMatrix[2], omath::dvec4{ 0.0, 0.0, 0.0, 1.0 } };
}

// TODO double
bool camera::on_mouse_move( float x, float y ) {
	if( !m_window->is_cursor_disabled() )
		return false;
	bool handled{ false };
	// @todo/fixme cursor should center automatically, according to glfw doc, but doesn't
	glfwSetCursorPos( m_window->get_window(), m_centerOfScreen.x, m_centerOfScreen.y );
	m_pitch += m_mouseSensitivity * ( m_centerOfScreen.y - y );
	if( FIRST_PERSON == m_mode )
		m_yaw -= m_mouseSensitivity * ( m_centerOfScreen.x - x );
	else
		m_yaw += m_mouseSensitivity * ( m_centerOfScreen.x - x );
	// So screen doesn't get flipped
	if( m_pitch > 89.0 )
		m_pitch = 89.0;
	if( m_pitch < -89.0 )
		m_pitch = -89.0;
	// Update camera vectors and view frustum
	update_camera_vectors();
	handled = true;
	return handled;
}	// onMouseMove()

// virtual
bool camera::on_mouse_scroll( float xOffset, float yOffset ) {
	bool handled{ false };
	/* if( yOffset != 0 ) {
		if( m_zoom >= 1.0f && m_zoom <= 60.0f )
			m_zoom -= yOffset;
		if( m_zoom <= 1.0f )
			m_zoom = 1.0f;
		if( m_zoom >= 60.0f )
			m_zoom = 60.0f;
		calculateFOV();
		updatecameraVectors();
		handled = true;
	} */
	return handled;
}	// onMouseScroll()

// virtual
bool camera::on_key_pressed( int key, int scancode, int action, int mods ) {
	bool handled{ false };
	// movement is mirrored between fps and orbiting mode
    if( GLFW_PRESS == action ) {
    	switch( key ) {
    		case GLFW_KEY_P:
    			print_position();
    			handled = true;
    			break;
    		case GLFW_KEY_M:
    			m_frustum.print();
    			handled = true;
    			break;
    		case GLFW_KEY_W:
    			m_isMoving = true;
    			if( FIRST_PERSON == m_mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_FORWARD;
    				else
    					m_direction = FORWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_CLOSE;
    				else
    					m_direction = CLOSE;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_S:
    			m_isMoving = true;
    			if( FIRST_PERSON == m_mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_BACKWARD;
    				else
    					m_direction = BACKWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					m_direction = FAST_RETREAT;
    				else
    					m_direction = RETREAT;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_A:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_LEFT;
    			else
    				m_direction = LEFT;
    			handled = true;
    			break;
    		case GLFW_KEY_D:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_RIGHT;
    			else
    				m_direction = RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_Z:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_DOWN;
    			else
    				m_direction = DOWN;
    			handled = true;
    			break;
    		case GLFW_KEY_X:
    			m_isMoving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				m_direction = FAST_UP;
    			else
    				m_direction = UP;
    			handled = true;
    			break;
    		case GLFW_KEY_Q:
    			m_isMoving = true;
    			m_direction = ROTATE_LEFT;
    			break;
    		case GLFW_KEY_E:
    			m_isMoving = true;
    			m_direction = ROTATE_RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_TAB:
    			if( ORBITING == m_mode ) {
    				// @todo Set initial values from orbiting cam
    				m_mode = FIRST_PERSON;
    				set_position_and_target( m_position, m_target );
    			} else {
    				m_mode = ORBITING;
    				update_camera_vectors();
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_LEFT_ALT:
    			m_window->toggle_input_mode();
    			handled = true;
    			break;
    		case GLFW_KEY_F:
    			m_wireframe = !m_wireframe;
    			handled = true;
    			break;
    		case GLFW_KEY_ESCAPE:
    			m_window->set_should_close();
    			handled = true;
    			break;
    	}
    }
    if( GLFW_RELEASE == action && ( GLFW_KEY_W == key || GLFW_KEY_S == key ||
    								GLFW_KEY_A == key || GLFW_KEY_D == key ||
									GLFW_KEY_Z == key || GLFW_KEY_X == key ||
									GLFW_KEY_Q == key || GLFW_KEY_E == key ) ) {
    	m_isMoving = false;
    	handled = true;
    }
    // If moving, camera vectors and frustum planes are updated in updateMoving()
    return handled;
}	// onKeyPressed()

// virtual
bool camera::on_mouse_button( int button, int action, int mods ) {
	bool handled = false;
	// when clicked, cast ray and find out where
	return handled;
}	// onMouseButton()

bool camera::on_framebuffer_resize( int width, int height ) {
	bool handled = false;
	m_window->set_width( width );
	m_window->set_height( height );
	calculate_fov();
	// Tell the renderer that framebuffer must be resized
	m_window->set_damaged( true );
	return handled;
}	// onMouseButto

void camera::update_moving(const double delta_time) {
	/* Has been transferred to the application for view dependent calculations
	if( globals::showAppUI ) {
		const float oldNearPlane{ m_nearPlane };
		const float oldFarPlane{ m_farPlane };
		const float oldZoom{ m_zoom };
		ImGui::Begin( "camera Control" );
		ImGui::SliderFloat( "Near plane", &m_nearPlane, 0.1f, 10.0f );
		ImGui::SliderFloat( "Far plane", &m_farPlane, 20.0f, 2000.0f );
		ImGui::SliderFloat( "Zoom angle", &m_zoom, 30.0f, 60.0f );
		ImGui::End();
		if( oldNearPlane != m_nearPlane || oldFarPlane != m_farPlane || oldZoom != m_zoom )
			calculateFOV();
	}*/
	if( m_isMoving ) {
		double velocity = m_movementSpeed * delta_time;
		switch( m_direction ) {
			// up, front and right must be unit vectors or else you will be lost in space.
			case FAST_FORWARD:
				m_position += m_front * 10.0 * velocity;
				break;
			case FORWARD:
				m_position += m_front * velocity;
				break;
			case FAST_BACKWARD:
				m_position -= m_front * 10.0 * velocity;
				break;
			case BACKWARD:
				m_position -= m_front * velocity;
				break;
			case FAST_LEFT:
				m_position -= m_right * 10.0 * velocity;
				break;
			case LEFT:
				m_position -= m_right * velocity;
				break;
			case FAST_RIGHT:
				m_position += m_right * 10.0 * velocity;
				break;
			case RIGHT:
				m_position += m_right * velocity;
				break;
			case FAST_UP:
				m_position += m_up * 10.0 * velocity;
				break;
			case UP:
				m_position += m_up * velocity;
				break;
			case FAST_DOWN:
				m_position -= m_up * 10.0 * velocity;
				break;
			case DOWN:
				m_position -= m_up * velocity;
				break;
			case ROTATE_LEFT:
				break;
			case ROTATE_RIGHT:
				break;
			case FAST_CLOSE:
				m_distanceToTarget -= 10.0 * velocity;
				break;
			case CLOSE:
				m_distanceToTarget -= velocity;
				break;
			case FAST_RETREAT:
				m_distanceToTarget += 10.0 * velocity;
				break;
			case RETREAT:
				m_distanceToTarget += velocity;
				break;
		}
	}
	update_camera_vectors();
}

void camera::set_position_and_target( const omath::dvec3 &pos, const omath::dvec3 &target ) {
	m_position = pos;
	m_target = target;
	calculate_initial_angles();
}

const double &camera::get_zoom() const {
	return m_zoom;
}

void camera::set_zoom( const double &zoom ) {
	m_zoom = zoom;
	calculate_fov();
}

void camera::set_up( const omath::dvec3 &up ) {
	m_up = up;
}

const double &camera::get_near_plane() const {
	return m_nearPlane;
}

const double &camera::get_far_plane() const {
	return m_farPlane;
}

void camera::set_near_plane( const double &np ) {
	m_nearPlane = np;
	calculate_fov();
}

void camera::set_far_plane( const double &fp ) {
	m_farPlane = fp;
	calculate_fov();
}

void camera::set_mode( const camera_mode mode ) {
	m_mode = mode;
}

void camera::set_movement_speed( const float &speed ) {
	m_movementSpeed = speed;
}

void camera::print_position() const {
	std::ostringstream s;
	s<<"Cam position: "<<m_position<<"; target: "<<m_target<<"; mode: "<<(m_mode==FIRST_PERSON?"first person":"orbiting")<<
	   "\nCam front: " << m_front << "; up: " << m_up << "; right: " << m_right <<
	   "\nCamera near: " << m_nearPlane << "; far: " << m_farPlane;
	logbook::log_msg( logbook::WINDOW,logbook::INFO,s.str() );
}

const omath::dmat4 &camera::get_view_matrix() const {
	return m_viewMatrix;
}

const omath::dmat4 &camera::get_zoffset_perspective_matrix() const {
	return m_zOffsetPerspectiveMatrix;
}

const omath::dmat4 &camera::get_perspective_matrix() const {
	return m_perspectiveMatrix;
}

const omath::dmat4 &camera::get_view_perspective_matrix() const {
	return m_viewPerspectiveMatrix;
}

const omath::dmat4 &camera::get_untranslated_view_perspective_matrix() const {
	return m_untranslatedViewPerspectiveMatrix;
}

const omath::dvec3 &camera::get_position() const {
	return m_position;
}

const float &camera::get_movement_speed() const {
	return m_movementSpeed;
}

const omath::dvec3 &camera::get_target() const {
	return m_target;
}

const bool &camera::get_wireframe_mode() const {
	return m_wireframe;
}

const omath::view_frustum& camera::get_view_frustum() const {
	return m_frustum;
}

const omath::dvec3 &camera::get_front() const {
	return m_front;
}

const omath::dvec3 &camera::get_right() const {
	return m_right;
}

const omath::dvec3 &camera::get_up() const {
	return m_up;
}

camera::~camera() {
	de_register_object( this );
}

}
