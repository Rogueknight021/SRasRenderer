#pragma once
#include"object.h"
struct cubemap :public object
{
public:
	cubemap();
	void Init();
	void Update();
	void Clear();
public:
	QImage uv;
	std::vector<vector_3f> pos;
	std::vector<vector_3i> ind;
	float size;
	std::vector<vector_2f> uv_pos;
	std::vector<vector_3i> uv_ind;
};
void load_cubemap(cubemap& cm,QString& path);