#pragma once
#include"matrix.h"
#include"light.h"
#include"object.h"
#include"triangle.h"
#include<vector>
#include<algorithm>
#include<QImage>
struct fragment_shader_payload
{
	vector_3f &viewspace_pos;
	vector_3f &lightspace_pos;
	vector_3f &specular_intensity;
	vector_3f &color;
	vector_3f &normal;
	explicit fragment_shader_payload(vector_3f& v_p,vector_3f& l_p,vector_3f& s,vector_3f& c,vector_3f& n);

};

class VertexShader
{
public:
	void Init(const matrix_4f& m, const matrix_4f& v, const matrix_4f& p);
	void Init(const matrix_4f& m, const matrix_4f& v, const matrix_4f& p, const light& l);

	vector_3f shadow_shader(vector_4f& v);
	void shadow_shader(std::vector<Vertex>& v);
	void no_shader(std::vector<Vertex>& v);
	void test_shader(std::vector<Vertex>& v);

	//void set_uv_normal(QImage* Q);
	void clear();
public:
	matrix_4f mvp_inverse;
	QImage* uv_normal = new QImage;
	float TBN[3][3];
private:
	matrix_4f mvp,mv,inv_trans;
	matrix_4f Model, View, Projection;
	light l_t;
};

class TessellationShader
{
public:
	void HullShader(int &dp);
	std::vector<Triangle*> DomainShader(std::vector<Vertex>& vertexes);
private:
	void TessellationPrimitiveGenerator();
	void VertexesGenerator(std::vector<vector_3f>& oldVertexes,int cur_dp);
	std::vector<vector_3f> newVertexes;
	int depth;

	float interp_factor=0.75;
	//int tessellation_factor;
	//int inner_tessellation_factor;
	//float inner_step;
};

vector_3f phong_shader(const std::vector<light>& lights, const fragment_shader_payload& payload);
float CalculateShadow(const light& l, const vector_3f& lightspace_pos);
vector_3f no_shader(const std::vector<light>& lights,const fragment_shader_payload& payload);