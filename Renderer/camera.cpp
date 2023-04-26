#include"camera.h"

camera::camera(vector_3f p, vector_3f l_a,vector_3f u, float e_f, float a_r, float z_n, float z_f,bool t):
	eye_pos(p),look_at(l_a),eye_fov(e_f),up(u),
	aspect_ratio(a_r),zNear(z_n),zFar(z_f),type(t)
{
	width = 20.f;
	height = 20.f;
	if (!type)
	{
		width = e_f;
		height = a_r;
	}
	Init();
}

//camera::camera(float w, float h, float z_n,float z_f, vector_3f p, vector_3f l_a):
//	eye_pos(p), look_at(l_a), width(w),
//	height(h), zNear(z_n), zFar(z_f)
//{
//	type = 0;
//	Init();
//}

void camera::Init()
{
	set_model();
	set_view();
	set_projection();
}

void camera::Update()
{
	set_model();
	set_view();
}

vector_3f camera::GetCurPos()
{
	return to_vector_3f(Model * to_vector_4f(eye_pos, 1));
}

void camera::set_view()
{
	vector_3f cur_eye_pos = to_vector_3f(Model * to_vector_4f(eye_pos,1));
	vector_3f cur_look_at = to_vector_3f(Model * to_vector_4f(look_at, 1));
	vector_3f cur_up = to_vector_3f(Model * to_vector_4f(up, 0.f));
	//vector_3f cur_look_at = look_at;
	vector_3f eye_view = (cur_look_at - cur_eye_pos).normalized();
	//eye_view = to_vector_3f(rotation * to_vector_4f(eye_view, 0.f));
	vector_3f R = vector_crossproduct(eye_view, cur_up);
	float T[4][4] = {
	{1,0,0,-cur_eye_pos.x},
	{0,1,0,-cur_eye_pos.y},
	{0,0,1,-cur_eye_pos.z},
	{0,0,0,1} };
	matrix_4f Translate(T);
	//镜头指向负z轴
	float Ro[4][4] = {
	{R.x,R.y,R.z,0},
	{cur_up.x,cur_up.y,cur_up.z,0},
	{-eye_view.x,-eye_view.y,-eye_view.z,0},
	{0,0,0,1} };
	matrix_4f Rotate(Ro);
	View = Rotate * Translate;
}

void camera::set_projection()
{
	if (zNear > 0)
	{
		zNear = -zNear;
		zFar = -zFar;
	}
	if (type)
	{
		float m[4][4] = {
			{zNear,0,0,0},
			{0,zNear,0,0},
			{0,0,zNear + zFar,-zNear * zFar},
			{0,0,1,0}
		};
		float top = tan(DEG2RAD(eye_fov / 2) * abs(zNear));
		float bottom = -top;
		float right = top * aspect_ratio;
		float left = -right;
		float n[4][4] = {
			{2 / (right - left),0,0,0},
			{0,2 / (top - bottom),0,0},
			{0,0,2 / (zNear - zFar),0},
			{0,0,0,1}
		};
		float p[4][4] = {
			{1,0,0,-(right + left) / 2},
			{0,1,0,-(top + bottom) / 2},
			{0,0,1,-(zFar + zNear) / 2},
			{0,0,0,1}
		};
		matrix_4f M(m), N(n), P(p);
		Projection = N * P * M;
	}
	else
	{
		float top = height / 2;
		float bottom = -top;
		float right = width / 2;
		float left = -right;
		float n[4][4] = {
		{2 / (right - left),0,0,0},
		{0,2 / (top - bottom),0,0},
		{0,0,2 / (zNear - zFar),0},
		{0,0,0,1}
		};
		float p[4][4] = {
			{1,0,0,-(right + left) / 2},
			{0,1,0,-(top + bottom) / 2},
			{0,0,1,-(zFar + zNear) / 2},
			{0,0,0,1}
		};
		matrix_4f N(n), P(p);
		Projection = N * P;
	}

}