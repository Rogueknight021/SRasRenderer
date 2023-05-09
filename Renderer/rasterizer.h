#pragma once
#include"cubemap.h"
#include"light.h"
#include"model.h"
#include"triangle.h"
#include"shader.h"
#include<omp.h>
#include<functional>
#include<thread>
#include<mutex>
#include<QTime>
#include<tuple>
#include<mutex>
#define FXAA_ABSOLUTE_LUMA_THRESHOLD 0.0312
#define FXAA_RELATIVE_LUMA_THRESHOLD 0.125
#define FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT 5
enum class render_type { Dot, Line, Triangle };
enum class anti_aliasing_type{None,SSAA,MSAA,FXAA};
enum class lightcolor_type{AMBIENT,DIFFUSE,SPECULAR};
enum class multithread_type{GEO,RAS};
class rasterizer
{
public:
	explicit rasterizer(int h, int wd);
	//vertex
	void Render(model& obj, int s, int e);//main pass
	void Render(model& obj, light& l);//shadowmap
	void Render(cubemap& obj);//skybox
	void Multithread(model& obj);
	void Multithread(Triangle& t, std::vector<std::thread>& pixel_threads);
	//culling
	bool ViewFrustumCull(const vector_4f& v1, const vector_4f& v2, const vector_4f& v3);
	void ViewportCull(std::vector<Vertex>& v);//SutherlandHodgeman
	bool FaceCull(const vector_4f& v1, const vector_4f& v2, const vector_4f& v3);
	bool AllVertexInsideViewport(const std::vector<Vertex>& v);
	bool InsideViewport(const vector_3f& line, const vector_4f& v);
	float ComputeWeight2D(const vector_3f& line, const vector_4f& v1, const vector_4f& v2);
	Vertex LinearInterpolate(const Vertex& v1, const Vertex& v2, const float& weight);
	//rasterize
	void RasterizeDot(Triangle& t);
	void RasterizeWire(Triangle& t);
	void RasterizeTriangle(Triangle& t, int x_min, int x_max, int y_min, int y_max);
	void DrawLine(vector_4f& begin, vector_4f& end);//Bresenham
	bool InsideTriangle(const float& x, const float& y, const std::vector<Vertex>& v);
	std::tuple<float, float, float> ComputeBarycentric2D(float x, float y, const std::vector<Vertex>& v);
	vector_4f Screen2World(float x,float y,float& z,float& w);
	//FXAA
	void FXAA();//FXAA 3.1 quality
	float ComputeLuma(QColor& color);
	void LinearInterpolate(vector_3f& blendColor, QColor& M, QColor& A, float& weight);
	QColor get_pixel_color(int i, int j);
	//others
	void UpdateLights();
	void MoveModel(float& x, float& y);

	void set_open_viewfrustumcull();
	void set_open_viewportcull();
	void set_open_facecull();
	void set_render_type(render_type& type);
	void set_anti_aliasing_type(anti_aliasing_type& type);
	void set_camera(camera* Camera);
	void set_camera_trans(vector_3f& trans_delta);
	void set_camera_rotat(vector_3f& rotat_delta);
	void set_camera_type(bool& type);
	void set_camera_size(int& val, short& type);
	void set_light(light& l);
	void set_light_rotat(vector_3f& rotat_delta);
	void set_light_trans(vector_3f& trans_delta);
	void set_light_color(vector_3f& color, lightcolor_type& type);
	void set_pixel(int& x, int& y, vector_3f& color);
	void set_pixel_oversampling(float x, float y, vector_3f& color);
	void set_uv(QImage& UV,uv_type type);
	void set_ts_level(int& n);
	void set_fragment_shader(std::function<const vector_3f(std::vector<light>&, const fragment_shader_payload&)> f_shader);
	void set_threads_num(int n, multithread_type type);

	void clear();
	void clearUV();

public:
	int obj_idx;
	std::vector<light> lights;

	render_type r_t = render_type::Line;
	anti_aliasing_type a_t = anti_aliasing_type::None;

	uchar* frame_buf;
	std::vector<std::vector<vector_3f>> frame_buf_oversampling;

private:
	//[-1,1]->[1,50]
	const float f1 = (1 - 50) / 2.0;
	const float f2 = (50 + 1) / 2.0;
	//ViewportCull
	const std::vector<vector_3f> ViewLines = {
	vector_3f(0,1,-1),
	vector_3f(0,-1,-1),
	vector_3f(1,0,-1),
	vector_3f(-1,0,-1)
	};
	//FXAA
	int EdgeStep[5] = { 1,2,4,4,12 };
	//default color
	vector_3f dot_color = { 0,0,0 };
	vector_3f line_color = { 0,0,0 };

	int height, width;
	camera* main_camera;

	bool is_open_facecull = true;
	bool is_open_viewportcull = true;
	bool is_open_viewfrustumcull = true;

	std::vector<float> depth_buf;
	std::vector<float> depth_buf_oversampling;
	
	std::function<const vector_3f(std::vector<light>&, const fragment_shader_payload&)> fragment_shader;
	VertexShader obj_vs;
	TessellationShader ts;
	int ts_level=0;

	int uv_height = 0, uv_width = 0;
	QImage *uv=new QImage;
	QImage *uv_normal = new QImage;
	QImage *uv_specular = new QImage;

	int geo_threads_num = 1;
	int ras_threads_num = 1;
	std::vector<std::thread> vertex_threads;
	std::mutex mtx;

};