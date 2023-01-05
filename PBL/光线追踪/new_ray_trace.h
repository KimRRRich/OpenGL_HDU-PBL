#pragma once
#include  "lineTriangle3DIntersection.h"
#include  "direct_lighting.h"
#include  <atlconv.h>
#include  <gdiplus.h>


#pragma comment(lib,  "gdiplus.lib")
using namespace  Gdiplus;
#define M_PI  3.141592653589793
#define CUR_INFINITY  1e8
#define MAX_RAY_DEPTH  2
void samplepoint_sonprojectionplan(float left_x, float right_x, float bottom_y, float up_y, float
	nearplane_distance, float eye_z, std::map<my_3D_point_coord*, my_draw_color*>&
	render_vertices, unsigned& width, unsigned& height)
{
	//对投影区域进行采样密度设置
	float x_delt = (right_x - left_x) / (ceil(right_x) - floor(left_x));
	float y_delt = (up_y - bottom_y) / (ceil(up_y) - floor(bottom_y));
	float z_val = eye_z - nearplane_distance - 1;
	width = 0;
	height = 0;
	bool counted = false;
	for (float x_iter = left_x; x_iter <= right_x; x_iter += x_delt)
	{
		width++;
		for (float y_iter = bottom_y; y_iter <= up_y; y_iter += y_delt)
		{
			my_3D_point_coord* tempPoint_ptr = new my_3D_point_coord(x_iter, y_iter,
				z_val);
			my_draw_color* tempColor_ptr = new my_draw_color{ 0,0,0 };
			render_vertices.insert(pair<my_3D_point_coord*,
				my_draw_color*>(tempPoint_ptr, tempColor_ptr));
			if (counted == false)  height++;
		}
		counted = true;
	}
}
//两点间的距离
float distance(my_3D_point_coord p1, my_3D_point_coord  p2)
{
	float dis = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2) + pow((p1.z - p2.z), 2));
	return  dis;
}


//求三角面片中一点的纹理颜色
my_draw_color calculate_texture_color(my_3D_point_coord input_vertex, my_3D_point_coord
	p1, my_3D_point_coord p2, my_3D_point_coord  p3,
	my_2D_Texture_coord p1TextCoord, my_2D_Texture_coord  p2TextCoord,
	my_2D_Texture_coord p3TextCoord, int  nearest_model_index)
{
	//读取纹理图片
	wstring  infilename;
	switch (nearest_model_index)
	{
	case  0:
	{
		infilename = L"红色.bmp";
		break;
	}
	case  1:
	{
		infilename = L"蓝色.bmp";
		break;
	}
	case  2:
	{
		infilename = L"黄色.bmp";
		break;
	}
	}
	//处理纹理重复度问题，纹理坐标在0-1之间
	if (p1TextCoord.u > 1.0)  p1TextCoord.u--;
	else if (p1TextCoord.u < 0.0)  p1TextCoord.u++;
	if (p1TextCoord.v > 1.0)  p1TextCoord.v--;
	else if (p1TextCoord.v < 0.0)  p1TextCoord.v++;
	if (p2TextCoord.u > 1.0)  p2TextCoord.u--;
	else if (p2TextCoord.u < 0.0)  p2TextCoord.u++;
	if (p2TextCoord.v > 1.0)  p2TextCoord.v--;
	else if (p2TextCoord.v < 0.0)  p2TextCoord.v++;
	if (p3TextCoord.u > 1.0)  p3TextCoord.u--;
	else if (p3TextCoord.u < 0.0)  p3TextCoord.u++;
	if (p3TextCoord.v > 1.0)  p3TextCoord.v--;
	else if (p3TextCoord.v < 0.0)  p3TextCoord.v++;


	//求两点间的距离
	float d1 = distance(input_vertex, p1);
	float d2 = distance(input_vertex, p2);
	float d3 = distance(input_vertex, p3);
	//系数
	float sum = d1 + d2 + d3;
	float k1 = d1 / sum;
	float k2 = d2 / sum;
	float k3 = d3 / sum;
	//求交点的纹理坐标
	my_2D_Texture_coord  input_vertex_coord;
	input_vertex_coord.u = k1 * p1TextCoord.u + k2 * p2TextCoord.u + k3 * p3TextCoord.u;
	input_vertex_coord.v = k1 * p1TextCoord.v + k2 * p2TextCoord.v + k3 * p3TextCoord.v;
	//取出该点的纹理颜色
	GdiplusStartupInput  gdiplusstartupinput;
	ULONG_PTR  gdiplustoken;
	GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);
	Bitmap* bmp = new  Bitmap(infilename.c_str());
	UINT height = bmp->GetHeight();
	UINT width = bmp->GetWidth();
	//求交点纹理的下标
	int input_vertex_index_u = floor(width * input_vertex_coord.u + 0.5);
	int input_vertex_index_v = floor(height * input_vertex_coord.v + 0.5);
	Color  color;
	my_draw_color p1color, p2color, p3color, input_vertex_color;
	bmp->GetPixel(input_vertex_index_u, input_vertex_index_v, &color);
	input_vertex_color.r = (float)color.GetRed() / 255;
	input_vertex_color.g = (float)color.GetGreen() / 255;
	input_vertex_color.b = (float)color.GetBlue() / 255;
	delete  bmp;
	GdiplusShutdown(gdiplustoken);
	return  input_vertex_color;
}


