#pragma once

#include "Engine/Renderer/transform.h"
#include "Engine/Math/Matrix4.hpp"

class Camera
{
    public:
        Transform m_transform;
        float m_movement_speed;
        float m_mouse_sensitivity;

        float m_nz;
        float m_fz;
        float m_fov;
        float m_aspect;

        float m_yaw;
        float m_pitch;

    public:
        Camera();
        
        void update(float ds);
        Matrix4 get_view();
        Matrix4 get_projection();
        Vector3 get_world_position();

		void look_at(const Vector3& world_pos, const Vector3& up = Vector3::Y_AXIS);
};