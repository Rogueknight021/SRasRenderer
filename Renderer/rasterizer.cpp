#include"rasterizer.h"
rasterizer::rasterizer(int h, int wd) :height(h), width(wd)
{
	depth_buf.resize(height * width);
	std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
	frame_buf = new uchar[width * height * 4];
	main_camera = new camera();
}

void rasterizer::Render(model& obj, int s, int e)
{
	std::vector<std::thread> pixel_threads(ras_threads_num);
	obj_vs.Init(obj.Model, main_camera->View, main_camera->Projection, lights[0]);
	if (!uv_normal->isNull() && obj_vs.uv_normal->isNull())
		obj_vs.uv_normal = uv_normal;
	float& zN = main_camera->zNear;
	for (int j = s; j < e; j++)
	{
		std::vector<Vertex> tmp_vertexes(3);
		tmp_vertexes[0] = *obj.vs[obj.ind[j].x - 1];
		tmp_vertexes[1] = *obj.vs[obj.ind[j].y - 1];
		tmp_vertexes[2] = *obj.vs[obj.ind[j].z - 1];
		if (obj.uv_height!=0)
		{
			tmp_vertexes[0].uv_pos = { obj.uv_pos[obj.uv_ind[j].x - 1].x * uv_width,obj.uv_pos[obj.uv_ind[j].x - 1].y * uv_height };
			tmp_vertexes[1].uv_pos = { obj.uv_pos[obj.uv_ind[j].y - 1].x * uv_width,obj.uv_pos[obj.uv_ind[j].y - 1].y * uv_height };
			tmp_vertexes[2].uv_pos = { obj.uv_pos[obj.uv_ind[j].z - 1].x * uv_width,obj.uv_pos[obj.uv_ind[j].z - 1].y * uv_height };
		}
		if (!obj.normal.empty())
		{
			tmp_vertexes[0].normal = obj.normal[obj.normal_ind[j].x - 1];
			tmp_vertexes[1].normal = obj.normal[obj.normal_ind[j].y - 1];
			tmp_vertexes[2].normal = obj.normal[obj.normal_ind[j].z - 1];
		}
		std::vector<Triangle*> Triangles;
		(ts_level == 0) ? Triangles.push_back({ new Triangle(tmp_vertexes) }) : Triangles = ts.DomainShader(tmp_vertexes);
		/*
		* you can use code below for the loop subdivision
		*/
		//if (obj.subdivision_level == 0)
		//	Triangles.push_back( {new Triangle(tmp_vertexes)});
		//else
		//	Triangles= obj.LoopSubdivision(tmp_vertexes);
		for (auto& Tr : Triangles)
		{
			mtx.lock();
			obj_vs.shadow_shader(Tr->vertex);
			float TBN[3][3];
			if (!uv_normal->isNull())
				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						TBN[i][j] = obj_vs.TBN[i][j];
			mtx.unlock();
			bool isCulling = false;
			if (main_camera->type)
			{
				for (int i = 0; i < 3; i++)
				{
					if (Tr->vertex[i].pos.w > zN)
					{
						isCulling = true;
						break;
					}
					Tr->vertex[i].pos.x /= Tr->vertex[i].pos.w;
					Tr->vertex[i].pos.y /= Tr->vertex[i].pos.w;
					Tr->vertex[i].pos.z /= Tr->vertex[i].pos.w;
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					if (Tr->vertex[i].pos.z < -1.f || Tr->vertex[i].pos.z>1.f)
					{
						isCulling = true;
						break;
					}
				}
			}
			//viewfrustum culling
			if (is_open_viewfrustumcull && (isCulling||!ViewFrustumCull(Tr->vertex[0].pos, Tr->vertex[1].pos, Tr->vertex[2].pos)))
				continue;
			//viewport culling
			if (is_open_viewportcull && !AllVertexInsideViewport(Tr->vertex))
				ViewportCull(Tr->vertex);
			//ndc->screen
			for (int i = 0; i < Tr->vertex.size(); i++)
			{
				Tr->vertex[i].pos.x = 0.5 * width * (Tr->vertex[i].pos.x + 1);
				Tr->vertex[i].pos.y = 0.5 * height * (Tr->vertex[i].pos.y + 1);
				Tr->vertex[i].pos.z = Tr->vertex[i].pos.z * f1 + f2;//[-1,1]->[1,50]
			}
			for (int i = 0; i < (int)Tr->vertex.size() - 2; i++)
			{
				//face culling
				if (is_open_facecull && !FaceCull(Tr->vertex[0].pos, Tr->vertex[i + 1].pos, Tr->vertex[i + 2].pos))
					continue;
				Triangle t;
				t.set_vertexs(Tr->vertex[0], Tr->vertex[i + 1], Tr->vertex[i + 2]);
				if (!uv_normal->isNull())
					memcpy(t.TBN, TBN, sizeof(TBN));
				switch (r_t)
				{
				case render_type::Dot:
					RasterizeDot(t);
					break;
				case render_type::Line:
					RasterizeWire(t);
					break;
				case render_type::Triangle:
					//RasterizeTriangle(t);
					Multithread(t, pixel_threads);
					break;
				default:
					break;
				}
			}
		}
		//release resource
		Release(Triangles);
	}
	if (r_t == render_type::Triangle && (a_t == anti_aliasing_type::SSAA || a_t == anti_aliasing_type::MSAA))
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				vector_3f a = frame_buf_oversampling[i * 2][j * 2];
				vector_3f b = frame_buf_oversampling[i * 2 + 1][j * 2];
				vector_3f c = frame_buf_oversampling[i * 2][j * 2 + 1];
				vector_3f d = frame_buf_oversampling[i * 2 + 1][j * 2 + 1];
				int red = (int)((a.x + b.x + c.x + d.x) / 4);
				int green = (int)((a.y + b.y + c.y + d.y) / 4);
				int blue = (int)((a.z + b.z + c.z + d.z) / 4);
				frame_buf[(i * width + j) * 4] = red;
				frame_buf[(i * width + j) * 4 + 1] = green;
				frame_buf[(i * width + j) * 4 + 2] = blue;
				frame_buf[(i * width + j) * 4 + 3] = 255;
			}
		}
	}
}

