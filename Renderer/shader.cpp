#include"shader.h"
fragment_shader_payload::fragment_shader_payload(vector_3f& v_p, vector_3f& l_p,vector_3f& s, vector_3f& c, vector_3f& n) :
	viewspace_pos(v_p), lightspace_pos(l_p), specular_intensity(s), color(c), normal(n){}

void VertexShader::Init(const matrix_4f& m, const matrix_4f& v, const matrix_4f& p)
{
	Projection = p;
	View = v;
	Model = m;
	mvp = Projection * View * Model;
	mv = View * Model;
	inv_trans = matrix_inverse(mv);
	inv_trans.transpose();
}

void VertexShader::Init(const matrix_4f& m, const matrix_4f& v, const matrix_4f& p, const light& l)
{
	Projection = p;
	View = v;
	Model = m;
	mvp = Projection * View * Model;
	mv = View * Model;
	mvp_inverse = matrix_inverse(mvp);
	inv_trans = matrix_inverse(mv);
	inv_trans.transpose();
	l_t = l;
}

void VertexShader::shadow_shader(std::vector<Vertex>& v)
{
	for(int i=0;i<3;i++)
		v[i].normal = to_vector_3f(inv_trans * to_vector_4f(v[i].normal, 0)).normalized();
	if (hasUVnormal)
	{
		//calculate TBN
		vector_3f E1 = to_vector_3f(v[1].pos - v[0].pos);
		vector_3f E2 = to_vector_3f(v[2].pos - v[0].pos);
		vector_2f uv1 = v[1].uv_pos - v[0].uv_pos;
		vector_2f uv2 = v[2].uv_pos - v[0].uv_pos;
		vector_3f& normal = v[0].normal;

		vector_3f tangent = (uv1.y * E2 - uv2.y * E1) / (uv1.y * uv2.x - uv2.y * uv1.x);
		//vector_3f bitangent = ( uv2.x * E1 - uv1.x * E2 ) / (uv1.y * uv2.x - uv2.y * uv1.x);
		//orthogonalization
		tangent = (tangent - vector_dotproduct(tangent, normal) * normal).normalized();
		vector_3f bitangent = vector_crossproduct(normal, tangent).normalized();
		TBN[0][0] = tangent.x;
		TBN[0][1] = bitangent.x;
		TBN[0][2] = normal.x;
		TBN[1][0] = tangent.y;
		TBN[1][1] = bitangent.y;
		TBN[1][2] = normal.y;
		TBN[2][0] = tangent.z;
		TBN[2][1] = bitangent.z;
		TBN[2][2] = normal.z;
		//for (int i = 0; i < 3; i++)
		//{
		//	vector_3f E1 = to_vector_3f(v[(i + 1) % 3].pos - v[i].pos);
		//	vector_3f E2 = to_vector_3f(v[(i + 2) % 3].pos - v[i].pos);
		//	vector_2f uv1 = v[(i + 1) % 3].uv_pos - v[i].uv_pos;
		//	vector_2f uv2 = v[(i + 2) % 3].uv_pos - v[i].uv_pos;
		//	vector_3f& normal = v[i].normal;
		//	
		//	vector_3f tangent = (uv1.y * E2 - uv2.y * E1) / (uv1.y * uv2.x - uv2.y * uv1.x);
		//	//vector_3f bitangent = ( uv2.x * E1 - uv1.x * E2 ) / (uv1.y * uv2.x - uv2.y * uv1.x);
		//	//orthogonalization
		//	tangent = (tangent - vector_dotproduct(tangent, normal) * normal).normalized();
		//	vector_3f bitangent = vector_crossproduct(normal, tangent);
		//	QColor normaluv_pixel;
		//	normaluv_pixel = uv_normal->pixel(v[i].uv_pos.x, v[i].uv_pos.y);
		//	vector_3f normal_deviation = {
		//		2.f * (float)normaluv_pixel.red() / 255.f - 1.f,
		//		2.f * (float)normaluv_pixel.green() / 255.f - 1.f,
		//		2.f * (float)normaluv_pixel.blue() / 255.f - 1.f
		//	};
		//	//TBN->WORLD
		//	normal_deviation.x = normal_deviation.x * tangent.x + normal_deviation.x * bitangent.x + normal_deviation.x * normal.x;
		//	normal_deviation.y = normal_deviation.y * tangent.y + normal_deviation.y * bitangent.y + normal_deviation.y * normal.y;
		//	normal_deviation.z = normal_deviation.z * tangent.z + normal_deviation.z * bitangent.z + normal_deviation.z * normal.z;
		//	normal += normal_deviation;
		//}
	}
	for (int i = 0; i < 3; i++)
	{
		v[i].viewspace_pos = to_vector_3f(mv * v[i].pos);
		v[i].pos = mvp * v[i].pos;
	}
}

