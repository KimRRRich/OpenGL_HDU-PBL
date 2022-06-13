#pragma once
#include "ObjLoader.h"
//颜色值
struct my_draw_color
{
	float r;
	float g;
	float b;
	my_draw_color operator +(const my_draw_color& input_color) {
		return my_draw_color{ r + input_color.r,g + input_color.g,b + input_color.b };
	}
	my_draw_color operator *(const float& cons) {
		return my_draw_color{ r * cons,g * cons,b * cons };
	}
};
//Blinn-Phong Reflection Model
my_draw_color calculate_direct_light_on_one_vertex_usingBPRM(my_3D_point_coord
	input_vertex, my_3Dvector vertex_normal,
	my_3D_point_coord eye_position, my_3D_point_coord light_position,
	float light_rgb_ambient[], float material_ambient_rgb_reflection[],
	float light_rgb_diffuse_specular[], float material_diffuse_rgb_reflection[],
	float material_specular_rgb_reflection[], float ns) {
	//环境光反射能量
	float rval = light_rgb_ambient[0] * material_ambient_rgb_reflection[0];
	float gval = light_rgb_ambient[1] * material_ambient_rgb_reflection[1];
	float bval = light_rgb_ambient[2] * material_ambient_rgb_reflection[2];
	//漫反射光能量——需保证非零增长角度在0-90
	my_3Dvector pointTolight_vector(input_vertex, light_position);
	pointTolight_vector.normalized();
	float costheta = vertex_normal.dot(pointTolight_vector);
	costheta = (costheta > 0.0) ? costheta : 0.0;
	rval += light_rgb_diffuse_specular[0] * material_diffuse_rgb_reflection[0] * costheta;
	gval += light_rgb_diffuse_specular[1] * material_diffuse_rgb_reflection[1] * costheta;
	bval += light_rgb_diffuse_specular[2] * material_diffuse_rgb_reflection[2] * costheta;
	//镜面高光能量
	my_3Dvector pointToView_vector(input_vertex, eye_position);
	pointToView_vector.normalized();
	my_3Dvector half_vector = pointTolight_vector + pointToView_vector;
	half_vector = half_vector * 0.5;
	float costheta2 = vertex_normal.dot(half_vector);
	costheta2 = (costheta2 > 0.0) ? costheta2 : 0.0;
	float special_coeff = powf(costheta2, ns);
	rval += light_rgb_diffuse_specular[0] * material_specular_rgb_reflection[0] * special_coeff;
	gval += light_rgb_diffuse_specular[1] * material_specular_rgb_reflection[1] * special_coeff;
	bval += light_rgb_diffuse_specular[2] * material_specular_rgb_reflection[2] * special_coeff;
	my_draw_color output_color = { rval,gval,bval };
	return output_color;
}

