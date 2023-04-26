#pragma once
#include<cmath>
#include<iostream>
struct matrix_4f {
	float m[4][4];
	matrix_4f() {};
	matrix_4f(float M[4][4]);
	void show();
	void transpose();
	matrix_4f operator*(const matrix_4f& m1);
};
struct vector_4f {
	float x, y, z, w;
	vector_4f() {}
	vector_4f(float a, float b, float c, float d) :x(a), y(b), z(c), w(d) {}
	void show();
	void operator /=(const float& w);
	void operator *=(const float& w);
	void operator +=(const vector_4f& b);
};

struct vector_4i {
	int x, y, z, w;
	vector_4i() {}
	vector_4i(int a, int b, int c, int d) : x(a), y(b), z(c), w(d) {}
};


struct vector_3f {
	float x, y, z;
	vector_3f():x(0),y(0),z(0) {};
	vector_3f(float a, float b, float c) : x(a), y(b), z(c) {}
	vector_3f(const vector_3f& v) :x(v.x), y(v.y), z(v.z) {}
	void show();
	vector_3f normalized();
	void operator =(const vector_3f& v);
	void operator +=(const vector_3f& b);
	void operator -=(const vector_3f& b);
	void operator %=(int b);
	void operator *=(const float& w);
};

struct vector_3i {
	int x, y, z;
	vector_3i() {}
	vector_3i(int a, int b, int c) :x(a), y(b), z(c) {}
};
struct vector_2f {
	float x, y;
	vector_2f() {}
	vector_2f(float a, float b) :x(a), y(b) {}
	vector_2f(const vector_2f& v) :x(v.x), y(v.y) {}
};

//|M|
float matrix_det(const float m[3][3]);
float matrix_det(const matrix_4f M);
//M*
matrix_4f matrix_adj(matrix_4f M);
//M-1
matrix_4f matrix_inverse(matrix_4f M);

//|v|
float vector_length(vector_3f v);
float vector_length(vector_3f* v);

//v3 = v1 + v2
//vector_4f vector_add(vector_4f& v1, vector_4f& v2);
//v3 = v1 - v2
//vector_4f vector_sub(vector_4f& v1, vector_4f& v2);

//v1¡¤v2
float vector_dotproduct(vector_3f& v1, vector_3f& v2);
//v1 x v2
vector_3f vector_crossproduct(vector_3f& v1, vector_3f& v2);
float vector_crossproduct(vector_2f& v1, vector_2f& v2);

//vector_3f GetIntersectionWithLineAndPlane(vector_3f & plane_normal,vector_3f &plane_point,vector_3f &line_point,vector_3f &line_direct);

//4 cord to 3 cord
vector_3f to_vector_3f(const vector_4f& v3);
//3 cord to 4 cord
vector_4f to_vector_4f(const vector_3f& v3, float val);
//judge the point locates which side of the plane 
bool point_to_plane(const vector_3f& v, const vector_4f& p);
//calculate the projection of the point to the tangent plane
vector_3f project(vector_3f& p, vector_3f& c, vector_3f& n);

vector_4f operator +(const vector_4f& b, const vector_4f& v);
vector_3f operator +(const vector_3f& b, const vector_3f& v);
vector_2f operator +(const vector_2f& b, const vector_2f& v);
vector_4f operator -(const vector_4f& b, const vector_4f& v);
vector_3f operator -(const vector_3f& b, const vector_3f& v);
vector_3f operator -(const vector_3f& v);
vector_2f operator -(const vector_2f& b, const vector_2f& v);
vector_2f operator *(const float& b, const vector_2f& v);
vector_3f operator *(const float& b, const vector_3f& v);
vector_3f operator *(const vector_3f& b,const vector_3f& v);
vector_4f operator *(const float& b, const vector_4f& v);
vector_4f operator *(const matrix_4f& M, const vector_4f& v);
matrix_4f operator /(const matrix_4f &M, float v);
vector_4f operator /(const vector_4f &b, float v);
vector_3f operator /(const vector_3f &b, float v);
