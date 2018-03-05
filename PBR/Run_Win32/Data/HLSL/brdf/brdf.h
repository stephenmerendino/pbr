float3 importance_sample_ggx(float2 Xi, float non_linear_roughness, float3 normal)
{
	//roughness = pow(roughness + 1.0f, 2.0f);
	float a = non_linear_roughness * non_linear_roughness;
	float a2 = a * a;

	float phi_h = Xi.x * TWO_PI;
	float cos_theta_h = sqrt((1.0f - Xi.y) / (1.0f + (a2 - 1.0f) * Xi.y));
	float sin_theta_h = sqrt(1.0f - min(1.0f, cos_theta_h * cos_theta_h));

	float3 half_vec;
	half_vec.x = sin_theta_h * cos(phi_h);
	half_vec.y = sin_theta_h * sin(phi_h);
	half_vec.z = cos_theta_h;

	float3 up = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent_x = normalize(cross(up, normal));
	float3 tangent_y = normalize(cross(normal, tangent_x));

	return tangent_x * half_vec.x + tangent_y * half_vec.y + normal * half_vec.z;
}

void importance_sample_cos_dir(float2 random_sample,
							   float3 normal,
							   out float3 light,
							   out float n_dot_l,
							   out float pdf)
{
	float3 up = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(0, 1, 0);
	float3 local_right = normalize(cross(up, normal));
	float3 local_forward = cross(normal, local_right);

	float r = sqrt(random_sample.x);
	float phi = random_sample.y * TWO_PI;

	light = float3(r * cos(phi), r * sin(phi), sqrt(max(0.0f, 1.0f - random_sample.x)));
	light = normalize(local_right * light.x + local_forward * light.y + normal * light.z);

	n_dot_l = saturate(dot(normal, light));
	pdf = n_dot_l * INV_PI;
}

float3 F_approx_schlick(float3 f0, float f90, float u)
{
	return f0 + ((f90 - f0) * pow(1.0f - u, 5.0f));
}

float G_calc_smith_ggx_remapped(float n_dot_v, float n_dot_l, float non_linear_roughness)
{
	float alpha_g = 0.5 + (non_linear_roughness * 0.5f);
	float alpha_g2 = alpha_g * alpha_g;

	float lambda_ggx_v = n_dot_l * sqrt((-n_dot_v * alpha_g2 + n_dot_v) * n_dot_v + alpha_g2);
	float lambda_ggx_l = n_dot_v * sqrt((-n_dot_l * alpha_g2 + n_dot_l) * n_dot_l + alpha_g2);

	return 0.5f / (lambda_ggx_v + lambda_ggx_l);
}

float G_calc_smith_ggx(float n_dot_v, float n_dot_l, float non_linear_roughness)
{
	float alpha_g = non_linear_roughness * non_linear_roughness;
	float alpha_g2 = alpha_g * alpha_g;

	float lambda_ggx_v = n_dot_l * sqrt((-n_dot_v * alpha_g2 + n_dot_v) * n_dot_v + alpha_g2);
	float lambda_ggx_l = n_dot_v * sqrt((-n_dot_l * alpha_g2 + n_dot_l) * n_dot_l + alpha_g2);

	return 0.5f / (lambda_ggx_v + lambda_ggx_l);
}

float D_calc_ggx(float n_dot_h, float m)
{
	float m2 = (m * m);
	float f = (n_dot_h * m2 - n_dot_h) * n_dot_h + 1;
	return m2 / (f * f);
}

float3 calculate_spec_energy(float3 f0,
					         float l_dot_h,
					         float n_dot_v,
					         float n_dot_l,
					         float n_dot_h,
					         float non_linear_roughness)
{
	float linear_roughness = non_linear_roughness * non_linear_roughness;
	float3 fresnel = F_approx_schlick(f0, 1.0f, l_dot_h);
	float vis = G_calc_smith_ggx_remapped(n_dot_v, n_dot_l, non_linear_roughness);
	float ndf = D_calc_ggx(n_dot_h, linear_roughness);
	return fresnel * vis * ndf;
}

float calculate_diffuse_energy(float l_dot_h,
	                           float n_dot_v,
	                           float n_dot_l,
	                           float non_linear_roughness)
{
	float linear_roughness = non_linear_roughness * non_linear_roughness;

	float energy_bias = lerp(0.0f, 0.5f, linear_roughness);
	float energy_factor = lerp(1.0f, 1.0f / 1.51f, linear_roughness);
	float fd90 = energy_bias + 2.0f * l_dot_h * l_dot_h * linear_roughness;
	float3 f0 = float3(1.0f, 1.0f, 1.0f);

	float light_scatter = F_approx_schlick(f0, fd90, n_dot_l).r;
	float view_scatter = F_approx_schlick(f0, fd90, n_dot_v).r;

	return light_scatter * view_scatter * energy_factor;
}