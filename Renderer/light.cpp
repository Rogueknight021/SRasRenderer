#include"light.h"
light::light(const vector_3f& pos, const vector_3f& i)
{
	/*
	* if change the pos of light,you need to change the up,
	* otherwise the shadowmap is wrong
	*/
	eye_pos = pos;
	intensity = i;
	up = { 0,1,0 };
	look_at = { 0,0,0 };
	zNear = 1;
	zFar = 50;
	height = 20;
	width = 20;

	amb_light_color = { 1.f,1.f,1.f };
	dif_light_color = { 1.f,1.f,1.f };
	spe_light_color = { 1.f,1.f,1.f };

	shadowmap_w = 500;
	shadowmap_h = 500;
	shadowmap.resize(shadowmap_h, std::vector<float>(shadowmap_w, std::numeric_limits<float>::infinity()));
	
	type = 0;
	Init();
}

vector_4f light::GetCurPos()
{
	return Model * to_vector_4f(eye_pos, 1);
}

//void light::set_projection(std::pair<vector_4f, vector_4f> AABB)
//{
//	AABB.first = View * AABB.first;
//	AABB.second = View * AABB.second;
//	float width = (AABB.second.x - AABB.first.x) / 2;
//	float height = (AABB.second.y - AABB.first.y) / 2;
//	zFar = AABB.first.z;
//	zNear = AABB.second.z;
//}