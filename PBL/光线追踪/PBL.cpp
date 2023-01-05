#define GLUT_DISABLE_ATEXIT_HACK
#include <math.h>
#include <atlconv.h>
#include <winnt.h>
#include "ObjLoader.h"
#include "lineTriangle3DIntersection.h"
#include "ray_trace.h"
#include <GL/glut.h>
#include <omp.h>
bool shangxia = false;
int sxi = 0, sxj = 0;
int nearplane_width = 500; //�Ӿ�����
int nearplane_height = 500; //�Ӿ���߶�
int nearplane_distance = 500; //�Ӿ����ƽ�����ӵ����
int farplane_distance = nearplane_distance + 2000; //�Ӿ���Զƽ�����ӵ����
my_3D_point_coord eye_position = { 0, 0, 1000 };
//my_3D_point_coord eye_position = { 500, 500, 500 };

my_3Dvector v(eye_position.x, eye_position.y, eye_position.z);
std::vector< my_triangle_3DModel> all_models; //����������ģ��
std::map<my_3D_point_coord*, my_draw_color*> render_vertices;//������Ҫ���Ƶĵ��Լ�������

float theta = 0.1;
float radius = 500;

//��Դλ��
//my_3D_point_coord light_position(100, 1500.0, 250);
my_3D_point_coord light_position(1000, 1000.0, 1000);
float light_rgb_ambient[] = { 1.0, 1.0, 1.0 };
float light_rgb_diffuse_specular[] = { 1.0, 1.0, 1.0 };
bool open_light = false;
bool open_shadow = false;
bool rendered = false;
unsigned image_w, image_h;

//������ص���Ϣ
GLuint textName[10]; 

