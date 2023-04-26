#pragma once
#include"object.h"
struct camera:public object
{
public:
	camera(){}
	camera(vector_3f p, vector_3f l_a, vector_3f u,float e_f, float a_r, float z_n, float z_f,bool t);
	//camera(float w, float h, float z_n, float z_f, vector_3f p, vector_3f l_a);
	void Init();
	void Update();
	vector_3f GetCurPos();
public:
	vector_3f eye_pos;
	vector_3f look_at;
	vector_3f up;// y of camera
	float eye_fov;
	float aspect_ratio;
	float zNear;
	float zFar;
	float width, height;
	bool type;//0:orthographic£¬1:perspective
	matrix_4f View, Projection;

protected:
	void set_view();
	virtual void set_projection();

};