vector_3f VertexShader::shadow_shader(vector_4f& v)
{
	vector_4f tmp = l_t.Projection * l_t.View * Model * v;
	tmp.x = 0.5 * l_t.shadowmap_h * (tmp.x + 1);
	tmp.y = 0.5 * l_t.shadowmap_h * (tmp.y + 1);
	tmp.z = tmp.z * -127.5 + 127.5;
	return to_vector_3f(tmp);
}

void VertexShader::no_shader(std::vector<Vertex>& v)
{
	for (int i = 0; i < 3; i++)
	{
		v[i].viewspace_pos = to_vector_3f(mv * v[i].pos);
		v[i].pos = mvp * v[i].pos;
		v[i].normal = to_vector_3f(inv_trans * to_vector_4f(v[i].normal, 0)).normalized();
	}
}

void VertexShader::test_shader(std::vector<Vertex>& v)
{
	for (int i = 0; i < 3; i++)
	{
		v[i].pos = mvp * v[i].pos;
		v[i].pos /= v[i].pos.w;
	}
}

//void VertexShader::set_uv_normal(QImage* Q)
//{
//	uv_normal = Q;
//}

void VertexShader::clear()
{
	hasUVnormal = false;
}

void TessellationShader::HullShader(int& dp)
{
	newVertexes.clear();
	depth = dp;
	TessellationPrimitiveGenerator();
	//tessellation_factor = t_f;
	//inner_tessellation_factor = i_t_f;
	//inner_step = 1.f / (tessellation_factor + 1);
}

void TessellationShader::TessellationPrimitiveGenerator()
{
	//only triangle
	int cur_dp = 0;
	std::vector<vector_3f> oldVertexes(3);
	std::vector<vector_3f> edgeVertexes(3);
	oldVertexes[0] = { 0,1,0 };
	oldVertexes[1] = { 0,0,1 };
	oldVertexes[2] = { 1,0,0 };
	for (int i = 0; i < 3; i++)
		edgeVertexes[i] = 0.5 * oldVertexes[i] + 0.5 * oldVertexes[(i + 1) % 3];
	VertexesGenerator(edgeVertexes,cur_dp+1);
	for (int i = 0; i < 3; i++)
	{
		std::vector<vector_3f> tmp(3);
		tmp[0] = oldVertexes[i];
		tmp[1] = edgeVertexes[i];
		tmp[2] = edgeVertexes[(i + 2) % 3];
		VertexesGenerator(tmp,cur_dp+1);
	}

}