//光线折射向量
bool get_refract_dir_my(const my_3Dvector& inpuray_dir, my_3Dvector& nhit, float  ni_over_nt,
	my_3Dvector& refracted_dir)//
{
	my_3Dvector uv(inpuray_dir.dx, inpuray_dir.dy, inpuray_dir.dz);
	uv.normalized();
	float dt = uv.dot(nhit);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		refracted_dir = (uv - nhit * dt) * ni_over_nt - nhit * sqrt(discriminant);
		return true;
	}
	else
		return false;
}
//光线跟踪算法
my_draw_color one_ray_trace_my(my_3D_point_coord rayorig, my_3Dvector raydir, const
	std::vector<my_triangle_3DModel>& all_models, const int& depth,
	const my_3D_point_coord& eye_position, const my_3D_point_coord& light_position, float
	light_rgb_ambient[], float light_rgb_diffuse_specular[])//用于交点直接颜色计算
{
	//超过递归层数，返回000
	if (depth > MAX_RAY_DEPTH)
		return my_draw_color{ 0,0,0 };
	//计算模型（光线的交点）——三角网格遍历与光线求交——一定要注意起点在三角面上的问题
		float nearest_t = INFINITY;
	//射线参数方程中的t
	my_3Dvector  nearestTrangleNormal;
	const my_triangle_3DModel* nearestModel = NULL;
	my_3D_line curRay(rayorig.x, rayorig.y, rayorig.z, raydir.dx, raydir.dy, raydir.dz);
	my_3D_point_coord point1, point2, point3;
	my_2D_Texture_coord textCoord1, textCoord2, textCoord3;
	int  nearest_model_index;
	for (unsigned model_index = 0; model_index < all_models.size(); model_index++)
	{
		for (unsigned int tri_index = 0; tri_index < all_models[model_index].faceSets.size();
			tri_index++)
		{
			//从面实例中取出三个顶点


			int firstPointIndex =
				all_models[model_index].faceSets[tri_index].first_point_index;//取出顶点索引
			int secondPointIndex =
				all_models[model_index].faceSets[tri_index].second_point_index;
			int thirdPointIndex =
				all_models[model_index].faceSets[tri_index].third_point_index;
			int firstTextIndex =
				all_models[model_index].faceSets[tri_index].first_point_texture_index;//纹理顶点索引
			int secondTextIndex =
				all_models[model_index].faceSets[tri_index].second_point_texture_index;
			int thirdTextIndex =
				all_models[model_index].faceSets[tri_index].third_point_texture_index;
			my_3D_point_coord p1 = all_models[model_index].pointSets[firstPointIndex];//第一个顶点
				my_3D_point_coord p2 = all_models[model_index].pointSets[secondPointIndex];
			//第二个顶点
			my_3D_point_coord p3 = all_models[model_index].pointSets[thirdPointIndex];  //第三个顶点
				my_2D_Texture_coord p1TextCoord =
				all_models[model_index].pointTextureSets[firstTextIndex];//第一个点的纹理坐标
			my_2D_Texture_coord p2TextCoord =
				all_models[model_index].pointTextureSets[secondTextIndex];//第二个的纹理坐标
			my_2D_Texture_coord p3TextCoord =
				all_models[model_index].pointTextureSets[thirdTextIndex];//第三个点的纹理坐标
			my_3D_triangle curTriangle = { p1, p2, p3,true };
			IntersectionBetweenLineAndTriangle newIntTest(curRay, curTriangle);
			//不仅有交点，还要求不能是出发点newIntTest.GetLineParameter() > 1e-3
			if (newIntTest.Find() && newIntTest.GetLineParameter() > 0.002f &&
				(newIntTest.GetLineParameter() < nearest_t))
			{
				nearestModel = &all_models[model_index];
				nearestTrangleNormal = newIntTest.GetHitPointNormal();
				nearest_t = newIntTest.GetLineParameter();
				//赋值
				point1 = p1;
				point2 = p2;
				point3 = p3;
				textCoord1 = p1TextCoord;
				textCoord2 = p2TextCoord;


				textCoord3 = p3TextCoord;
				nearest_model_index = model_index;
			}
		}
	}
	if (!nearestModel)
		return depth == 0 ? my_draw_color{ 1,1,1 } : my_draw_color{ 0,0,0 };
	my_3Dvector added_valVec = raydir * nearest_t;
	my_3D_point_coord phit = rayorig.add(added_valVec.dx, added_valVec.dy,
		added_valVec.dz); //获得交点
	my_3Dvector
		nhit = nearestTrangleNormal; //获得交点处的法向
	nhit.normalized();
	my_draw_color phit_textCoord_color = calculate_texture_color(phit, point1, point2, point3,
		textCoord1, textCoord2, textCoord3, nearest_model_index);
	//用Blinn-Phong Reﬂection Model计算交点颜色  ——只要有击中就算一下是否能被光源覆盖到，不管是直射、反射还是折射
		float ambient_rgb_reflection[3] = {
		nearestModel->material_ambient_rgb_reflection[0],nearestModel->material_ambient_rgb_reflection[1],nearestModel->material_ambient_rgb_reflection[2] };
	float diffuse_rgb_reflection[3] = { nearestModel->material_diffuse_rgb_reflection[0],
	nearestModel->material_diffuse_rgb_reflection[1],
	nearestModel->material_diffuse_rgb_reflection[2] };
	float specular_rgb_reflection[3] = {
	nearestModel->material_specular_rgb_reflection[0] ,nearestModel->material_specular_rgb_reflection[1] ,nearestModel->material_specular_rgb_reflection[2] };
	my_draw_color surface_directColor =
		calculate_direct_light_on_one_vertex_usingBPRM(phit, nhit, eye_position, light_position,
			light_rgb_ambient, ambient_rgb_reflection,
			light_rgb_diffuse_specular, diffuse_rgb_reflection,
			specular_rgb_reflection, nearestModel->ns);
	//纹理光照混合
	surface_directColor = surface_directColor * 0.5;
	surface_directColor.r += (phit_textCoord_color.r * 0.5);
	surface_directColor.g += (phit_textCoord_color.g * 0.5);
	surface_directColor.b += (phit_textCoord_color.b * 0.5);
	//若是内部点，说明光线在模型内部走动，则交到的法向反向，目前此种情况只考虑折射光继续，反射光终止
		bool inside = false;


	if (raydir.dot(nhit) > 0)
	{
		inside = true;
		nhit = nhit * -1;
	}
	//计算反射光贡献的颜色
	my_draw_color reflectionColor = { 0,0,0 };
	if (nearestModel->reflection > 0 && inside == false)
	{
		my_3Dvector refldir = raydir - nhit * 2 * raydir.dot(nhit);//计算反射光方向
		refldir.normalized();
		reflectionColor = one_ray_trace_my(phit, refldir, all_models, depth + 1, eye_position,
			light_position, light_rgb_ambient, light_rgb_diffuse_specular); //+ nhit*bias
		reflectionColor = reflectionColor * 0.5;//0.5反射系数
	}
	//计算折射光贡献的颜色
	my_draw_color
		refractionColor = { 0,0,0 };
	if (nearestModel->transparency > 0)
	{
		my_3Dvector refrdir = raydir;
		float ni_over_nt = inside ? nearestModel->transparency / 1.00029 : 1.00029 /
			nearestModel->transparency; //1.00029为空气的折射率
		bool refrected = get_refract_dir_my(raydir, nhit, ni_over_nt, refrdir); //计算折射光贡献的颜色
			if (refrected)
			{
				refrdir.normalized();
				refractionColor = one_ray_trace_my(phit, refrdir, all_models, depth + 1,
					eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular); //- nhit*bias
				refractionColor = refractionColor * 0.5; //0.5折射系数
			}
	}
	return surface_directColor + reflectionColor + refractionColor;
}
