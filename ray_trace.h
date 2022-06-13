#pragma once
#include "lineTriangle3DIntersection.h"
#include "direct_lighting.h"
#include <omp.h>
#include  <atlconv.h>
#include  <gdiplus.h>
#pragma comment(lib,  "gdiplus.lib")
#define M_PI 3.141592653589793
#define CUR_INFINITY 1e8
#define MAX_RAY_DEPTH 3
using namespace  Gdiplus;
//构建深度缓存数组
struct my_map
{
	float shadow = 0;//初始化点为黑色，即处于阴影之中，取值为0至1之间的浮点数
	float t = 999999;//用作记录采样点和模型上的点的距离，初始值为最大
	my_3D_point_coord models_crossed_point;//透过采样点的光线与模型相交的点
}my_map_point[800][800];
my_3Dvector facenormal;
my_3D_point_coord CalPlaneLineIntersectPoint(my_3Dvector planeVector,
	my_3D_point_coord planePoint, my_3Dvector lineVector, my_3D_point_coord
	linePoint) {
	my_3D_point_coord crosspoint;
	float vp1, vp2, vp3, n1, n2, n3, v1, v2, v3, m1, m2, m3, t, vpt;
	vp1 = planeVector.dx;
	vp2 = planeVector.dy;
	vp3 = planeVector.dz;
	n1 = planePoint.x;
	n2 = planePoint.y;
	n3 = planePoint.z;
	v1 = lineVector.dx;
	v2 = lineVector.dy;
	v3 = lineVector.dz;
	m1 = linePoint.x;
	m2 = linePoint.y;
	m3 = linePoint.z;
	vpt = v1 * vp1 + v2 * vp2 + v3 * vp3;
	//首先判断直线是否与平面平行
	if (vpt == 0)
	{
		return { 0,0,0 };
	}
	else
	{
		t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
		crosspoint.x = m1 + v1 * t;
		crosspoint.y = m2 + v2 * t;
		crosspoint.z = m3 + v3 * t;
	}
	return crosspoint;
}

float distance(my_3D_point_coord p1, my_3D_point_coord p2)//两点间距离
{
	float dis = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2) + pow((p1.z - p2.z), 2));
	return dis;
}

