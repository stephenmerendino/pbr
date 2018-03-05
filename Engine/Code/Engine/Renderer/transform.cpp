#include "Engine/Renderer/transform.h"

Transform Transform::IDENTITY;

Transform::Transform()
    :m_local(Matrix4::IDENTITY)
    ,m_parent(nullptr)
{
}

Transform::Transform(Matrix4 local, const Transform* parent)
    :m_local(local)
    ,m_parent(parent)
{
}

void Transform::set_parent(const Transform* parent)
{
    if(nullptr == parent){
        m_parent = &Transform::IDENTITY;
    }else{
        m_parent = parent;
    }
}

Matrix4 Transform::calc_world_matrix() const
{
    if(nullptr == m_parent){
        return m_local;
    }

    return m_local * m_parent->calc_world_matrix();
}

Transform Transform::calc_world_transform() const
{
    return Transform(calc_world_matrix(), nullptr);
}

void Transform::look_at(const Vector3& location, const Vector3& up)
{
    Vector3 center = m_local.get_translation().xyz;

	Vector3 k_basis = (location - center).Normalized();
	Vector3 i_basis = CrossProduct(up, k_basis);
    Vector3 j_basis = CrossProduct(k_basis, i_basis);

	// Set basis vectors of transform matrix
	m_local.set_i_basis(i_basis, 0.0f);
	m_local.set_j_basis(j_basis, 0.0f);
	m_local.set_k_basis(k_basis, 0.0f);
}

Matrix4 Transform::calc_world_rotation() const
{
    Matrix4 rot = calc_world_matrix();
    rot.set_translation(Vector3::ZERO, 1.0f);
    return rot;
}

Matrix4 Transform::calc_local_rotation() const
{
    Matrix4 rot = m_local;
    rot.set_translation(Vector3::ZERO, 1.0f);
    return rot;
}

Matrix4 Transform::calc_world_translation() const
{
    Vector3 world_pos = calc_world_position();
    return Matrix4::make_translation(world_pos);
}

Vector3 Transform::calc_world_position() const
{
    Matrix4 world = calc_world_matrix();
    return world.get_translation().xyz;
}

Vector3 Transform::calc_world_forward() const
{
    Vector4 default_forward(0.0f, 0.0f, 1.0f, 0.0f);
    Matrix4 world = calc_world_matrix();
    Vector4 forward = default_forward * world;
    return forward.xyz;
}

Vector3 Transform::calc_world_left() const
{
    Vector4 default_left(-1.0f, 0.0f, 0.0f, 0.0f);
    Matrix4 world = calc_world_matrix();
    Vector4 forward = default_left * world;
    return forward.xyz;
}

Vector3 Transform::calc_forward_xz() const
{
	Vector3 forward = calc_world_forward();

	Vector3 vertical_component = Vector3::Y_AXIS * (DotProduct(forward, Vector3::Y_AXIS));
	Vector3 horizontal_component = forward - vertical_component;
	horizontal_component.Normalize();

	return horizontal_component;
}

float Transform::calc_yaw_degrees() const
{
	Vector3 xz_forward = calc_forward_xz();

	float cos_theta = DotProduct(xz_forward, Vector3::Z_AXIS);
	float yaw_rads = -1.0f * acosf(cos_theta);

	return ConvertRadiansToDegrees(yaw_rads);
}

float Transform::calc_pitch_degrees() const
{
	Vector3 forward = calc_world_forward();
	Vector3 xz_forward = calc_forward_xz();

	float cos_phi = DotProduct(forward, xz_forward);
	float pitch_rads = acosf(cos_phi);

	return ConvertRadiansToDegrees(pitch_rads);
}

void Transform::set_world_position(const Vector3& world_pos)
{
    Matrix4 world = calc_world_matrix();
    Vector3 current_position = world.get_translation().xyz;
    Vector3 relative_delta = world_pos - current_position;
    m_local.translate(relative_delta);
}

void Transform::set_world_position(float x, float y, float z)
{
	set_world_position(Vector3(x, y, z));
}

void Transform::set_world_rotation(const Matrix4& world_rot)
{
    m_local.set_i_basis(world_rot.get_i_basis());
    m_local.set_j_basis(world_rot.get_j_basis());
    m_local.set_k_basis(world_rot.get_k_basis());
}

void Transform::set_world_rotation(float yaw, float pitch, float roll)
{
    Matrix4 yaw_rot = Matrix4::make_rotation_y_degrees(yaw);
    Matrix4 pitch_rot = Matrix4::make_rotation_x_degrees(pitch);
    Matrix4 roll_rot = Matrix4::make_rotation_z_degrees(roll);
    Matrix4 orientation = roll_rot * pitch_rot * yaw_rot;
    set_world_rotation(orientation);
}

void Transform::set_world_scale(float x_scale, float y_scale, float z_scale)
{
    Matrix4 scale = Matrix4::make_scale(x_scale, y_scale, z_scale);
    Matrix4 rot = calc_world_rotation();
    Matrix4 trans = calc_world_translation();
    m_local = scale * rot * trans;
}