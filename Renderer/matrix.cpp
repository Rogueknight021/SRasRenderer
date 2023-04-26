#include"matrix.h"
matrix_4f::matrix_4f(float M[4][4])
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = M[i][j];
}

void matrix_4f::show()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << m[i][j] << " ";
		std::cout << std::endl;
	}
}

void matrix_4f::transpose()
{
	float M[4][4];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			M[i][j] = this->m[j][i];
	memcpy(this->m, M, sizeof(M));
}

matrix_4f matrix_4f::operator*(const matrix_4f& m1)
{
	matrix_4f m2;
	for (int i = 0; i < 4; i++)
	{
		for (int k = 0; k < 4; k++)
		{
			float sum = 0;
			for (int j = 0; j < 4; j++)
			{
				sum += this->m[i][j] * m1.m[j][k];
			}
			m2.m[i][k] = sum;
		}
	}
	return m2;
}

void vector_4f::show()
{
	std::cout << this->x << " " << this->y << " " << this->z << " " << this->w << " " << std::endl;
}

void vector_4f::operator /=(const float& w)
{
	this->x /= w;
	this->y /= w;
	this->z /= w;
	this->w = 1;
}

void vector_4f::operator *=(const float& w)
{
	this->x *= w;
	this->y *= w;
	this->z *= w;
	this->w = 1;
}

void vector_4f::operator +=(const vector_4f& b)
{
	this->x += b.x;
	this->y += b.y;
	this->z += b.z;
	this->w += b.w;
}

void vector_3f::show()
{
	std::cout << this->x << " " << this->y << " " << this->z << " " << std::endl;
}

vector_3f vector_3f::normalized()
{
	float length = vector_length(this);
	this->x /= length;
	this->y /= length;
	this->z /= length;
	return{ this->x,this->y,this->z };
}

void vector_3f::operator =(const vector_3f& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

void vector_3f::operator +=(const vector_3f& b)
{
	this->x += b.x;
	this->y += b.y;
	this->z += b.z;
}

void vector_3f::operator -=(const vector_3f& b)
{
	this->x -= b.x;
	this->y -= b.y;
	this->z -= b.z;
}

void vector_3f::operator %=(int b)
{
	if (this->x > 360) this->x -= 360;
	else if(this->x < 0) this->x += 360;
	if (this->y > 360) this->y -= 360;
	else if (this->y < 0) this->y += 360;
	if (this->z > 360) this->z -= 360;
	else if (this->z < 0) this->z += 360;
}

void vector_3f::operator *=(const float& w)
{
	this->x *= w;
	this->y *= w;
	this->z *= w;
}

//|M|
float matrix_det(const float m[3][3])
{

	return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[1][0] * m[2][1] * m[0][2] - m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
}

float matrix_det(const matrix_4f M)
{
	float sum = 0;
	for (int i = 0; i < 4; i++)
	{
		float m[3][3];
		for (int k = 0; k < 3; k++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (j < i)
					m[k][j] = M.m[k + 1][j];
				else
					m[k][j] = M.m[k + 1][j + 1];
			}
		}
		sum += pow(-1, i) * M.m[0][i] * matrix_det(m);
	}
	return sum;
}
//M*
matrix_4f matrix_adj(matrix_4f M)
{
	float rs[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			float m[3][3];
			for (int a = 0; a < 3; a++)
			{
				for (int b = 0; b < 3; b++)
				{
					if (a < i)
					{
						if (b < j)
							m[a][b] = M.m[a][b];
						else
							m[a][b] = M.m[a][b + 1];
					}
					else
					{
						if (b < j)
							m[a][b] = M.m[a + 1][b];
						else
							m[a][b] = M.m[a + 1][b + 1];
					}
				}
			}
			rs[j][i] = pow(-1, i + j) * matrix_det(m);
		}
	}
	matrix_4f RS(rs);
	return RS;
}
//M-1
matrix_4f matrix_inverse(matrix_4f M)
{
	matrix_4f m = matrix_adj(M) / matrix_det(M);
	return m;
}

