#pragma once

#include "Engine/Math/Matrix4.hpp"

class Transform
{
public:
    Matrix4           m_local;
    const Transform*  m_parent;

public:
    Transform();
    Transform(Matrix4 local, const Transform* parent = nullptr);

    void        set_parent(const Transform* parent);
    Matrix4     calc_world_matrix() const;
    Transform   calc_world_transform() const;
    void        look_at(const Vector3& location, const Vector3& up = Vector3::Y_AXIS);

    Matrix4 calc_world_rotation() const;
    Matrix4 calc_local_rotation() const;
    Matrix4 calc_world_translation() const;

    Vector3 calc_world_position() const;
    Vector3 calc_world_forward() const;
	Vector3 calc_forward_xz() const;
    Vector3 calc_world_left() const;

	float calc_yaw_degrees() const;
	float calc_pitch_degrees() const;

    void set_world_position(const Vector3& world_pos);
    void set_world_position(float x, float y, float z);
    void set_world_rotation(const Matrix4& world_rot);
    void set_world_rotation(float yaw, float pitch, float roll);
    void set_world_scale(float x_scale, float y_scale, float z_scale);

    static Transform IDENTITY;
};