void rasterizer::Render(model& obj, light& l)
{
	VertexShader shadow_vs;
	shadow_vs.Init(obj.Model, l.View, l.Projection);
	float ff1 = (0 - 255) / 2.0;
	float ff2 = (255 + 0) / 2.0;
	for (int j = 0; j < obj.ind.size(); j++)
	{
		Triangle t;
		std::vector<Vertex> vertexes(3);
		vertexes[0] = *obj.vs[obj.ind[j].x - 1];
		vertexes[1] = *obj.vs[obj.ind[j].y - 1];
		vertexes[2] = *obj.vs[obj.ind[j].z - 1];
		shadow_vs.no_shader(vertexes);
		if (l.type)
		{
			for (int i = 0; i < 3; i++)
			{
				vertexes[i].pos.x /= vertexes[i].pos.w;
				vertexes[i].pos.y /= vertexes[i].pos.w;
				vertexes[i].pos.z /= vertexes[i].pos.w;
			}
		}
		bool isCulling = false;
		for (int i = 0; i < 3; i++)
		{
			if (vertexes[i].pos.z < -1.f)
			{
				isCulling = true;
				break;
			}
		}
		//viewfrustum culling
		if (is_open_viewfrustumcull && (isCulling||!ViewFrustumCull(vertexes[0].pos, vertexes[1].pos, vertexes[2].pos)))
			continue;
		//viewport culling
		if (is_open_viewportcull && !AllVertexInsideViewport(vertexes))
			ViewportCull(vertexes);
		//ndc->screen
		for (int i = 0; i < vertexes.size(); i++)
		{
			vertexes[i].pos.x = 0.5 * l.shadowmap_w * (vertexes[i].pos.x + 1);
			vertexes[i].pos.y = 0.5 * l.shadowmap_h * (vertexes[i].pos.y + 1);
			vertexes[i].pos.z = vertexes[i].pos.z * ff1 + ff2;
		}
		for (int i = 0; i < (int)vertexes.size() - 2; i++)
		{
			//face culling
			if (is_open_facecull && !FaceCull(vertexes[0].pos, vertexes[i + 1].pos, vertexes[i + 2].pos))
				continue;
			t.set_vertexs(vertexes[0], vertexes[i + 1], vertexes[i + 2]);
			float x_max, x_min, y_max, y_min;
			x_max = std::max(std::max(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
			x_min = std::min(std::min(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
			y_max = std::max(std::max(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
			y_min = std::min(std::min(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
			for (int x = (int)x_min; x < x_max; x++)
			{
				for (int y = (int)y_min; y < y_max; y++)
				{
					if (!InsideTriangle(x, y, t.vertex)) continue;
					//Barycentric Interpolation
					auto [alpha, beta, gamma] = ComputeBarycentric2D(x, y, t.vertex);
					//Perspective Correct Interpolation
					float z_interpolated = alpha * t.vertex[0].pos.z + beta * t.vertex[1].pos.z + gamma * t.vertex[2].pos.z;
					//zbuff test
					if (l.shadowmap[y][x] <= z_interpolated) continue;
					l.shadowmap[y][x] = z_interpolated;
				}
			}
		}
	}
}

void rasterizer::Render(cubemap& obj)
{
	bool camera_type = main_camera->type;
	if (main_camera->type)
	{
		main_camera->type = 0;
		set_camera_type(main_camera->type);
	}
	obj_vs.Init(obj.Model, main_camera->View, main_camera->Projection);

	for (int j = 0; j < obj.ind.size(); j++)
	{
		Triangle t;
		std::vector<Vertex> vertexes(3);
		vertexes[0].pos = to_vector_4f(obj.pos[obj.ind[j].x - 1], 1);
		vertexes[1].pos = to_vector_4f(obj.pos[obj.ind[j].y - 1], 1);
		vertexes[2].pos = to_vector_4f(obj.pos[obj.ind[j].z - 1], 1);
		if (!obj.uv.isNull())
		{
			vertexes[0].uv_pos = { obj.uv_pos[obj.uv_ind[j].x - 1].x,obj.uv_pos[obj.uv_ind[j].x - 1].y };
			vertexes[1].uv_pos = { obj.uv_pos[obj.uv_ind[j].y - 1].x,obj.uv_pos[obj.uv_ind[j].y - 1].y };
			vertexes[2].uv_pos = { obj.uv_pos[obj.uv_ind[j].z - 1].x,obj.uv_pos[obj.uv_ind[j].z - 1].y };
		}
		obj_vs.no_shader(vertexes);
		for (int i = 0; i < 3; i++)
		{
			vertexes[i].pos.x /= 50;
			vertexes[i].pos.y /= 50;
			//vertexes[i].pos.z /= vertexes[i].pos.w;
			vertexes[i].pos.z = -0.99;
		}
		//viewfrustum culling
		if (!ViewFrustumCull(vertexes[0].pos, vertexes[1].pos, vertexes[2].pos))
			continue;
		//viewport culling
		if (!AllVertexInsideViewport(vertexes))
			ViewportCull(vertexes);
		//ndc->screen
		for (int i = 0; i < vertexes.size(); i++)
		{
			vertexes[i].pos.x = 0.5 * width * (vertexes[i].pos.x + 1);
			vertexes[i].pos.y = 0.5 * height * (vertexes[i].pos.y + 1);
			vertexes[i].pos.z = vertexes[i].pos.z * f1 + f2;//[-1,1]->[1,50]
		}
		for (int i = 0; i < (int)vertexes.size() - 2; i++)
		{

			//face culling
			if (is_open_facecull && !FaceCull(vertexes[0].pos, vertexes[i + 1].pos, vertexes[i + 2].pos))
				continue;
			t.set_vertexs(vertexes[0], vertexes[i + 1], vertexes[i + 2]);

			//float x_max, x_min, y_max, y_min;
			int x_max, x_min, y_max, y_min;
			x_max = std::max(std::max(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
			x_min = std::min(std::min(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
			y_max = std::max(std::max(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
			y_min = std::min(std::min(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
			x_max = std::min(x_max, width - 1);
			y_max = std::min(y_max, height - 1);
			x_min = std::max(x_min, 0);
			y_min = std::max(y_min, 0);

			for (int x = x_min; x <= x_max; x++)
			{
				for (int y = y_min; y <= y_max; y++)
				{
					if (!InsideTriangle(x, y, t.vertex)) continue;
					//Barycentric Interpolation
					auto [alpha, beta, gamma] = ComputeBarycentric2D(x, y, t.vertex);
					//Perspective Correct Interpolation
					float Z = 1.0 / (alpha / t.vertex[0].pos.w + beta / t.vertex[1].pos.w + gamma / t.vertex[2].pos.w);
					float z_interpolated = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
					z_interpolated *= Z;
					//zbuff test
					if (depth_buf[y * width + x] != std::numeric_limits<float>::infinity()) continue;
					depth_buf[y * width + x] = z_interpolated;

					QColor pixel;
					if (uv->isNull())
						pixel.setRgb(128, 128, 128);
					else
					{
						float interpolated_uv_x = alpha * t.vertex[0].uv_pos.x / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.x / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.x / t.vertex[2].pos.w;
						float interpolated_uv_y = alpha * t.vertex[0].uv_pos.y / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.y / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.y / t.vertex[2].pos.w;
						interpolated_uv_x *= Z;
						interpolated_uv_y *= Z;

						pixel = uv->pixel(interpolated_uv_x, interpolated_uv_y);
					}
					vector_3f interpolated_color(
						(int)pixel.red(),
						(int)pixel.green(),
						(int)pixel.blue()
					);
					set_pixel(x, y, interpolated_color);
				}
			}
		}
	}
	set_camera_type(camera_type);
}

void rasterizer::Multithread(model& obj)
{
	int num = obj.ind.size() / geo_threads_num;
	int remainder = obj.ind.size() - num * geo_threads_num;
	for (int i = 0; i < geo_threads_num; i++)
	{
		if (i == geo_threads_num - 1 && remainder != 0)
		{
			int s = i * num;
			int e = obj.ind.size();
			vertex_threads[i] = std::thread(static_cast<void(rasterizer::*)(model&, int, int)>(&rasterizer::Render), this, std::ref(obj), s, e);
		}
		else
		{
			int s = i * num;
			int e = (i + 1) * num;
			vertex_threads[i] = std::thread(static_cast<void(rasterizer::*)(model&, int, int)>(&rasterizer::Render), this, std::ref(obj), s, e);
		}
	}
	for (auto& t : vertex_threads)
		t.join();
}

void rasterizer::Multithread(Triangle& t, std::vector<std::thread>& pixel_threads)
{
	//get AABB
	int x_max, x_min, y_max, y_min;
	x_max = std::max(std::max(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
	x_min = std::min(std::min(t.vertex[0].pos.x, t.vertex[1].pos.x), t.vertex[2].pos.x);
	y_max = std::max(std::max(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
	y_min = std::min(std::min(t.vertex[0].pos.y, t.vertex[1].pos.y), t.vertex[2].pos.y);
	x_max = std::min(x_max, width - 1);
	y_max = std::min(y_max, height - 1);
	x_min = std::max(x_min, 0);
	y_min = std::max(y_min, 0);

	if (x_max - x_min < 50)
		RasterizeTriangle(t, x_min, x_max, y_min, y_max);
	else
	{
		int num = (x_max - x_min) / ras_threads_num;
		for (int i = 0; i < ras_threads_num; i++)
		{
			if (i == ras_threads_num - 1)
			{
				int s = i * num + x_min;
				pixel_threads[i] = std::thread(&rasterizer::RasterizeTriangle, this, std::ref(t), s, x_max, y_min, y_max);
			}
			else
			{
				int s = i * num + x_min;
				int e = (i + 1) * num + x_min;
				pixel_threads[i] = std::thread(&rasterizer::RasterizeTriangle, this, std::ref(t), s, e, y_min, y_max);
			}
		}
		for (auto& t : pixel_threads)
			t.join();
	}
}

bool rasterizer::ViewFrustumCull(const vector_4f& v1, const vector_4f& v2, const vector_4f& v3)
{
	//get AABB
	vector_3f minPoint, maxPoint;
	minPoint.x = std::min(v1.x, std::min(v2.x, v3.x));
	minPoint.y = std::min(v1.y, std::min(v2.y, v3.y));
	//minPoint.z = std::min(v1.z, std::min(v2.z, v3.z));
	maxPoint.x = std::max(v1.x, std::max(v2.x, v3.x));
	maxPoint.y = std::max(v1.y, std::max(v2.y, v3.y));
	//maxPoint.z = std::max(v1.z, std::max(v2.z, v3.z));
	//zNear and zFar only retain the object in the frustum completely 
	//if (minPoint.z >= 1 || maxPoint.z >= 1)
	//	return false;
	//if (minPoint.z <= -1 || maxPoint.z <= -1)
	//	return false;
	if (minPoint.x >= 1 && maxPoint.x >= 1)
		return false;
	if (minPoint.x <= -1 && maxPoint.x <= -1)
		return false;
	if (minPoint.y >= 1 && maxPoint.y >= 1)
		return false;
	if (minPoint.y <= -1 && maxPoint.y <= -1)
		return false;
	return true;
}

void rasterizer::ViewportCull(std::vector<Vertex>& v)
{
	//SutherlandHodgeman
	std::vector<Vertex> output = { v[0],v[1],v[2] };
	for (int i = 0; i < 4; i++)
	{
		std::vector<Vertex> input = output;
		output.clear();
		int size = input.size();
		for (int j = 0; j < input.size(); j++)
		{
			Vertex& p = input[j];
			Vertex& s = input[(size - 1 + j) % size];
			if (InsideViewport(ViewLines[i], p.pos))
			{
				if (!InsideViewport(ViewLines[i], s.pos))
				{
					//s is not in clip plane,p is
					Vertex inter_vertex = LinearInterpolate(s, p, ComputeWeight2D(ViewLines[i], s.pos, p.pos));
					output.emplace_back(inter_vertex);
				}
				//both s and p in clip plane
				output.emplace_back(p);
			}
			else if (InsideViewport(ViewLines[i], s.pos))
			{
				//p is not in clip plane,s is
				Vertex inter_vertex = LinearInterpolate(s, p, ComputeWeight2D(ViewLines[i], s.pos, p.pos));
				output.emplace_back(inter_vertex);
			}
		}
	}
	v = output;
}

bool rasterizer::AllVertexInsideViewport(const std::vector<Vertex>& v)
{
	//judge whether the triangle is in the viewport completely 
	if (v[0].pos.x > 1 || v[0].pos.x < -1)
		return false;
	if (v[0].pos.y > 1 || v[0].pos.y < -1)
		return false;
	if (v[1].pos.x > 1 || v[1].pos.x < -1)
		return false;
	if (v[1].pos.y > 1 || v[1].pos.y < -1)
		return false;
	if (v[2].pos.x > 1 || v[2].pos.x < -1)
		return false;
	if (v[2].pos.y > 1 || v[2].pos.y < -1)
		return false;
	return true;
}

bool rasterizer::InsideViewport(const vector_3f& line, const vector_4f& v)
{
	return line.x * v.x + line.y * v.y + line.z < 0;
}

float rasterizer::ComputeWeight2D(const vector_3f& line, const vector_4f& v1, const vector_4f& v2)
{
	float weight = 0;
	if (line.x == 1)
		weight = (1 - v1.x) / (v2.x - v1.x);
	else if (line.x == -1)
		weight = (-1 - v1.x) / (v2.x - v1.x);
	else if (line.y == 1)
		weight = (1 - v1.y) / (v2.y - v1.y);
	else if (line.y == -1)
		weight = (-1 - v1.y) / (v2.y - v1.y);
	return weight;
}

Vertex rasterizer::LinearInterpolate(const Vertex& v1, const Vertex& v2, const float& weight)
{
	Vertex v;
	v.pos = (1 - weight) * v1.pos + weight * v2.pos;
	v.normal = (1 - weight) * v1.normal + weight * v2.normal;
	//v.color = (1 - weight) * v1.color + weight * v2.color;
	v.uv_pos = (1 - weight) * v1.uv_pos + weight * v2.uv_pos;
	v.viewspace_pos = (1 - weight) * v1.viewspace_pos + weight * v2.viewspace_pos;
	return v;
}

bool rasterizer::FaceCull(const vector_4f& v1, const vector_4f& v2, const vector_4f& v3)
{
	vector_3f tmp1 = { v2.x - v1.x,v2.y - v1.y,v2.z - v1.z };
	vector_3f tmp2 = { v3.x - v1.x,v3.y - v1.y,v3.z - v1.z };

	vector_3f normal = vector_crossproduct(tmp1, tmp2);
	vector_3f view = vector_3f(0, 0, -1);
	return vector_dotproduct(normal, view) < 0;
}

void rasterizer::RasterizeDot(Triangle& t)
{
	for (auto& p : t.vertex)
	{
		int x = p.pos.x;
		int y = p.pos.y;
		if (!is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
			set_pixel(x, y, dot_color);
		if (is_open_viewportcull && y < 500.f && x < 500.f)
			set_pixel(x, y, dot_color);
	}
}

void rasterizer::RasterizeWire(Triangle& t)
{
	DrawLine(t.vertex[0].pos, t.vertex[1].pos);
	DrawLine(t.vertex[1].pos, t.vertex[2].pos);
	DrawLine(t.vertex[2].pos, t.vertex[0].pos);
}

void rasterizer::DrawLine(vector_4f& begin, vector_4f& end)
{
	auto x1 = begin.x;
	auto y1 = begin.y;
	auto x2 = end.x;
	auto y2 = end.y;

	int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;

	dx = x2 - x1;
	dy = y2 - y1;
	dx1 = fabs(dx);
	dy1 = fabs(dy);
	px = 2 * dy1 - dx1;
	py = 2 * dx1 - dy1;

	if (dy1 <= dx1)
	{
		if (dx >= 0)
		{
			x = x1;
			y = y1;
			xe = x2;
		}
		else
		{
			x = x2;
			y = y2;
			xe = x1;
		}
		if (!is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
			set_pixel(x, y, line_color);
		if (is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
			set_pixel(x, y, line_color);
		for (i = 0; x < xe; i++)
		{
			x = x + 1;
			if (px < 0)
			{
				px = px + 2 * dy1;
			}
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
				{
					y = y + 1;
				}
				else
				{
					y = y - 1;
				}
				px = px + 2 * (dy1 - dx1);
			}
			if (!is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
				set_pixel(x, y, line_color);
			if (is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
				set_pixel(x, y, line_color);
		}
	}
	else
	{
		if (dy >= 0)
		{
			x = x1;
			y = y1;
			ye = y2;
		}
		else
		{
			x = x2;
			y = y2;
			ye = y1;
		}
		if (!is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
			set_pixel(x, y, line_color);
		if (is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
			set_pixel(x, y, line_color);
		for (i = 0; y < ye; i++)
		{
			y = y + 1;
			if (py <= 0)
			{
				py = py + 2 * dx1;
			}
			else
			{
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
				{
					x = x + 1;
				}
				else
				{
					x = x - 1;
				}
				py = py + 2 * (dx1 - dy1);
			}
			if (!is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
				set_pixel(x, y, line_color);
			if (is_open_viewportcull && x >= 0.f && x < 500.f && y >= 0.f && y < 500.f)
				set_pixel(x, y, line_color);
		}
	}
}

void rasterizer::RasterizeTriangle(Triangle& t, int x_min, int x_max, int y_min, int y_max)
{
	vector_3f specular_intensity = { 0.7937, 0.7937, 0.7937 };
	vector_3f interpolated_color = { 128.f,128.f,128.f };
	vector_3f normal_deviation = { 0,0,0 };
	for (int x = x_min; x <= x_max; x++)
	{
		for (int y = y_min; y <= y_max; y++)
		{
			if (a_t == anti_aliasing_type::SSAA)
			{
				for (float i = 0.25; i < 1; i += 0.5)
				{
					for (float j = 0.25; j < 1; j += 0.5)
					{
						if (!InsideTriangle(x + i, y + j, t.vertex)) continue;
						//Barycentric Interpolation
						auto [alpha, beta, gamma] = ComputeBarycentric2D(x + i, y + j, t.vertex);
						//Perspective Correct Interpolation
						float Z = 1.0 / (alpha / t.vertex[0].pos.w + beta / t.vertex[1].pos.w + gamma / t.vertex[2].pos.w);
						float z_interpolated = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
						z_interpolated *= Z;
						//zbuff test
						int index = (y + j - 0.25) * 2 * width * 2 + (x + i - 0.25) * 2;
						if (depth_buf_oversampling[index] <= z_interpolated || z_interpolated < 0.1 || z_interpolated>50) continue;
						depth_buf_oversampling[index] = z_interpolated;

						vector_3f interpolated_normal = alpha * t.vertex[0].normal / t.vertex[0].pos.w + beta * t.vertex[1].normal / t.vertex[1].pos.w + gamma * t.vertex[2].normal / t.vertex[2].pos.w;
						interpolated_normal *= Z;
						vector_3f interpolated_viewspace_p = alpha * t.vertex[0].viewspace_pos / t.vertex[0].pos.w + beta * t.vertex[1].viewspace_pos / t.vertex[1].pos.w + gamma * t.vertex[2].viewspace_pos / t.vertex[2].pos.w;
						interpolated_viewspace_p *= Z;

						if (uv_height != 0)
						{
							float interpolated_uv_x;
							float interpolated_uv_y;
							interpolated_uv_x = alpha * t.vertex[0].uv_pos.x / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.x / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.x / t.vertex[2].pos.w;
							interpolated_uv_y = alpha * t.vertex[0].uv_pos.y / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.y / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.y / t.vertex[2].pos.w;
							interpolated_uv_x *= Z;
							interpolated_uv_y *= Z;
							if (!uv->isNull())
							{
								QColor pixel = uv->pixel(interpolated_uv_x, interpolated_uv_y);
								interpolated_color = {
									(float)pixel.red(),
									(float)pixel.green(),
									(float)pixel.blue()
								};
							}
							//if (interpolated_uv_x<0 || interpolated_uv_x>uv_width || interpolated_uv_y<0 || interpolated_uv_y>uv_height) continue;
							if (!uv_normal->isNull())
							{
								QColor normaluv_pixel;
								normaluv_pixel = uv_normal->pixel(interpolated_uv_x, interpolated_uv_y);
								normal_deviation = {
									2.f * (float)normaluv_pixel.red() / 255.f - 1.f,
									2.f * (float)normaluv_pixel.green() / 255.f - 1.f,
									2.f * (float)normaluv_pixel.blue() / 255.f - 1.f
								};
								normal_deviation.x = normal_deviation.x * t.TBN[0][0] + normal_deviation.x * t.TBN[0][1] + normal_deviation.x * t.TBN[0][2];
								normal_deviation.y = normal_deviation.y * t.TBN[1][0] + normal_deviation.y * t.TBN[1][1] + normal_deviation.y * t.TBN[1][2];
								normal_deviation.z = normal_deviation.z * t.TBN[2][0] + normal_deviation.z * t.TBN[2][1] + normal_deviation.z * t.TBN[2][2];
								interpolated_normal += normal_deviation;
							}
							if (!uv_specular->isNull())
							{
								QColor specularuv_pixel = uv_specular->pixel(interpolated_uv_x, interpolated_uv_y);
								specular_intensity = {
									(float)specularuv_pixel.red() / 255.f,
									(float)specularuv_pixel.green() / 255.f,
									(float)specularuv_pixel.blue() / 255.f
								};
							}
						}
						//float z = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
						//z = Z * z;
						vector_3f tmp = obj_vs.shadow_shader(Screen2World(x, y, z_interpolated, Z));

						fragment_shader_payload payload(interpolated_viewspace_p, tmp, specular_intensity, interpolated_color, interpolated_normal);
						vector_3f pixel_color = fragment_shader(lights, payload);
						set_pixel_oversampling(x + i, y + j, pixel_color);
					}
				}
			}
			else if (a_t == anti_aliasing_type::MSAA)
			{
				bool is_sampling = false;
				vector_3f pixel_color;
				for (float i = 0.25; i < 1; i += 0.5)
				{
					for (float j = 0.25; j < 1; j += 0.5)
					{
						if (!InsideTriangle(x + i, y + j, t.vertex)) continue;
						auto [alpha, beta, gamma] = ComputeBarycentric2D(x + i, y + j, t.vertex);
						float Z = 1.0 / (alpha / t.vertex[0].pos.w + beta / t.vertex[1].pos.w + gamma / t.vertex[2].pos.w);
						if (!is_sampling)
						{
							vector_3f interpolated_normal = alpha * t.vertex[0].normal / t.vertex[0].pos.w + beta * t.vertex[1].normal / t.vertex[1].pos.w + gamma * t.vertex[2].normal / t.vertex[2].pos.w;
							interpolated_normal *= Z;
							vector_3f interpolated_viewspace_p = alpha * t.vertex[0].viewspace_pos / t.vertex[0].pos.w + beta * t.vertex[1].viewspace_pos / t.vertex[1].pos.w + gamma * t.vertex[2].viewspace_pos / t.vertex[2].pos.w;
							interpolated_viewspace_p *= Z;
							if (uv_height != 0)
							{
								float interpolated_uv_x;
								float interpolated_uv_y;
								interpolated_uv_x = alpha * t.vertex[0].uv_pos.x / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.x / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.x / t.vertex[2].pos.w;
								interpolated_uv_y = alpha * t.vertex[0].uv_pos.y / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.y / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.y / t.vertex[2].pos.w;
								interpolated_uv_x *= Z;
								interpolated_uv_y *= Z;
								if (!uv->isNull())
								{
									QColor pixel = uv->pixel(interpolated_uv_x, interpolated_uv_y);
									interpolated_color = {
										(float)pixel.red(),
										(float)pixel.green(),
										(float)pixel.blue()
									};
								}
								//if (interpolated_uv_x<0 || interpolated_uv_x>uv_width || interpolated_uv_y<0 || interpolated_uv_y>uv_height) continue;
								if (!uv_normal->isNull())
								{
									QColor normaluv_pixel;
									normaluv_pixel = uv_normal->pixel(interpolated_uv_x, interpolated_uv_y);
									normal_deviation = {
										2.f * (float)normaluv_pixel.red() / 255.f - 1.f,
										2.f * (float)normaluv_pixel.green() / 255.f - 1.f,
										2.f * (float)normaluv_pixel.blue() / 255.f - 1.f
									};
									normal_deviation.x = normal_deviation.x * t.TBN[0][0] + normal_deviation.x * t.TBN[0][1] + normal_deviation.x * t.TBN[0][2];
									normal_deviation.y = normal_deviation.y * t.TBN[1][0] + normal_deviation.y * t.TBN[1][1] + normal_deviation.y * t.TBN[1][2];
									normal_deviation.z = normal_deviation.z * t.TBN[2][0] + normal_deviation.z * t.TBN[2][1] + normal_deviation.z * t.TBN[2][2];
									interpolated_normal += normal_deviation;
								}
								if (!uv_specular->isNull())
								{
									QColor specularuv_pixel = uv_specular->pixel(interpolated_uv_x, interpolated_uv_y);
									specular_intensity = {
										(float)specularuv_pixel.red() / 255.f,
										(float)specularuv_pixel.green() / 255.f,
										(float)specularuv_pixel.blue() / 255.f
									};
								}
							}
							float z_interpolated = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
							z_interpolated *= Z;
							vector_3f tmp = obj_vs.shadow_shader(Screen2World(x, y, z_interpolated, Z));

							fragment_shader_payload payload(interpolated_viewspace_p, tmp, specular_intensity, interpolated_color, interpolated_normal);
							pixel_color = fragment_shader(lights, payload);
							is_sampling = true;
						}
						float z_interpolated = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
						z_interpolated *= Z;
						int index = (y + j - 0.25) * 2 * width * 2 + (x + i - 0.25) * 2;
						if (depth_buf_oversampling[index] <= z_interpolated || z_interpolated < 0.1 || z_interpolated>50) continue;
						depth_buf_oversampling[index] = z_interpolated;
						set_pixel_oversampling(x + i, y + j, pixel_color);
					}
				}
			}
			else
			{
				if (!InsideTriangle(x, y, t.vertex)) continue;
				//Barycentric Interpolation
				auto [alpha, beta, gamma] = ComputeBarycentric2D(x, y, t.vertex);
				//Perspective Correct Interpolation
				float Z = 1.0 / (alpha / t.vertex[0].pos.w + beta / t.vertex[1].pos.w + gamma / t.vertex[2].pos.w);
				float z_interpolated = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
				z_interpolated *= Z;
				//zbuff test
				if (depth_buf[y * width + x] <= z_interpolated) continue;
				depth_buf[y * width + x] = z_interpolated;

				vector_3f interpolated_normal = alpha * t.vertex[0].normal / t.vertex[0].pos.w + beta * t.vertex[1].normal / t.vertex[1].pos.w + gamma * t.vertex[2].normal / t.vertex[2].pos.w;
				interpolated_normal *= Z;
				vector_3f interpolated_viewspace_p = alpha * t.vertex[0].viewspace_pos / t.vertex[0].pos.w + beta * t.vertex[1].viewspace_pos / t.vertex[1].pos.w + gamma * t.vertex[2].viewspace_pos / t.vertex[2].pos.w;
				interpolated_viewspace_p *= Z;
				if (uv_height != 0)
				{
					float interpolated_uv_x;
					float interpolated_uv_y;
					interpolated_uv_x = alpha * t.vertex[0].uv_pos.x / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.x / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.x / t.vertex[2].pos.w;
					interpolated_uv_y = alpha * t.vertex[0].uv_pos.y / t.vertex[0].pos.w + beta * t.vertex[1].uv_pos.y / t.vertex[1].pos.w + gamma * t.vertex[2].uv_pos.y / t.vertex[2].pos.w;
					interpolated_uv_x *= Z;
					interpolated_uv_y *= Z;
					if (!uv->isNull())
					{
						QColor pixel = uv->pixel(interpolated_uv_x, interpolated_uv_y);
						interpolated_color = {
							(float)pixel.red(),
							(float)pixel.green(),
							(float)pixel.blue()
						};
					}
					//if (interpolated_uv_x<0 || interpolated_uv_x>uv_width || interpolated_uv_y<0 || interpolated_uv_y>uv_height) continue;
					if (!uv_normal->isNull())
					{
						QColor normaluv_pixel;
						normaluv_pixel = uv_normal->pixel(interpolated_uv_x, interpolated_uv_y);
						normal_deviation = {
							2.f * (float)normaluv_pixel.red() / 255.f - 1.f,
							2.f * (float)normaluv_pixel.green() / 255.f - 1.f,
							2.f * (float)normaluv_pixel.blue() / 255.f - 1.f
						};
						normal_deviation.x = normal_deviation.x * t.TBN[0][0] + normal_deviation.x * t.TBN[0][1] + normal_deviation.x * t.TBN[0][2];
						normal_deviation.y = normal_deviation.y * t.TBN[1][0] + normal_deviation.y * t.TBN[1][1] + normal_deviation.y * t.TBN[1][2];
						normal_deviation.z = normal_deviation.z * t.TBN[2][0] + normal_deviation.z * t.TBN[2][1] + normal_deviation.z * t.TBN[2][2];
						interpolated_normal += normal_deviation;
					}
					if (!uv_specular->isNull())
					{
						QColor specularuv_pixel = uv_specular->pixel(interpolated_uv_x, interpolated_uv_y);
						specular_intensity = {
							(float)specularuv_pixel.red() / 255.f,
							(float)specularuv_pixel.green() / 255.f,
							(float)specularuv_pixel.blue() / 255.f
						};
					}
				}
				//float z = alpha * t.vertex[0].pos.z / t.vertex[0].pos.w + beta * t.vertex[1].pos.z / t.vertex[1].pos.w + gamma * t.vertex[2].pos.z / t.vertex[2].pos.w;
				//z *= Z;
				vector_3f tmp = obj_vs.shadow_shader(Screen2World(x, y, z_interpolated, Z));
				fragment_shader_payload payload(interpolated_viewspace_p, tmp, specular_intensity, interpolated_color, interpolated_normal);

				vector_3f pixel_color = fragment_shader(lights, payload);
				set_pixel(x, y, pixel_color);
			}
		}
	}
}

bool rasterizer::InsideTriangle(const float& x, const float& y, const std::vector<Vertex>& v)
{
	//judge whether the pixel is in the triangle
	const vector_2f P(x, y);
	const vector_2f A = { v[0].pos.x,v[0].pos.y };
	const vector_2f B = { v[1].pos.x,v[1].pos.y };
	const vector_2f C = { v[2].pos.x,v[2].pos.y };

	vector_2f AB = B - A;
	vector_2f BC = C - B;
	vector_2f CA = A - C;

	vector_2f AP = P - A;
	vector_2f BP = P - B;
	vector_2f CP = P - C;

	float z[3];
	z[0] = vector_crossproduct(AB, AP);
	z[1] = vector_crossproduct(BC, BP);
	z[2] = vector_crossproduct(CA, CP);

	int tmp = 0;
	for (int i = 0; i < 3; i++)
	{
		if (z[i] > 0)
			tmp++;
		else if (z[i] < 0)
			tmp--;
	}
	return (tmp >= -1 && tmp <= 1) ? false : true;
}

std::tuple<float, float, float> rasterizer::ComputeBarycentric2D(float x, float y, const std::vector<Vertex>& v)//计算重心坐标系数
{
	float a = ((y - v[1].pos.y) * (v[2].pos.x - v[1].pos.x) - (x - v[1].pos.x) * (v[2].pos.y - v[1].pos.y)) /
		((v[0].pos.y - v[1].pos.y) * (v[2].pos.x - v[1].pos.x) - (v[0].pos.x - v[1].pos.x) * (v[2].pos.y - v[1].pos.y));
	float b = ((y - v[2].pos.y) * (v[0].pos.x - v[2].pos.x) - (x - v[2].pos.x) * (v[0].pos.y - v[2].pos.y)) /
		((v[1].pos.y - v[2].pos.y) * (v[0].pos.x - v[2].pos.x) - (v[1].pos.x - v[2].pos.x) * (v[0].pos.y - v[2].pos.y));
	float c = 1 - b - a;
	return{ a,b,c };
}

vector_4f rasterizer::Screen2World(float x, float y, float& z, float& w)
{
	x = x / (0.5 * width) - 1;
	y = y / (0.5 * height) - 1;
	z = (z - f2) / f1;
	x *= w;
	y *= w;
	z *= w;
	vector_4f tmp(x, y, z, w);
	tmp = obj_vs.mvp_inverse * tmp;
	tmp /= tmp.w;
	return tmp;
}

void rasterizer::FXAA()
{
	//FXAA 3.11 quality
	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			//judge boundary
			QColor& cM = get_pixel_color(i, j);
			QColor& cN = get_pixel_color(i, j - 1);
			QColor& cS = get_pixel_color(i, j + 1);
			QColor& cW = get_pixel_color(i - 1, j);
			QColor& cE = get_pixel_color(i + 1, j);
			QColor& cNW = get_pixel_color(i - 1, j - 1);
			QColor& cNE = get_pixel_color(i + 1, j - 1);
			QColor& cSW = get_pixel_color(i - 1, j + 1);
			QColor& cSE = get_pixel_color(i + 1, j + 1);
			float M = ComputeLuma(cM);
			float N = ComputeLuma(cN);
			float S = ComputeLuma(cS);
			float W = ComputeLuma(cW);
			float E = ComputeLuma(cE);
			float NE = ComputeLuma(cNE);
			float NW = ComputeLuma(cNW);
			float SE = ComputeLuma(cSE);
			float SW = ComputeLuma(cSW);
			float MaxLuma = std::max(std::max(N, E), std::max(std::max(W, S), M));
			float MinLuma = std::min(std::min(N, E), std::min(std::min(W, S), M));
			float LumaContrast = MaxLuma - MinLuma;
			float EdgeThreshold = std::max(FXAA_ABSOLUTE_LUMA_THRESHOLD, MaxLuma * FXAA_RELATIVE_LUMA_THRESHOLD);
			bool isEdge = LumaContrast > EdgeThreshold;
			if (!isEdge)
				continue;
			//make sure blend direction
			float LumaGradS = S - M;
			float LumaGradN = N - M;
			float LumaGradW = W - M;
			float LumaGradE = E - M;
			float LumaGradH = abs(NW + NE - 2 * N) + 2 * abs(W + E - 2 * M) + abs(SW + SE - 2 * S);
			float LumaGradV = abs(NW + SW - 2 * W) + 2 * abs(N + S - 2 * M) + abs(NE + SE - 2 * E);
			bool isHorz = LumaGradV >= LumaGradH;
			//calculate blend factor
			float searchEndThreshold;
			float LumaStart, LumaEnd;
			float edgeLength, dst, targetP;
			QColor stepN;
			QColor stepM;
			vector_3f blendColor;
			float blend = 0.f;
			int count = 0;
			int normal = 0;
			int dir1, dir2;
			if (isHorz)
			{
				dir1 = std::max(0, i - EdgeStep[count]), dir2 = std::min(width - 1, i + EdgeStep[count]);
				(abs(LumaGradN) - abs(LumaGradS) > 0) ? normal = -1 : normal = 1;
				if (normal == 1)
				{
					searchEndThreshold = 0.25 * LumaGradS;
					LumaStart = 0.5 * M + 0.5 * S;
				}
				else
				{
					searchEndThreshold = 0.25 * LumaGradN;
					LumaStart = 0.5 * M + 0.5 * N;
				}
				stepM = get_pixel_color(dir1, j);
				stepN = get_pixel_color(dir1, j + normal);
				while (++count < FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT && 0.5 * ComputeLuma(stepM) + 0.5 * ComputeLuma(stepN) - LumaStart <= searchEndThreshold)
				{
					dir1 -= EdgeStep[count];
					if (dir1 < 0)
					{
						dir1 = 0;
						stepM = get_pixel_color(dir1, j);
						stepN = get_pixel_color(dir1, j + normal);
						break;
					}
					stepM = get_pixel_color(dir1, j);
					stepN = get_pixel_color(dir1, j + normal);
				}
				count = 0;
				stepM = get_pixel_color(dir2, j);
				stepN = get_pixel_color(dir2, j + normal);
				while (++count < FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT && 0.5 * ComputeLuma(stepM) + 0.5 * ComputeLuma(stepN) - LumaStart <= searchEndThreshold)
				{
					dir2 += EdgeStep[count];
					if (dir2 > width - 1)
					{
						dir2 = width - 1;
						stepM = get_pixel_color(dir2, j);
						stepN = get_pixel_color(dir2, j + normal);
						break;
					}
					stepM = get_pixel_color(dir2, j);
					stepN = get_pixel_color(dir2, j + normal);
				}
				edgeLength = dir2 - dir1;
				if (dir2 - i > i - dir1)
				{
					dst = i - dir1;
					targetP = dir1;
				}
				else
				{
					dst = dir2 - i;
					targetP = dir2;
				}
			}
			else
			{
				dir1 = std::max(0, j - EdgeStep[count]), dir2 = std::min(height - 1, j + EdgeStep[count]);
				(abs(LumaGradW) - abs(LumaGradE) > 0) ? normal = -1 : normal = 1;
				if (normal == 1)
				{
					searchEndThreshold = 0.25 * LumaGradE;
					LumaStart = 0.5 * M + 0.5 * E;
				}
				else
				{
					searchEndThreshold = 0.25 * LumaGradW;
					LumaStart = 0.5 * M + 0.5 * W;
				}
				stepM = get_pixel_color(i, dir1);
				stepN = get_pixel_color(i + normal, dir1);
				while (++count < FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT && 0.5 * ComputeLuma(stepM) + 0.5 * ComputeLuma(stepN) - LumaStart <= searchEndThreshold)
				{
					dir1 -= EdgeStep[count];
					if (dir1 < 0)
					{
						dir1 = 0;
						stepM = get_pixel_color(i, dir1);
						stepN = get_pixel_color(i + normal, dir1);
						break;
					}
					stepM = get_pixel_color(i, dir1);
					stepN = get_pixel_color(i + normal, dir1);
				}
				count = 0;
				stepM = get_pixel_color(i, dir2);
				stepN = get_pixel_color(i + normal, dir2);
				while (++count < FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT && 0.5 * ComputeLuma(stepM) + 0.5 * ComputeLuma(stepN) - LumaStart <= searchEndThreshold)
				{
					dir2 += EdgeStep[count];
					if (dir2 > height - 1)
					{
						dir2 = height - 1;
						stepM = get_pixel_color(i, dir2);
						stepN = get_pixel_color(i + normal, dir2);
						break;
					}
					stepM = get_pixel_color(i, dir2);
					stepN = get_pixel_color(i + normal, dir2);
				}
				edgeLength = dir2 - dir1;
				if (dir2 - j > j - dir1)
				{
					dst = j - dir1;
					targetP = dir1;
				}
				else
				{
					dst = dir2 - j;
					targetP = dir2;
				}
			}

			stepM = (isHorz) ? get_pixel_color(targetP, j) : get_pixel_color(i, targetP);
			LumaEnd = ComputeLuma(stepM);
			if ((M - LumaStart) * (LumaEnd - LumaStart) < 0)
				blend = 0.5 - dst / edgeLength;
			else
				continue;
			if (isHorz)
				(normal == 1) ? LinearInterpolate(blendColor, cM, cS, blend) : LinearInterpolate(blendColor, cM, cN, blend);
			else
				(normal == 1) ? LinearInterpolate(blendColor, cM, cE, blend) : LinearInterpolate(blendColor, cM, cW, blend);
			int index = (i * width + j) * 4;
			frame_buf[index] = blendColor.x;
			frame_buf[index + 1] = blendColor.y;
			frame_buf[index + 2] = blendColor.z;
		}
	}
}

QColor rasterizer::get_pixel_color(int i, int j)
{
	int index = (i * width + j) * 4;
	return { frame_buf[index],frame_buf[index + 1] ,frame_buf[index + 2] };
}

float rasterizer::ComputeLuma(QColor& color)
{
	return 0.213 * (float)color.red() + 0.715 * (float)color.green() + 0.072 * (float)color.blue();
}

void rasterizer::LinearInterpolate(vector_3f& blendColor, QColor& M, QColor& A, float& weight)
{
	blendColor.x = (1 - weight) * (float)M.red() + weight * (float)A.red();
	blendColor.y = (1 - weight) * (float)M.green() + weight * (float)A.green();
	blendColor.z = (1 - weight) * (float)M.blue() + weight * (float)A.blue();
	return;
}

void rasterizer::UpdateLights()
{
	for (auto& l : lights)
		l.viewspace_pos = to_vector_3f(main_camera->View * l.GetCurPos());
}

void rasterizer::MoveModel(float& x, float& y)
{
	if (main_camera->type)
	{
		float ratio = abs(main_camera->GetCurPos().z / main_camera->zNear);
		//std::cout << ratio;
		x = x / width * (2 * main_camera->zNear * tan(DEG2RAD(main_camera->eye_fov / 2))) * main_camera->aspect_ratio * ratio;
		y = y / height * (2 * main_camera->zNear * tan(DEG2RAD(main_camera->eye_fov / 2))) * ratio;
		vector_3f tmp(x, y, 0);
		set_camera_trans(tmp);
	}
	else
	{
		//float ratio = abs(main_camera->GetCurPos().z / main_camera->zNear);
		x = -x / width * main_camera->width;
		y = -y / height * main_camera->height;
		vector_3f tmp(x, y, 0);
		set_camera_trans(tmp);
	}
}

void rasterizer::set_open_viewfrustumcull()
{
	is_open_viewfrustumcull = !is_open_viewfrustumcull;
}

void rasterizer::set_open_viewportcull()
{
	is_open_viewportcull = !is_open_viewportcull;
}

void rasterizer::set_open_facecull()
{
	is_open_facecull = !is_open_facecull;
}

void rasterizer::set_render_type(render_type& type)
{
	r_t = type;
}

void rasterizer::set_anti_aliasing_type(anti_aliasing_type& type)
{
	a_t = type;
	if (type == anti_aliasing_type::SSAA || type == anti_aliasing_type::MSAA)
	{
		depth_buf_oversampling.resize(height * width * 4);
		frame_buf_oversampling.resize(height * 2, std::vector<vector_3f>(width * 2, { 255,255,255 }));
	}
}

void rasterizer::set_camera(camera* Camera)
{
	main_camera = Camera;
}

void rasterizer::set_camera_trans(vector_3f &trans_delta)
{
	main_camera->set_trans(trans_delta);
}

void rasterizer::set_camera_rotat(vector_3f& rotat_delta)
{
	main_camera->set_rotat(rotat_delta);

}

void rasterizer::set_camera_type(bool& type)
{
	main_camera->type = type;
	main_camera->Init();
}

void rasterizer::set_camera_size(int& val, short& type)
{
	if (main_camera->type)
	{
		//透视投影
		switch (type)
		{
		case 0:
			main_camera->eye_fov = val;
			break;
		case 1:
			main_camera->zNear = -(float)val/10;
			break;
		case 2:
			main_camera->aspect_ratio = (float)val/100;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (type)
		{
		case 0:
			main_camera->width = val;
			break;
		case 1:
			main_camera->height = val;
			break;
		default:
			break;
		}
	}
	main_camera->Init();
}

void rasterizer::set_light(light &l)
{
	lights.emplace_back(l);
}

void rasterizer::set_light_rotat(vector_3f& rotat_delta)
{
	//only calculate the first light
	if (!lights.empty())
		lights[0].set_rotat(rotat_delta);
}

void rasterizer::set_light_color(vector_3f& color, lightcolor_type& type)
{
	switch (type)
	{
	case lightcolor_type::AMBIENT:
		lights[0].amb_light_color = color;
		break;
	case lightcolor_type::DIFFUSE:
		lights[0].dif_light_color = color;
		break;
	case lightcolor_type::SPECULAR:
		lights[0].spe_light_color = color;
		break;
	default:
		break;
	}
}

void rasterizer::set_light_trans(vector_3f& trans_delta)
{
	if (!lights.empty())
	{
		lights[0].set_trans(trans_delta);
	}
}

void rasterizer::set_pixel(int& x, int& y, vector_3f& color)
{

	if (color.x > 255) color.x = 255;
	if (color.y > 255) color.y = 255;
	if (color.z > 255) color.z = 255;
	int index = ((height - y - 1) * width + x) * 4;
	frame_buf[index] = color.x;
	frame_buf[index + 1] = color.y;
	frame_buf[index + 2] = color.z;
	frame_buf[index + 3] = 255;
}

void rasterizer::set_pixel_oversampling(float x, float y, vector_3f& color)
{
	if (x<0 || x>width || y<0 || y>height)
		return;
	if (color.x > 255) color.x = 255;
	if (color.y > 255) color.y = 255;
	if (color.z > 255) color.z = 255;
	int y_index = (2 * height - (y - 0.25) * 2 - 1);
	int x_index = (x - 0.25) * 2;
	frame_buf_oversampling[y_index][x_index] = color;
}

void rasterizer::set_uv(QImage& UV,uv_type type)
{
	switch (type)
	{
	case uv_type::DIFFUSE:
		uv = &UV;
		uv_height = uv->height();
		uv_width = uv->width();
		break;
	case uv_type::NORMAL:
		uv_normal = &UV;
		uv_height = uv_normal->height();
		uv_width = uv_normal->width();
		break;
	case uv_type::SPECULAR:
		uv_specular = &UV;
		uv_height = uv_specular->height();
		uv_width = uv_specular->width();
		break;
	default:
		break;
	}
}

void rasterizer::set_ts_level(int& n)
{
	ts_level = n;
	if (ts_level != 0)
		ts.HullShader(ts_level);
}

void rasterizer::set_fragment_shader(std::function<const vector_3f(std::vector<light>&, const fragment_shader_payload&)> f_shader)
{
	fragment_shader = f_shader;
}

void rasterizer::set_threads_num(int n, multithread_type type)
{
	switch (type)
	{
	case multithread_type::GEO:
		vertex_threads.clear();
		geo_threads_num = n;
		vertex_threads.resize(n);
		break;
	case multithread_type::RAS:
		ras_threads_num = n;
	default:
		break;
	}
}

void rasterizer::clear()
{
	//BitBlt(w.screenHDC, 0, 0, 700, 700, NULL, NULL, NULL, BLACKNESS);
	if (a_t == anti_aliasing_type::SSAA || a_t == anti_aliasing_type::MSAA)
	{
		std::fill(depth_buf_oversampling.begin(), depth_buf_oversampling.end(), std::numeric_limits<float>::infinity());
		std::fill(frame_buf_oversampling.begin(), frame_buf_oversampling.end(), std::vector<vector_3f>(width * 2, { 255,255,255 }));
	}
	else
		std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
	//std::fill(select_buf.begin(), select_buf.end(), std::numeric_limits<int>::infinity());
	std::fill(frame_buf, frame_buf + width * height * 4, (uchar)255);
}

void rasterizer::clearUV()
{
	uv = new QImage;
	uv_normal = new QImage;
	uv_specular = new QImage;
	obj_vs.clear();
}