//|v|
float vector_length(vector_3f v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
float vector_length(vector_3f* v) { return std::sqrt(v->x * v->x + v->y * v->y + v->z * v->z); }
//v3 = v1 + v2
//vector_4f vector_add(vector_4f& v1, vector_4f& v2)
//{
//	vector_4f v3;
//	v3.x = v1.x + v2.x;
//	v3.y = v1.y + v2.y;
//	v3.z = v1.z + v2.z;
//	v3.w = 1;
//	return v3;
//}

//v3 = v1 - v2
//vector_4f vector_sub(vector_4f& v1, vector_4f& v2)
//{
//	vector_4f v3;
//	v3.x = v1.x - v2.x;
//	v3.y = v1.y - v2.y;
//	v3.z = v1.z - v2.z;
//	v3.w = 1;
//	return v3;
//}

//v1¡¤v2
float vector_dotproduct(vector_3f& v1, vector_3f& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

//v1 x v2
vector_3f vector_crossproduct(vector_3f& v1, vector_3f& v2)
{
	vector_3f v3;
	v3.x = v1.y * v2.z - v1.z * v2.y;
	v3.y = v1.z * v2.x - v1.x * v2.z;
	v3.z = v1.x * v2.y - v1.y * v2.x;
	return v3;
}

float vector_crossproduct(vector_2f& v1, vector_2f& v2)
{
	return v1.x * v2.y - v1.y * v2.x ;
}

//vector_3f GetIntersectionWithLineAndPlane(vector_3f& plane_normal, vector_3f& plane_point, vector_3f& line_point, vector_3f& line_direct)
//{
//	float d = vector_dotproduct(plane_point - line_point, plane_normal) / vector_dotproduct(line_direct, plane_normal);
//	return line_point + d * line_direct;
//}

//4 cord to 3 cord
vector_3f to_vector_3f(const vector_4f& v3)
{
	return { v3.x, v3.y, v3.z };
}

//3 cord to 4 cord
vector_4f to_vector_4f(const vector_3f& v3, float val)
{
	return { v3.x, v3.y, v3.z, val };
}

bool point_to_plane(const vector_3f& v, const vector_4f& p)
{
	//d=Ax+By+Cz+d;
	//d>0 the point locates at the normal direction
	//d<0
	//d=0 on the plane
	return v.x * p.x + p.y * v.y + p.z * v.z + p.w >= 0;
}

vector_3f project(vector_3f& p, vector_3f& c, vector_3f& n) 
{
	return p - vector_dotproduct(p - c, n) * n;
}
vector_2f operator +(const vector_2f& b, const vector_2f& v)
{
	return { b.x + v.x, b.y + v.y };
}

vector_3f operator +(const vector_3f& b, const vector_3f& v)
{
	return { b.x + v.x, b.y + v.y, b.z + v.z };
}

vector_4f operator +(const vector_4f& b, const vector_4f& v)
{
	return { b.x + v.x, b.y + v.y, b.z + v.z,b.w + v.w };
}

vector_3f operator -(const vector_3f& v)
{
	return { -v.x,-v.y,-v.z };
}

vector_4f operator -(const vector_4f& b, const vector_4f& v)
{
	return { b.x - v.x, b.y - v.y, b.z - v.z,b.w - v.w };
}

vector_3f operator -(const vector_3f& b, const vector_3f& v)
{
	return { b.x - v.x, b.y - v.y, b.z - v.z };
}

vector_2f operator -(const vector_2f& b, const vector_2f& v)
{
	return { b.x - v.x, b.y - v.y };
}

vector_2f operator *(const float& b, const vector_2f& v)
{
	return { b * v.x, b * v.y};
}

vector_3f operator *(const float& b, const vector_3f& v)
{
	return { b * v.x, b * v.y, b * v.z };
}

vector_3f operator *(const vector_3f& b, const vector_3f& v)
{
	return { b.x * v.x,b.y * v.y ,b.z * v.z };
}

vector_4f operator*(const float& b, const vector_4f& v)
{
	return { b * v.x, b * v.y, b * v.z,b * v.w };
}

vector_4f operator*(const matrix_4f& M, const vector_4f& v)
{
	vector_4f v1;
	float a[4];
	for (int i = 0; i < 4; i++)
		a[i] = M.m[i][0] * v.x + M.m[i][1] * v.y + M.m[i][2] * v.z + M.m[i][3] * v.w;
	v1.x = a[0];
	v1.y = a[1];
	v1.z = a[2];
	v1.w = a[3];
	return v1;
}

matrix_4f operator /(const matrix_4f &M, float v)
{
	float m[4][4];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = M.m[i][j] / v;
	matrix_4f rs(m);
	return rs;
}

vector_4f operator /(const vector_4f &b, float v)
{
	return { b.x / v, b.y / v, b.z / v ,b.w/ v};
}

vector_3f operator /(const vector_3f &b, float v)
{
	return { b.x / v, b.y / v, b.z / v };
}