void TessellationShader::VertexesGenerator(std::vector<vector_3f>& oldVertexes,int cur_dp)
{
	if (cur_dp == depth)
	{
		newVertexes.insert(newVertexes.end(), oldVertexes.begin(), oldVertexes.end());
		return;
	}
	std::vector<vector_3f> edgeVertexes(3);
	for (int i = 0; i < 3; i++)
		edgeVertexes[i] = 0.5 * oldVertexes[i] + 0.5 * oldVertexes[(i + 1) % 3];
	VertexesGenerator(edgeVertexes, cur_dp + 1);
	for (int i = 0; i < 3; i++)
	{
		std::vector<vector_3f> tmp(3);
		tmp[0] = oldVertexes[i];
		tmp[1] = edgeVertexes[i];
		tmp[2] = edgeVertexes[(i + 2) % 3];
		VertexesGenerator(tmp, cur_dp + 1);
	}
}
std::vector<Triangle*> TessellationShader::DomainShader(std::vector<Vertex>& vertexes)
{
	//Phong Tessellation
	std::vector<Triangle*> rs;
	for (int i = 0; i < newVertexes.size(); i+=3)
	{
		std::vector<Vertex> tmp(3);
		for (int j = 0; j < 3; j++)
		{
			Vertex v;
			vector_3f& uvw = newVertexes[i + j];
			vector_4f pos =
				uvw.x * vertexes[0].pos +
				uvw.y * vertexes[1].pos +
				uvw.z * vertexes[2].pos;
			vector_3f normal =
				uvw.x * vertexes[0].normal +
				uvw.y * vertexes[1].normal +
				uvw.z * vertexes[2].normal;
			vector_2f uv_pos =
				uvw.x * vertexes[0].uv_pos +
				uvw.y * vertexes[1].uv_pos +
				uvw.z * vertexes[2].uv_pos;
			v.normal = normal;
			v.uv_pos = uv_pos;
			vector_3f c0 = project(to_vector_3f(pos), to_vector_3f(vertexes[0].pos), vertexes[0].normal);
			vector_3f c1 = project(to_vector_3f(pos), to_vector_3f(vertexes[1].pos), vertexes[1].normal);
			vector_3f c2 = project(to_vector_3f(pos), to_vector_3f(vertexes[2].pos), vertexes[2].normal);
			vector_4f p(
				(1 - interp_factor) * pos.x + interp_factor * (uvw.x * c0.x + uvw.y * c1.x + uvw.z * c2.x),
				(1 - interp_factor) * pos.y + interp_factor * (uvw.x * c0.y + uvw.y * c1.y + uvw.z * c2.y),
				(1 - interp_factor) * pos.z + interp_factor * (uvw.x * c0.z + uvw.y * c1.z + uvw.z * c2.z),
				pos.w
			);
			v.pos = p;
			tmp[j]=v;
		}
		rs.push_back({ new Triangle(tmp) });
	}
	return rs;
}

vector_3f phong_shader(const std::vector<light>& lights, const fragment_shader_payload& payload)
{
	vector_3f &normal=payload.normal.normalized();
	vector_3f &pos=payload.viewspace_pos;
	//Init
	vector_3f ka(0.005, 0.005, 0.005);
	vector_3f kd(payload.color / 255);
	//vector_3f ks(0.7937, 0.7937, 0.7937);
	vector_3f &ks = payload.specular_intensity;
	float p = 150;
	vector_3f amb_light_intensity(10, 10, 10);
	vector_3f eye_pos(0, 0, 0);
	vector_3f result_color(0, 0, 0);
	vector_3f &view_dir = (eye_pos - pos).normalized();

	float shadow= CalculateShadow(lights[0],payload.lightspace_pos);
	//float shadow = 1.f;
	for (auto& light : lights)
	{
		float r2 = std::pow(vector_length(light.viewspace_pos - pos), 2);
		vector_3f light_dir = (light.viewspace_pos - pos).normalized();
		vector_3f half = (light_dir + view_dir).normalized();

		vector_3f intensity = light.intensity / r2;
		float cos_nl = vector_dotproduct(normal, light_dir);
		float cos_nh = vector_dotproduct(normal, half);
		vector_3f diffuse(
			kd.x * intensity.x * std::max(0.f, cos_nl),
			kd.y * intensity.y * std::max(0.f, cos_nl),
			kd.z * intensity.z * std::max(0.f, cos_nl));
		vector_3f specular(
			ks.x * intensity.x * std::pow(std::max(0.f, cos_nh), p),
			ks.y * intensity.y * std::pow(std::max(0.f, cos_nh), p),
			ks.z * intensity.z * std::pow(std::max(0.f, cos_nh), p)
		);
		vector_3f ambient(
			ka.x * amb_light_intensity.x,
			ka.y * amb_light_intensity.y,
			ka.z * amb_light_intensity.z);
		result_color += shadow * diffuse * light.dif_light_color +
						shadow * specular * light.spe_light_color +
						ambient* light.amb_light_color;
	}
	return 255.f * result_color;
}

float CalculateShadow(const light& l, const vector_3f& lightspace_pos)
{
	//the viewfrustum of light is fixed
	if (lightspace_pos.y >= 500.f || lightspace_pos.y < 0.f || lightspace_pos.x >= 500.f || lightspace_pos.x < 0.f)
		return 1;
	return (lightspace_pos.z - l.shadowmap[lightspace_pos.y][lightspace_pos.x] > 0.5) ? 0.0 : 1.0;
}

vector_3f no_shader(const std::vector<light>& lights, const fragment_shader_payload& payload)
{
	return payload.color;
}