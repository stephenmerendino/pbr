#include "Engine/Renderer/camera.h"
#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Math/MathUtils.hpp"

Camera::Camera()
    :m_movement_speed(5.0f)
    ,m_mouse_sensitivity(0.2f)
    ,m_nz(0.1f)
    ,m_fz(1000.0f)
    ,m_fov(67.7f)
    ,m_aspect(16.0f / 9.0f)
    ,m_yaw(0.0f)
    ,m_pitch(0.0f)
{
}

void Camera::update(float ds)
{
    float moveDistance = m_movement_speed * ds;

    if(g_theInputSystem->IsKeyDown(KEYCODE_SHIFT)){
        moveDistance *= 0.1f;
    }

    if(g_theInputSystem->WasKeyJustPressed('O')){
        m_transform.m_local = Matrix4::IDENTITY;
    }

    Vector3 world_pos = m_transform.calc_world_position();
    Vector3 forward_xz = m_transform.calc_forward_xz();
    Vector3 left = m_transform.calc_world_left();

    // Keyboard position
    if(g_theInputSystem->IsKeyDown('W')){
        world_pos += forward_xz * moveDistance;
    }

    if(g_theInputSystem->IsKeyDown('S')){
        world_pos += forward_xz * -moveDistance;
    }

    if(g_theInputSystem->IsKeyDown('D')){
        world_pos += left * -moveDistance;
    }

    if(g_theInputSystem->IsKeyDown('A')){
        world_pos += left * moveDistance;
    }

    if(g_theInputSystem->IsKeyDown('E') || g_theInputSystem->IsKeyDown(KEYCODE_SPACE)){
        world_pos.y += moveDistance;
    }

    if(g_theInputSystem->IsKeyDown('Q') || g_theInputSystem->IsKeyDown('Z')){
        world_pos.y -= moveDistance;
    }

    // Mouse orientation
    IntVector2 mouseDeltaMove = g_theInputSystem->GetMouseMoveDelta();

    float mouseMoveDx = (float)mouseDeltaMove.x;
    float yaw = mouseMoveDx * m_mouse_sensitivity;

    float mouseMoveDy = (float)mouseDeltaMove.y;
    float pitch = mouseMoveDy * m_mouse_sensitivity;

    KeepDegrees0To360(yaw);

    m_yaw += yaw;
    m_pitch += pitch;

    m_pitch = Clamp(m_pitch, -89.9f, 89.9f);

    m_transform.set_world_rotation(m_yaw, m_pitch, 0.0f);
    m_transform.set_world_position(world_pos);
}

Matrix4 Camera::get_view()
{
    Matrix4 inv_pos = Matrix4::make_translation(-m_transform.calc_world_position());
    Matrix4 inv_rot = m_transform.calc_world_rotation().transposed();
    return inv_pos * inv_rot;
}

Matrix4 Camera::get_projection()
{
    return RHIInstance::GetInstance().CreatePerspectiveProjection(m_nz, m_fz, m_fov, m_aspect);
}

Vector3 Camera::get_world_position()
{
    return m_transform.calc_world_position();
}

void Camera::look_at(const Vector3& world_pos, const Vector3& up)
{
	m_transform.look_at(world_pos, up);

	m_yaw = m_transform.calc_yaw_degrees();
	m_pitch = m_transform.calc_pitch_degrees();
}