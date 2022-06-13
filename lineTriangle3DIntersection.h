#pragma once

class my_3D_line
{
public:
	my_3D_point_coord my_orig;
	my_3Dvector my_dir;
public:
	my_3D_line() {}
	~my_3D_line() {}
	//��ֱ�߸�ֵ�Ĺ��캯��
	my_3D_line(const my_3D_point_coord& orig, const my_3Dvector& direction)
	{
		my_orig = orig;
		my_dir = direction;
	}
	//��ֱ�߸�ֵ�Ĺ��캯��
	my_3D_line(float origx, float origy, float origz, float raydirx, float raydiry, float raydirz)
	{
		my_orig.x = origx;
		my_orig.y = origy;
		my_orig.z = origz;
		my_dir.dx = raydirx;
		my_dir.dy = raydiry;
		my_dir.dz = raydirz;
	}
};
//�ռ��е�һ��������
struct my_3D_triangle
{
	my_3D_point_coord first_point;
	my_3D_point_coord second_point;
	my_3D_point_coord third_point;
	bool isFilled = false;
};
//ֱ�ߺ������εĽ���
class IntersectionBetweenLineAndTriangle
{
private:
	my_3D_line my_line;
	my_3D_triangle my_triangle;
	float mLineParameter;//ֱ����t��ֵ
	float mTriBary0, mTriBary1, mTriBary2;//���������ϲ���
	my_3Dvector mNormal; //��������ķ���
public:
	IntersectionBetweenLineAndTriangle(const my_3D_line& line, const my_3D_triangle& triangle)
	{
		my_line = line;
		my_triangle = triangle;
	}
	~IntersectionBetweenLineAndTriangle() {}
	bool Find()
	{
		//����ԭ��ƫ�������ߺͷ���
		my_3Dvector diff(my_triangle.first_point, my_line.my_orig);
		my_3Dvector edge1(my_triangle.first_point, my_triangle.second_point);
		my_3Dvector edge2(my_triangle.first_point, my_triangle.third_point);
		my_3Dvector normal = edge1.cross(edge2);
		float DdN = my_line.my_dir.dot(normal);
		float sign;
		if (DdN > 1e-6f)
		{
			sign = (float)1;
		}
		else if (DdN < -1e-6f)
		{
			sign = (float)-1;
			DdN = -DdN;
		}
		else
		{
			return false;
		}
		float DdQxE2 = sign * my_line.my_dir.dot(diff.cross(edge2));
		if (DdQxE2 >= (float)0)
		{
			float DdE1xQ = sign * my_line.my_dir.dot(edge1.cross(diff));
			if (DdE1xQ >= (float)0)
			{
				if (DdQxE2 + DdE1xQ <= DdN)
				{
					//�������������ཻ
					float QdN = -sign * diff.dot(normal);
					float inv = ((float)1) / DdN;
					mLineParameter = QdN * inv;
					mTriBary1 = DdQxE2 * inv;
					mTriBary2 = DdE1xQ * inv;
					mTriBary0 = (float)1 - mTriBary1 - mTriBary2;
					mNormal = normal;
					return true;
				}
				// else: b1+b2 > 1, �޽���
			}
			// else: b2 < 0, �޽���
		}
		// else: b1 < 0, �޽���
		return false;
	}
	//����ֱ�ߵ�tֵ
	float GetLineParameter() const
	{
		return mLineParameter;
	}
	//�������������ϵĲ���mTriBary0
	float GetTriBary0() const
	{
		return mTriBary0;
	}
	//�������������ϵĲ���mTriBary1
	float GetTriBary1() const
	{
		return mTriBary1;
	}
	//�������������ϵĲ���mTriBary2
	float GetTriBary2() const
	{
		return mTriBary2;
	}
	//��������ķ���,�ǵ�λ����
	my_3Dvector GetHitPointNormal()
	{
		return mNormal;
	}
};
