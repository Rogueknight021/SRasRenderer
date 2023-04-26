#pragma once
#include"camera.h"
/*
* parallel light
* generating shadowmap based on the orthographic camera
*/
struct light :public camera
{
public:
	light() {};
	light(const vector_3f& pos, const vector_3f& i);
	vector_4f GetCurPos();
	//void set_projection(std::pair<vector_4f, vector_4f> AABB);

public:
	vector_3f intensity;
	vector_3f amb_light_color;
	vector_3f dif_light_color;
	vector_3f spe_light_color;

	vector_3f viewspace_pos;
	int shadowmap_w, shadowmap_h;
	std::vector<std::vector<float>> shadowmap;
};