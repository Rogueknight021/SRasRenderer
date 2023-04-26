#include"object.h"
double DEG2RAD(double deg) { return deg * M_PI / 180; }

void object::set_model()
{
	double x_rad = DEG2RAD(rotat.x);
	double y_rad = DEG2RAD(rotat.y);
	double z_rad = DEG2RAD(rotat.z);
	float S[4][4] = {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{0,0,0,1}
	};
	float T[4][4] = {
		{1,0,0,trans.x},
		{0,1,0,trans.y},
		{0,0,1,trans.z},
		{0,0,0,1}
	};
	float X_R[4][4] = {
		{1,0,0,0},
		{0,cos(x_rad),-sin(x_rad),0},
		{0,sin(x_rad), cos(x_rad),0},
		{0,0,0,1}
	};
	float Y_R[4][4] = {
		{cos(y_rad),0,sin(y_rad),0},
		{0,1,0,0},
		{-sin(y_rad),0, cos(y_rad),0},
		{0,0,0,1}
	};
	float Z_R[4][4] = {
		{cos(z_rad),-sin(z_rad),0,0},
		{sin(z_rad),cos(z_rad),0,0},
		{0,0,1,0},
		{0,0,0,1}
	};
	matrix_4f scale(S);
	matrix_4f translation(T);
	matrix_4f x_rotation(X_R);
	matrix_4f y_rotation(Y_R);
	matrix_4f z_rotation(Z_R);
	matrix_4f rotation = x_rotation * y_rotation * z_rotation;

	Model = translation * rotation * scale;

}

void object::set_trans(vector_3f& trans_delta)
{
	trans += trans_delta;
	Update();
}

void object::set_rotat(vector_3f& rotat_delta)
{
	rotat += rotat_delta;
	rotat %= 360;
	Update();
}