//��ʼ������ģ��
void init(void)
{
	//����Թ��ߵķ�����
	float material_ambient_rgb_reflection[] = { 0.2, 0.2, 0.2 };
	float material_specular_rgb_reflection[] = { 0.8, 0.8, 0.8 };
	float ns = 40; //�۹�ָ��
	//����ģ��
	//////////////////////////////////ģ�� ����/////////////////////////////////////////////////
	ObjLoader objLoader1 = ObjLoader("desk_30.obj");
	//ObjLoader objLoader1 = ObjLoader("newdesk2.obj");
	float obj1_material_diffuse_rgb_reflection[] = { 0.5, 0.7, 0.8 };
	objLoader1.my_3DModel.modify_color_configuration( -1, 1, material_ambient_rgb_reflection, obj1_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader1.my_3DModel);
	glGenTextures(1, &textName[0]);
	wchar_t file_path1[] = L"desk.bmp";
	HBITMAP hBMP1 = (HBITMAP)LoadImage(NULL, file_path1, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP;
	GetObject(hBMP1, sizeof(BMP), &BMP);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
	DeleteObject(hBMP1);
	//////////////////////////////////ģ�� ����/////////////////////////////////////////////////
	//ObjLoader objLoader2 = ObjLoader("newestroom.obj");
	ObjLoader objLoader2 = ObjLoader("room_30.obj");
	float obj2_material_diffuse_rgb_reflection[] = { 0.6, 0.4, 0.4 };
	objLoader2.my_3DModel.modify_color_configuration(-1, -1, material_ambient_rgb_reflection, obj2_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader2.my_3DModel);
	glGenTextures(1, &textName[1]);
	wchar_t file_path2[] = L"room.bmp";
	HBITMAP hBMP2 = (HBITMAP)LoadImage(NULL, file_path2, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP2;
	GetObject(hBMP2, sizeof(BMP2), &BMP2);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP2.bmWidth, BMP2.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP2.bmBits);
	DeleteObject(hBMP2);
	////////////////////////////////////ģ�� �ʼǱ�/////////////////////////////////////////////////
	ObjLoader objLoader3 = ObjLoader("laptop_30.obj");
	//ObjLoader objLoader3 = ObjLoader("newlaptop1.obj");
	float obj3_material_diffuse_rgb_reflection[] = { 0.4, 0.3, 0.1 };
	objLoader3.my_3DModel.modify_color_configuration(-1, -1, material_ambient_rgb_reflection, obj3_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader3.my_3DModel);
	glGenTextures(1, &textName[2]);
	wchar_t file_path3[] = L"laptop.bmp";
	HBITMAP hBMP3 = (HBITMAP)LoadImage(NULL, file_path3, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP3;
	GetObject(hBMP3, sizeof(BMP3), &BMP3);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP3.bmWidth, BMP3.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP3.bmBits);
	DeleteObject(hBMP3);
	//////////////////////////////////ģ�� ����/////////////////////////////////////////////////
	ObjLoader objLoader4 = ObjLoader("chair_30.obj");
	//ObjLoader objLoader4 = ObjLoader("chair.obj");
	float obj4_material_diffuse_rgb_reflection[] = { 0.8, 0.8, 0.7 };
	objLoader4.my_3DModel.modify_color_configuration(-1, -1, material_ambient_rgb_reflection, obj4_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader4.my_3DModel);
	glGenTextures(1, &textName[3]);
	wchar_t file_path4[] = L"chair.bmp";
	HBITMAP hBMP4 = (HBITMAP)LoadImage(NULL, file_path4, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP4;
	GetObject(hBMP4, sizeof(BMP4), &BMP4);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP4.bmWidth, BMP4.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP4.bmBits);
	DeleteObject(hBMP4);
	//////////////////////////////////ģ�� ball/////////////////////////////////////////////////
	ObjLoader objLoader5 = ObjLoader("newball0.obj");
	float obj5_material_diffuse_rgb_reflection[] = { 0.8, 0.9, 0.8 };
	objLoader5.my_3DModel.modify_color_configuration(1, -1, material_ambient_rgb_reflection, obj5_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader5.my_3DModel);
	glGenTextures(1, &textName[4]);
	wchar_t file_path5[] = L"ball1.bmp";
	HBITMAP hBMP5 = (HBITMAP)LoadImage(NULL, file_path5, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP5;
	GetObject(hBMP5, sizeof(BMP5), &BMP5);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP5.bmWidth, BMP5.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP5.bmBits);
	DeleteObject(hBMP5);
	//////////////////////////////////ģ�� ball/////////////////////////////////////////////////
	ObjLoader objLoader6 = ObjLoader("newball1.obj");
	float obj6_material_diffuse_rgb_reflection[] = { 0.8, 0.9, 0.8 };
	objLoader6.my_3DModel.modify_color_configuration(1, 1, material_ambient_rgb_reflection, obj6_material_diffuse_rgb_reflection, material_specular_rgb_reflection, ns);
	all_models.push_back(objLoader6.my_3DModel);
	glGenTextures(1, &textName[4]);
	wchar_t file_path6[] = L"ball.bmp";
	HBITMAP hBMP6 = (HBITMAP)LoadImage(NULL, file_path6, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);;
	BITMAP BMP6;
	GetObject(hBMP6, sizeof(BMP6), &BMP6);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textName[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP6.bmWidth, BMP6.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP6.bmBits);
	DeleteObject(hBMP6);
}

void up()
{
	//for (unsigned model_index1 = 0; model_index1 < all_models.size(); model_index1++)
	{
#pragma omp parallel for
		for (int tri_index1 = 0; tri_index1 < all_models[4].pointSets.size(); tri_index1++)
		{
			//����ʵ����ȡ����������
			all_models[4].pointSets[tri_index1].y += 10;//ȡ����������
			all_models[4].pointSets[tri_index1].y += 10;
			all_models[4].pointSets[tri_index1].y += 10;
		}
	}
}

void down()
{
	//for (unsigned model_index1 = 0; model_index1 < all_models.size(); model_index1++)
	{
#pragma omp parallel for
		for (int tri_index1 = 0; tri_index1 < all_models[4].pointSets.size(); tri_index1++)
		{
			//����ʵ����ȡ����������
			all_models[4].pointSets[tri_index1].y -= 10;//ȡ����������
			all_models[4].pointSets[tri_index1].y -= 10;
			all_models[4].pointSets[tri_index1].y -= 10;
		}
	}
}

//��������
void display(void)
{
	if (shangxia)
	{
		if (sxi < 5)
		{
			up();
			sxi++;
		}
		else
		{
			if (sxj < 5)
			{
				down();
				sxj++;
			}
			else
			{
				sxi = sxj = 0;
			}
		}
	}
	glClearColor(1.f, 1.f, 1.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!rendered)//ֱ�ӹ����
	{
		glShadeModel(GL_SMOOTH);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		for (unsigned int model_index = 0; model_index < all_models.size(); model_index++)
		{
			glBindTexture(GL_TEXTURE_2D, textName[model_index]);
//#pragma omp parallel for
			for (int i = 0; i < all_models[model_index].faceSets.size(); i++)
			{
				//����ʵ����ȡ����������
				my_triangle_indices curTriangle = all_models[model_index].faceSets[i];

				my_3D_point_coord p1 =  all_models[model_index].pointSets[curTriangle.first_point_index];//��һ������
				my_3D_point_coord p2 = all_models[model_index].pointSets[curTriangle.second_point_index]; //�ڶ�������
				my_3D_point_coord p3 = all_models[model_index].pointSets[curTriangle.third_point_index]; //����������
				my_2D_Texture_coord p1TextCoord = all_models[model_index].pointTextureSets[curTriangle.first_point_texture_index];//�� һ �� ��������
				my_2D_Texture_coord p2TextCoord = all_models[model_index].pointTextureSets[curTriangle.second_point_texture_index];//�� �� ����������
				my_2D_Texture_coord p3TextCoord = all_models[model_index].pointTextureSets[curTriangle.third_point_texture_index];//�� �� �� ��������
				my_3Dvector p1Normal = all_models[model_index].pointNormalSets[curTriangle.first_point_normal_index];//��һ�����㷨��
				my_3Dvector p2Normal = all_models[model_index].pointNormalSets[curTriangle.second_point_normal_index];//�ڶ������㷨��
				my_3Dvector p3Normal = all_models[model_index].pointNormalSets[curTriangle.third_point_normal_index];//���������㷨��

				my_draw_color p1color, p2color, p3color;
				p1color.r = p1color.g = p1color.b = 0;
				p2color.r = p2color.g = p2color.b = 0;
				p3color.r = p3color.g = p3color.b = 0;

				if (!(p1Normal.dot(v) < 0 && p2Normal.dot(v) < 0 && p3Normal.dot(v) < 0))
				{
					if (open_light)
					{
						bool LightOrShadow1, LightOrShadow2, LightOrShadow3;
						LightOrShadow1 = LightOrShadow2 = LightOrShadow3 = 1;

						if (open_shadow)
						{
							my_3Dvector lightvector1(light_position, p1);//��������
							my_3Dvector lightvector2(light_position, p2);//��������
							my_3Dvector lightvector3(light_position, p3);//��������

							lightvector1.normalized();
							lightvector2.normalized();
							lightvector3.normalized();

							my_3D_line light_line1(light_position, lightvector1);//��������
							my_3D_line light_line2(light_position, lightvector2);//��������
							my_3D_line light_line3(light_position, lightvector3);//��������

							float orit1 = lightvector1.len;
							float orit2 = lightvector2.len;
							float orit3 = lightvector3.len;

							//����all_models�е�ÿ����άͼ�β���light_line�󽻣�ȷ���Ȼ��ĵ����Դ���Ľ���
							for (unsigned model_index1 = 0; model_index1 < all_models.size(); model_index1++)
							{
								if (model_index1 == 1)
									continue;
#pragma omp parallel for
								for (int tri_index1 = 0; tri_index1 < all_models[model_index1].faceSets.size(); tri_index1++)
								{
									if (model_index1 == model_index && tri_index1 == i)
										continue;
									//����ʵ����ȡ����������
									int firstPointIndex1 = all_models[model_index1].faceSets[tri_index1].first_point_index;//ȡ����������
									int secondPointIndex1 = all_models[model_index1].faceSets[tri_index1].second_point_index;
									int thirdPointIndex1 = all_models[model_index1].faceSets[tri_index1].third_point_index;
									my_3D_point_coord p11 = all_models[model_index1].pointSets[firstPointIndex1];//��һ������
									my_3D_point_coord p21 = all_models[model_index1].pointSets[secondPointIndex1]; //�ڶ�������
									my_3D_point_coord p31 = all_models[model_index1].pointSets[thirdPointIndex1]; //����������
									my_3D_triangle curTriangle1 = { p11, p21, p31 };

									IntersectionBetweenLineAndTriangle getcross1(light_line1, curTriangle1);
									IntersectionBetweenLineAndTriangle getcross2(light_line2, curTriangle1);
									IntersectionBetweenLineAndTriangle getcross3(light_line3, curTriangle1);

									if (LightOrShadow1 && getcross1.Find())
									{
										float curt = getcross1.GetLineParameter();
										if (curt < orit1 - 0.0005)
											LightOrShadow1 = 0;
									}
									if (LightOrShadow2 && getcross2.Find())
									{
										float curt = getcross2.GetLineParameter();
										if (curt < orit2 - 0.0005)
											LightOrShadow2 = 0;
									}
									if (LightOrShadow3 && getcross3.Find())
									{
										float curt = getcross3.GetLineParameter();
										if (curt < orit3 - 0.0005)
											LightOrShadow3 = 0;
									}

									if (!LightOrShadow1 && !LightOrShadow2 && !LightOrShadow3)
										break;
								}
								if (!LightOrShadow1 && !LightOrShadow2 && !LightOrShadow3)
									break;
							}
						}

						//��Blinn-Phong Reflection Model����ÿһ�㵽��Դλ�õ�����
						if (LightOrShadow1)
							p1color = calculate_direct_light_on_one_vertex_usingBPRM(p1, p1Normal, eye_position, light_position, light_rgb_ambient, all_models[model_index].material_ambient_rgb_reflection, light_rgb_diffuse_specular, all_models[model_index].material_diffuse_rgb_reflection, all_models[model_index].material_specular_rgb_reflection, all_models[model_index].ns);
						if (LightOrShadow2)
							p2color = calculate_direct_light_on_one_vertex_usingBPRM(p2, p2Normal, eye_position, light_position, light_rgb_ambient, all_models[model_index].material_ambient_rgb_reflection, light_rgb_diffuse_specular, all_models[model_index].material_diffuse_rgb_reflection, all_models[model_index].material_specular_rgb_reflection, all_models[model_index].ns);
						if (LightOrShadow3)
							p3color = calculate_direct_light_on_one_vertex_usingBPRM(p3, p3Normal, eye_position, light_position, light_rgb_ambient, all_models[model_index].material_ambient_rgb_reflection, light_rgb_diffuse_specular, all_models[model_index].material_diffuse_rgb_reflection, all_models[model_index].material_specular_rgb_reflection, all_models[model_index].ns);
						if (!LightOrShadow1)
						{
							p1color.r *= 0.3;
							p1color.g *= 0.3;
							p1color.b *= 0.3;
						}
						if (!LightOrShadow2)
						{
							p2color.r *= 0.3;
							p2color.g *= 0.3;
							p2color.b *= 0.3;
						}
						if (!LightOrShadow3)
						{
							p3color.r *= 0.3;
							p3color.g *= 0.3;
							p3color.b *= 0.3;
						}
					}
					glBegin(GL_TRIANGLES);//��ʼ����
					glTexCoord2f(p1TextCoord.u, p1TextCoord.v);
					glColor3f(p1color.r, p1color.g, p1color.b);
					glVertex3f(p1.x, p1.y, p1.z);
					glTexCoord2f(p2TextCoord.u, p2TextCoord.v);
					glColor3f(p2color.r, p2color.g, p2color.b);
					glVertex3f(p2.x, p2.y, p2.z);
					glTexCoord2f(p3TextCoord.u, p3TextCoord.v);
					glColor3f(p3color.r, p3color.g, p3color.b);
					glVertex3f(p3.x, p3.y, p3.z);
					glEnd();
				}
			}
		}
	}
	else//���߸��ٻ���
	{
		glBegin(GL_POINTS);//��ʼ����
		for (std::map<my_3D_point_coord*, my_draw_color*>::iterator piter =
			render_vertices.begin(); piter != render_vertices.end(); piter++)
		{
			glColor3f(piter->second->r, piter->second->g, piter->second->b);
			glVertex3f(piter->first->x, piter->first->y, piter->first->z);
		}
		glEnd();
	}
	glutSwapBuffers();
}

//ͶӰ��ʽ��modelview��ʽ���á���ͶӰ�����
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	render_vertices.clear();
	if (w <= h)
	{
		//����͸���Ӿ���
		glFrustum(-0.5 * nearplane_width, 0.5 * nearplane_width, -0.5 * nearplane_height *
			(GLfloat)nearplane_height / (GLfloat)nearplane_width, 0.5 * nearplane_height *
			(GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, farplane_distance); //������ӵ�
		//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
		samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
	}
	else
	{
		//����͸���Ӿ���
		glFrustum(-0.5 * nearplane_width, 0.5 * nearplane_width, -0.5 * nearplane_height *
			(GLfloat)nearplane_width / (GLfloat)nearplane_height, 0.5 * nearplane_height *
			(GLfloat)nearplane_width / (GLfloat)nearplane_height,
			nearplane_distance, farplane_distance);
		//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
		samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_position.x, eye_position.y, eye_position.z, 0, 0, 0, 0, 1, 0);
}

//���̽����¼�
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case'1':
	{
		light_rgb_ambient[0] -= 0.05;
		light_rgb_ambient[1] -= 0.05;
		light_rgb_ambient[2] -= 0.05;
		glutPostRedisplay();
		break;
	}
	case'2':
	{
		light_rgb_ambient[0] += 0.05;
		light_rgb_ambient[1] += 0.05;
		light_rgb_ambient[2] += 0.05;
		glutPostRedisplay();
		break;
	}

	case'3':
	{
		down();
		glutPostRedisplay();
		break;
	}
	case'4':
	{
		up();
		glutPostRedisplay();
		break;
	}
	case' ':
	{
		if (sxi < 5)
		{
			up();
			sxi++;
		}
		else
		{
			if (sxj < 5)
			{
				down();
				sxj++;
			}
			else
			{
				sxi = sxj = 0;
			}
		}
		glutPostRedisplay();
		break;
	}	
	
	case 'o'://����ӹ�׷
	case 'O':
	{
		//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
		samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
		for (std::map<my_3D_point_coord*, my_draw_color*>::iterator piter =
			render_vertices.begin(); piter != render_vertices.end(); piter++)
		{
			my_3Dvector raydir(eye_position, *(piter->first));
			raydir.normalized();
			my_draw_color newColor = one_ray_trace_my(eye_position, raydir, all_models,
				0, eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular);
			cout << newColor.r << newColor.g << newColor.b << endl;
			*(piter->second) = newColor;
		}
		rendered = true;
		glutPostRedisplay();
		break;
	}
	case 'z': //��Zbuffer��Ȳ���
	case 'Z':
	{
		glEnable(GL_DEPTH_TEST); //����Ȼ������
		glDepthFunc(GL_LESS); //�ж��ڵ���ϵʱ�����ӵ���������ڵ����ӵ�Զ������
		glutPostRedisplay();
		break;
	}
	case 'c': //�ر�Zbuffer��Ȳ���
	case 'C':
	{
		glDisable(GL_DEPTH_TEST); //�ر���Ȼ������
		glutPostRedisplay();
		break;
	}
	case 'l'://�򿪵ƹ�
	case 'L':
	{
		open_light = !open_light;
		glutPostRedisplay();
		break;
	}
	case 'k'://�򿪵ƹ�
	case 'K':
	{
		open_shadow = !open_shadow;
		glutPostRedisplay();
		break;
	}
	case 'r'://���߸���
	case 'R':
	{
		rendered = !rendered;
		if (rendered)
		{
			//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
			samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
				-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
				0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
				nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
			for (std::map<my_3D_point_coord*, my_draw_color*>::iterator piter = render_vertices.begin(); piter != render_vertices.end(); piter++)
			{
				my_3Dvector raydir(eye_position, *(piter->first));
				raydir.normalized();
				my_draw_color newColor = one_ray_trace_my(eye_position, raydir, all_models, 0, eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular);
				*(piter->second) = newColor;
			}
		}
		glutPostRedisplay();
		break;
	}
	case 'f'://���߸��ٻ�����Ӱͼ
	case 'F':
	{
		//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
		samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
		build_shadow_map(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width, light_position,
			all_models);
		for (std::map<my_3D_point_coord*, my_draw_color*>::iterator piter =
			render_vertices.begin(); piter != render_vertices.end(); piter++)
		{
			my_3Dvector raydir(eye_position, *(piter->first));
			raydir.normalized();
			float ShawdowColor = one_ray_trace_shadow(-nearplane_width * 0.5,
				nearplane_width * 0.5, -nearplane_height * 0.5, nearplane_height * 0.5, eye_position, raydir,
				all_models, light_position);
			my_draw_color newColor = { ShawdowColor ,ShawdowColor ,ShawdowColor };
			*(piter->second) = newColor;
		}
		rendered = true;
		glutPostRedisplay();
		break;
	}
	case '5'://���߸��ٻ�����Ӱ
	/*case 'R':*/
	{
		//���ݵ�ǰ�Ӿ������ö�ͶӰƽ����ж������
		samplepoint_sonprojectionplan(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			nearplane_distance, eye_position.z, render_vertices, image_w, image_h);
		build_shadow_map(-0.5 * nearplane_width, 0.5 * nearplane_width,
			-0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width,
			0.5 * nearplane_height * (GLfloat)nearplane_height / (GLfloat)nearplane_width, light_position,
			all_models);
		for (std::map<my_3D_point_coord*, my_draw_color*>::iterator piter0 =
			render_vertices.begin(); piter0 != render_vertices.end(); piter0++)
		{
			my_3Dvector raydir(eye_position, *(piter0->first));
			raydir.normalized();

			my_draw_color ShawdowColor = one_ray_trace_shadow_test(-nearplane_width * 0.5,
				nearplane_width * 0.5, -nearplane_height * 0.5, nearplane_height * 0.5, eye_position, raydir,
				all_models, 0, eye_position, light_position, light_rgb_ambient, light_rgb_diffuse_specular);

			//cout << ShawdowColor.r << " " << ShawdowColor.g << " " << ShawdowColor.b << endl;

			*(piter0->second) = ShawdowColor;
		}
		rendered = true;
		glutPostRedisplay();
		break;
	}
	case 't':
	case 'T':
	{
		//�� OpenGL ֧������
		glEnable(GL_TEXTURE_2D);
		glutPostRedisplay();
		break;
	}
	case 'u':
	case 'U':
	{
		//�� OpenGL ��֧������
		glDisable(GL_TEXTURE_2D);
		glutPostRedisplay();
		break;
	}
	case'a':
	{
		eye_position.x = eye_position.x * cosf(-theta) + eye_position.z * sinf(-theta);
		eye_position.z = eye_position.z * cosf(-theta) - eye_position.x * sinf(-theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}
	case'd':
	{
		eye_position.x = eye_position.x * cosf(theta) + eye_position.z * sinf(theta);
		eye_position.z = eye_position.z * cosf(theta) - eye_position.x * sinf(theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}
	case'w':
	{
		eye_position.y = eye_position.y * cosf(theta) + eye_position.z * sinf(theta);
		eye_position.z = eye_position.z * cosf(theta) - eye_position.y * sinf(theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}
	case's':
	{
		eye_position.y = eye_position.y * cosf(-theta) + eye_position.z * sinf(-theta);
		eye_position.z = eye_position.z * cosf(-theta) - eye_position.y * sinf(-theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}
	case'q':
	{
		eye_position.x = eye_position.x * cosf(-theta) + eye_position.y * sinf(-theta);
		eye_position.y = eye_position.y * cosf(-theta) - eye_position.x * sinf(-theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}
	case'e':
	{
		eye_position.x = eye_position.x * cosf(theta) + eye_position.y * sinf(theta);
		eye_position.y = eye_position.y * cosf(theta) - eye_position.x * sinf(theta);
		v.dx = eye_position.x;
		v.dy = eye_position.y;
		v.dz = eye_position.z;
		reshape(nearplane_width, nearplane_height);
		glutPostRedisplay();
		break;
	}

	case 27:
	{
		exit(0);
		break;
	}
	}
}

//��꽻���¼�
//������ı��ӵ㷽��
void mouse(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			eye_position.x = eye_position.x * cosf(-theta) + eye_position.z * sinf(-theta);
			eye_position.z = eye_position.z * cosf(-theta) - eye_position.x * sinf(-theta);
			eye_position.y = 1;
			reshape(nearplane_width, nearplane_height);
			glutPostRedisplay();
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			eye_position.x = eye_position.x * cosf(theta) + eye_position.z * sinf(theta);
			eye_position.z = eye_position.z * cosf(theta) - eye_position.x * sinf(theta);
			eye_position.y = 1;
			reshape(nearplane_width, nearplane_height);
			glutPostRedisplay();
		}
		break;
	default:
		break;
	}
}

//��������
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(nearplane_width, nearplane_height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("PBL");
	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);
	glutMainLoop();
	return 0;
}