/***************************************
* 对投影面进行离散点采样
* 本函数要求与视点与世界坐标系下某条轴重合，且投影平面与某个坐标平面平行
* 上述坐标轴垂直于上述坐标平面
* left_x, right_x, bottom_y, up_y为投影面（视口）矩形四个方向的边界值
* nearplane_distance为近平面距离 eye_z视点在z轴上的值
* render_vertices采样点集合
* width, height分别记录横向和纵向采样点的个数
***************************************/
void samplepoint_sonprojectionplan(float left_x, float right_x, float bottom_y, float up_y, float
	nearplane_distance, float eye_z, std::map<my_3D_point_coord*, my_draw_color*>&
	render_vertices, unsigned& width, unsigned& height)
{
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
			my_3D_point_coord* tempPoint_ptr = new my_3D_point_coord(x_iter, y_iter, z_val);
			my_draw_color* tempColor_ptr = new my_draw_color{ 0,0,0 };
			render_vertices.insert(pair<my_3D_point_coord*, my_draw_color*>(tempPoint_ptr, tempColor_ptr));
			if (counted == false) height++;
		}
		counted = true;
	}
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
		infilename = L"desk.bmp";
		break;
	}
	case  1:
	{
		infilename = L"room.bmp";
		break;
	}
	case  2:
	{
		infilename = L"laptop.bmp";
		break;
	}
	case  3:
	{
		infilename = L"chair.bmp";
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

/***************************************
* 计算折射光线方向的函数
* inpuray_dir为入射光向量 nhit为交点法向 refracted_dir为输出的折射光线方向
* ni_over_nt为所离开物体与被进入物体之间的反射率比值 用斯奈尔定律计算
******************************************/
bool get_refract_dir_my(const my_3Dvector& inpuray_dir, my_3Dvector& nhit, float ni_over_nt,
	my_3Dvector& refracted_dir)
{
	my_3Dvector uv(inpuray_dir.dx, inpuray_dir.dy, inpuray_dir.dz);
	uv.normalized();
	float dt = uv.dot(nhit);
	double discriminant = 1.0 - double(ni_over_nt * ni_over_nt * (1 - dt * dt));
	if (discriminant > 0)
	{
		refracted_dir = (uv - nhit * dt) * ni_over_nt - nhit * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

//光线跟踪算法计算阴影、颜色值 raydir需要是一个单位向量
my_draw_color one_ray_trace_shadow_test(
	float left_x, float right_x, float bottom_y, float up_y,
	my_3D_point_coord rayorig, my_3Dvector raydir,
	const std::vector<my_triangle_3DModel>& all_models,
	const int& depth,
	const my_3D_point_coord& eye_position,
	const my_3D_point_coord& light_position,
	float light_rgb_ambient[], float light_rgb_diffuse_specular[])//用于交点直接颜色计算
{
	//超过递归层数，返回000
	if (depth > MAX_RAY_DEPTH)
		return my_draw_color{ 0,0,0 };
	//计算模型（光线的交点）——三角网格遍历与光线求交——一定要注意起点在三角面上的问题
	float nearest_t = INFINITY;   //射线参数方程中的t
	my_3Dvector nearestTrangleNormal;
	const my_triangle_3DModel* nearestModel = NULL;
	my_3D_line curRay(rayorig.x, rayorig.y, rayorig.z, raydir.dx, raydir.dy, raydir.dz);
	for (unsigned model_index = 0; model_index < all_models.size(); model_index++)
	{
#pragma omp parallel for
		for (int tri_index = 0; tri_index < all_models[model_index].faceSets.size(); tri_index++)
		{
			//从面实例中取出三个顶点 
			int firstPointIndex = all_models[model_index].faceSets[tri_index].first_point_index;//取出顶点索引
			int secondPointIndex = all_models[model_index].faceSets[tri_index].second_point_index;
			int thirdPointIndex = all_models[model_index].faceSets[tri_index].third_point_index;

			my_3D_point_coord p1 = all_models[model_index].pointSets[firstPointIndex];//第一个顶点 
			my_3D_point_coord p2 = all_models[model_index].pointSets[secondPointIndex]; //第二个顶点 
			my_3D_point_coord p3 = all_models[model_index].pointSets[thirdPointIndex]; //第三个顶点 

			my_3D_triangle curTriangle = { p1, p2, p3 };
			IntersectionBetweenLineAndTriangle newIntTest(curRay, curTriangle);

			//不仅有交点，还要求不能是出发点newIntTest.GetLineParameter() > 1e-3 
			if (newIntTest.Find() && newIntTest.GetLineParameter() > 0.002f && (newIntTest.GetLineParameter() < nearest_t))
			{
				nearestModel = &all_models[model_index];
				nearestTrangleNormal = newIntTest.GetHitPointNormal();
				nearest_t = newIntTest.GetLineParameter();
			}
		}
	}

	//如果没有交到模型——如果是首次开始递归即无交，则表示是空间背景，否则是递归过程中（说明之前已经有击中）则返回0表示对上一次递归无能量贡献

	if (!nearestModel)
		return depth == 0 ? my_draw_color{ 1,1,1 } : my_draw_color{ 0,0,0 };

	my_3Dvector added_valVec = raydir * nearest_t;
	my_3D_point_coord phit = rayorig.add(added_valVec.dx, added_valVec.dy, added_valVec.dz); // 获得交点 
	my_3Dvector  nhit = nearestTrangleNormal; // 获得交点处的法向
	nhit.normalized();

	//用Blinn-Phong Reﬂection Model计算交点颜色 ——只要有击中就算一下是否能被光源覆盖到，不管是直射、反射还是折射
	float ambient_rgb_reflection[3] = { nearestModel->material_ambient_rgb_reflection[0],nearestModel->material_ambient_rgb_reflection[1],nearestModel->material_ambient_rgb_reflection[2] };
	float diffuse_rgb_reflection[3] = { nearestModel->material_diffuse_rgb_reflection[0], nearestModel->material_diffuse_rgb_reflection[1], nearestModel->material_diffuse_rgb_reflection[2] };
	float specular_rgb_reflection[3] = { nearestModel->material_specular_rgb_reflection[0] ,nearestModel->material_specular_rgb_reflection[1] ,nearestModel->material_specular_rgb_reflection[2] };
	my_draw_color surface_directColor = calculate_direct_light_on_one_vertex_usingBPRM(phit, nhit, eye_position, light_position,
		light_rgb_ambient, ambient_rgb_reflection,
		light_rgb_diffuse_specular, diffuse_rgb_reflection,
		specular_rgb_reflection, nearestModel->ns);

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
		reflectionColor = one_ray_trace_shadow_test(left_x, right_x, bottom_y, up_y, phit, refldir, all_models, depth + 1, eye_position,
			light_position, light_rgb_ambient, light_rgb_diffuse_specular); //+ nhit*bias
		reflectionColor = reflectionColor * 0.5;//0.5 反射系数（能量减一半）
	}
	//计算折射光贡献的颜色
	my_draw_color refractionColor = { 0,0,0 };
	if (nearestModel->transparency > 0)
	{
		my_3Dvector refrdir = raydir;
		float ni_over_nt = inside ? nearestModel->transparency / 1.00029 : 1.00029 /
			nearestModel->transparency; //1.00029 为空气的折射率
		bool refrected = get_refract_dir_my(raydir, nhit, ni_over_nt, refrdir); //计算折射光贡献的颜色
		if (refrected)
		{
			refrdir.normalized();
			refractionColor = one_ray_trace_shadow_test(left_x, right_x, bottom_y, up_y, phit, refrdir, all_models, depth + 1,
				eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular); //- nhit*bias
			refractionColor = refractionColor * 0.5; //0.5折射系数 能量减一半
		}
	}

	surface_directColor = surface_directColor + reflectionColor + refractionColor;


	int i, j;
	float shadow_color = 0;
	my_3D_point_coord crosspoint;//与深度缓存面的交点
	my_3Dvector light_to_point(light_position, phit);

	//求得点与深度缓存面的交点
	crosspoint = CalPlaneLineIntersectPoint(my_3Dvector(0, -1, 0), my_3D_point_coord(-left_x, 1000, -bottom_y), light_to_point, phit);

	//判断交点在数组上的大致位置
	if (crosspoint.x<left_x || crosspoint.x>right_x || crosspoint.z<bottom_y || crosspoint.z>up_y)//在缓存面范围之外
	{
		return surface_directColor;//只给表面颜色值
	}
	else//与深度缓存面有实交点
	{
		float x_delt = (right_x - left_x) / (ceil(right_x) - floor(left_x));//横向采样间隔
		float y_delt = (up_y - bottom_y) / (ceil(up_y) - floor(bottom_y)); //纵向采样间隔
		float ii = (crosspoint.x - left_x) / x_delt;//d
		float jj = (crosspoint.z - bottom_y) / y_delt;
		int ii_up = ceil(ii);//i的上界
		int ii_down = floor(ii);//i的下界1
		int jj_up = ceil(jj);
		int jj_down = floor(jj);
		if (ii_down == ii_up && jj_down == jj_up) //刚好在my_map_point中找到相应的点
		{
			i = ii_up, j = jj;
			if (distance(phit, my_map_point[i][j].models_crossed_point) <= 1)
			{
				shadow_color = my_map_point[i][j].shadow;
			}
			else
				shadow_color = 0;
		}
		else//没有在my_map_point中找到相应的点
		{
			if (distance(phit, my_map_point[ii_up][jj_up].models_crossed_point) > 5)
				shadow_color += 0;
			else
				shadow_color += my_map_point[ii_up][jj_up].shadow / 4;
			if (distance(phit, my_map_point[ii_up][jj_down].models_crossed_point) > 5)
				shadow_color += 0;
			else
				shadow_color += my_map_point[ii_up][jj_down].shadow / 4;
			if (distance(phit, my_map_point[ii_down][jj_down].models_crossed_point) > 5)
				shadow_color += 0;
			else
				shadow_color += my_map_point[ii_down][jj_down].shadow / 4;
			if (distance(phit, my_map_point[ii_down][jj_up].models_crossed_point) > 5)
				shadow_color += 0;
			else
				shadow_color += my_map_point[ii_down][jj_up].shadow / 4;
		}
		/*surface_directColor.r = surface_directColor.r * shadow_color;
		surface_directColor.g = surface_directColor.g * shadow_color;
		surface_directColor.b = surface_directColor.b * shadow_color;
		cout << surface_directColor.r << " " << surface_directColor.g << " " << surface_directColor.b << endl;*/
	}
	return surface_directColor * shadow_color;
	//return surface_directColor;
}

//光线跟踪算法、raydir需要是一个单位向量
my_draw_color one_ray_trace_my(my_3D_point_coord rayorig, my_3Dvector raydir, const
	std::vector<my_triangle_3DModel>& all_models, const int& depth,
	const my_3D_point_coord& eye_position, const my_3D_point_coord& light_position, float
	light_rgb_ambient[], float light_rgb_diffuse_specular[])//用于交点直接颜色计算
{
	//超过递归层数，返回000
	if (depth > MAX_RAY_DEPTH)
		return my_draw_color{ 0,0,0 };
	//计算模型（光线的交点）——三角网格遍历与光线求交——一定要注意起点在三角面上的问题
	float nearest_t = INFINITY; //射线参数方程中的t
	my_3Dvector nearestTrangleNormal;
	const my_triangle_3DModel* nearestModel = NULL;
	my_3D_line curRay(rayorig.x, rayorig.y, rayorig.z, raydir.dx, raydir.dy, raydir.dz);
	for (unsigned model_index = 0; model_index < all_models.size(); model_index++)
	{
#pragma omp parallel for
		for (int tri_index = 0; tri_index < all_models[model_index].faceSets.size(); tri_index++)
		{
			//从面实例中取出三个顶点
			int firstPointIndex = all_models[model_index].faceSets[tri_index].first_point_index;//取出顶点索引
			int secondPointIndex = all_models[model_index].faceSets[tri_index].second_point_index;
			int thirdPointIndex = all_models[model_index].faceSets[tri_index].third_point_index;
			my_3D_point_coord p1 = all_models[model_index].pointSets[firstPointIndex];//第一个顶点
			my_3D_point_coord p2 = all_models[model_index].pointSets[secondPointIndex];
			//第二个顶点
			my_3D_point_coord p3 = all_models[model_index].pointSets[thirdPointIndex]; //第三个顶点
			my_3D_triangle curTriangle = { p1, p2, p3, true };
			IntersectionBetweenLineAndTriangle newIntTest(curRay, curTriangle);
			//不仅有交点，还要求不能是出发点newIntTest.GetLineParameter() > 1e-3
			if (newIntTest.Find() && newIntTest.GetLineParameter() > 0.002f &&
				(newIntTest.GetLineParameter() < nearest_t))
			{
				nearestModel = &all_models[model_index];
				nearestTrangleNormal = newIntTest.GetHitPointNormal();
				nearest_t = newIntTest.GetLineParameter();
			}
		}
	}
	if (!nearestModel)
		return depth == 0 ? my_draw_color{ 1,1,1 } : my_draw_color{ 0,0,0 };
	my_3Dvector added_valVec = raydir * nearest_t;
	my_3D_point_coord phit = rayorig.add(added_valVec.dx, added_valVec.dy,
		added_valVec.dz); // 获得交点
	my_3Dvector nhit = nearestTrangleNormal; // 获得交点处的法向
	nhit.normalized();
	//用Blinn-Phong Reflection Model计算交点颜色 ——只要有击中就算一下是否能被光源覆盖到，不管是直射、反射还是折射
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
		reflectionColor = reflectionColor * 0.5;//0.5 反射系数（能量减一半）
	}
	//计算折射光贡献的颜色
	my_draw_color refractionColor = { 0,0,0 };
	if (nearestModel->transparency > 0)
	{
		my_3Dvector refrdir = raydir;
		float ni_over_nt = inside ? nearestModel->transparency / 1.00029 : 1.00029 /
			nearestModel->transparency; //1.00029 为空气的折射率
		bool refrected = get_refract_dir_my(raydir, nhit, ni_over_nt, refrdir); //计算折射光贡献的颜色
		if (refrected)
		{
			refrdir.normalized();
			refractionColor = one_ray_trace_my(phit, refrdir, all_models, depth + 1,
				eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular); //- nhit*bias
			refractionColor = refractionColor * 0.6; //0.5折射系数 能量减一半
		}
	}
	return surface_directColor + reflectionColor+refractionColor;
}

/***************************************
* 计算阴影
* left_x, right_x, bottom_y, up_y为投影面（视口）矩形四个方向的边界值
* rayorig，raydir分别为光线发出的位置和朝向
* all_models 场景中的所有三维图形ixi
* eye_position为视点位置 light_position光源位置
* 全局变量my_map_point为一个深度缓存矩阵（二维数组），类型为my_map
* 返回当前点的阴影值
***************************************/
float one_ray_trace_shadow(float left_x, float right_x, float bottom_y, float up_y,
	my_3D_point_coord rayorig, my_3Dvector raydir,
	const vector<my_triangle_3DModel>& all_models,
	const my_3D_point_coord& light_position) {
	//计算模型（光线的交点）——三角网格遍历与光线求交——一定要注意起点在三角面上的问题
	float nearest_t = INFINITY; //射线参数方程中的t
	my_3Dvector nearestTrangleNormal;
	const my_triangle_3DModel* nearestModel = NULL;
	my_3D_line curRay(rayorig.x, rayorig.y, rayorig.z, raydir.dx, raydir.dy, raydir.dz);
	for (unsigned model_index = 0; model_index < all_models.size(); model_index++)
	{
#pragma omp parallel for
		for (int tri_index = 0; tri_index < all_models[model_index].faceSets.size();
			tri_index++)
		{
			//从面实例中取出三个顶点
			int firstPointIndex =
				all_models[model_index].faceSets[tri_index].first_point_index;
			int secondPointIndex =
				all_models[model_index].faceSets[tri_index].second_point_index;
			int thirdPointIndex =
				all_models[model_index].faceSets[tri_index].third_point_index;
			my_3D_point_coord p1 =
				all_models[model_index].pointSets[firstPointIndex];//第一个顶点
			my_3D_point_coord p2 =
				all_models[model_index].pointSets[secondPointIndex]; //第二个顶点
			my_3D_point_coord p3 =
				all_models[model_index].pointSets[thirdPointIndex]; //第三个顶点
			my_3D_triangle curTriangle = { p1, p2, p3 };
			IntersectionBetweenLineAndTriangle newIntTest(curRay, curTriangle);
			//不仅有交点，还要求不能是出发点附近距离0.002范围内的点
			if (newIntTest.Find() && newIntTest.GetLineParameter() > 0.002f &&
				(newIntTest.GetLineParameter() < nearest_t))
			{
				nearestModel = &all_models[model_index];
				nearestTrangleNormal = newIntTest.GetHitPointNormal();
				nearest_t = newIntTest.GetLineParameter();
			}
		}
	}
	//当前光线与模型无交————则表示光线击中了空间背景，无阴影，返回白色
	if (!nearestModel)
		return 1;
	my_3Dvector added_valVec = raydir * nearest_t;
	my_3D_point_coord phit = rayorig.add(added_valVec.dx, added_valVec.dy,
		added_valVec.dz); // 获得交点
	my_3Dvector nhit = nearestTrangleNormal; // 获得交点处的法向
	nhit.normalized();
	int i, j;
	float shadow_color = 0;
	my_3D_point_coord crosspoint;//与深度缓存面的交点
	my_3Dvector light_to_point(light_position, phit);
	//求得点与深度缓存面的交点
	crosspoint = CalPlaneLineIntersectPoint(my_3Dvector(0, -1, 0),
		my_3D_point_coord(-left_x, 1000, -bottom_y), light_to_point, phit);
	//判断交点在数组上的大致位置
	if (crosspoint.x<left_x || crosspoint.x>right_x || crosspoint.z<bottom_y || crosspoint.z>up_y)//在缓存面范围之外，即光源无法覆盖的区域，返回黑色
	{
		return 0;
	}
	else//与深度缓存面有实交点
	{
		float x_delt = (right_x - left_x) / (ceil(right_x) - floor(left_x));//横向采样间隔
		float y_delt = (up_y - bottom_y) / (ceil(up_y) - floor(bottom_y)); //纵向采样间隔
		float ii = (crosspoint.x - left_x) / x_delt;
		float jj = (crosspoint.z - bottom_y) / y_delt;
		int ii_up = ceil(ii);//i的上界
		int ii_down = floor(ii);//i的下界
		int jj_up = ceil(jj);
		int jj_down = floor(jj);
		//刚好在my_map_point中找到相应的点
		if (ii_down == ii_up && jj_down == jj_up)
		{
			i = ii_up, j = jj;
			if (distance(phit, my_map_point[i][j].models_crossed_point) <= 1)
			{
				shadow_color = my_map_point[i][j].shadow;
			}
			else
				shadow_color = 0;
		}
		else//找到周围阈值范围内(最多4个)的点，计算平均阴影值
		{
			int count = 0;
			if (distance(phit, my_map_point[ii_up][jj_up].models_crossed_point) < 5)
				shadow_color += my_map_point[ii_up][jj_up].shadow; count++;
			if (distance(phit, my_map_point[ii_up][jj_down].models_crossed_point) < 5)
				shadow_color += my_map_point[ii_up][jj_down].shadow; count++;
			if (distance(phit, my_map_point[ii_down][jj_down].models_crossed_point) < 5)
				shadow_color += my_map_point[ii_down][jj_down].shadow; count++;
			if (distance(phit, my_map_point[ii_down][jj_up].models_crossed_point) < 5)
				shadow_color += my_map_point[ii_down][jj_up].shadow; count++;
			shadow_color /= count;
		}
	}
	return shadow_color;
}
/***************************************
* 构建深度缓存矩阵
* 本函数要求与视点与世界坐标系下某条轴重合，且投影平面与某个坐标平面平行
* 上述坐标轴垂直于上述坐标平面
* left_x, right_x, bottom_y, up_y为投影面（视口）矩形四个方向的边界值
* light_position光源位置
* all_models 场景中的所有三维图形
* 全局变量my_map_point为一个二维的数组，类型为my_map
***************************************/
void build_shadow_map(
	float left_x, float right_x, float bottom_y, float up_y,
	my_3D_point_coord& light_position,
	const std::vector<my_triangle_3DModel>& all_models) {
	//采样面点之间的间隔或密度
	float x_delt = (right_x - left_x) / (ceil(right_x) - floor(left_x));
	float y_delt = (up_y - bottom_y) / (ceil(up_y) - floor(bottom_y));
	int i, j;//作为map的下标
	//采样横向和纵向的数量
	int max_i = ceil(abs(right_x - left_x) / x_delt);
	int max_j = ceil(abs(up_y - bottom_y) / y_delt);
	float t = 0;
	for (i = 0; i < max_i; i++)
	{
#pragma omp parallel for
		for (j = 0; j < max_j; j++)
		{
			//深度缓存面上点的坐标 
			my_3D_point_coord samplepoint(left_x + i * x_delt, 1000, bottom_y + j *
				y_delt);//采样点
			my_3D_point_coord lightposition(light_position.x, light_position.y,
				light_position.z);//光源坐标
			my_3Dvector lightvector(lightposition, samplepoint);//射线向量
			lightvector.normalized();
			my_3D_line light_line(lightposition, lightvector);//生成射线
			//遍历all_models中的每个三维图形并与light_line求交，确定离光源最近的交点；将改交点存在深度缓存矩阵my_map_point[i][j]
			for (unsigned model_index = 0; model_index < all_models.size(); model_index++)
			{
				for (unsigned int tri_index = 0; tri_index <
					all_models[model_index].faceSets.size(); tri_index++)
				{
					//从面实例中取出三个顶点
					int firstPointIndex =
						all_models[model_index].faceSets[tri_index].first_point_index;//取出顶点索引
					int secondPointIndex =
						all_models[model_index].faceSets[tri_index].second_point_index;
					int thirdPointIndex =
						all_models[model_index].faceSets[tri_index].third_point_index;
					my_3D_point_coord p1 =
						all_models[model_index].pointSets[firstPointIndex];//第一个顶点
					my_3D_point_coord p2 =
						all_models[model_index].pointSets[secondPointIndex]; //第二个顶点
					my_3D_point_coord p3 =
						all_models[model_index].pointSets[thirdPointIndex]; //第三个顶点
					my_3D_triangle curTriangle = { p1, p2, p3 };
					IntersectionBetweenLineAndTriangle getcross(light_line, curTriangle);
					if (getcross.Find())
					{
						float t = getcross.GetLineParameter();
						if (my_map_point[i][j].t >= t)
						{
							my_3Dvector added_valVec = lightvector * t;
							my_3D_point_coord phit =
								light_position.add(added_valVec.dx, added_valVec.dy, added_valVec.dz); // 获得交点
							my_map_point[i][j].t = t;
							my_map_point[i][j].shadow = 1;//这个点被点亮
							my_map_point[i][j].models_crossed_point = phit;//储存光线与采样点连线和模型相交的点
						}
					}
				}
			}
		}
	}
}

