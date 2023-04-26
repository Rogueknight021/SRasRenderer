#pragma once
#include"object.h"
enum class uv_type { DIFFUSE, NORMAL, SPECULAR };

namespace std
{
	template<class T1, class T2>
	struct hash<std::pair<T1, T2>>
	{
		std::size_t operator()(const std::pair<T1, T2>& pair) const noexcept
		{

			size_t h1 = hash<T1>()(pair.first);
			size_t h2 = hash<T2>()(pair.second);
			return h1 ^ h2;
		}
	};
}

template<typename T> 
void Release(std::vector<T>& v)
{
	for (auto it = v.begin(); it != v.end(); it++)
	{
		if (*it != NULL)
		{
			delete* it;
			*it = NULL;
		}
	}
}
struct model :public object
{
public:
	model();
	void Init();
	void Update();
	void release();
	void create_face(Vertex* vs[3]);
	std::vector<HalfEdge*> GetHalfEdgesFromVertex(Vertex* v);
	std::vector<Face*> GetFacesFromVertex(Vertex* v);
	std::vector<Vertex*> GetNeighborsFromVertex(Vertex* v);
	std::vector<Vertex*> GetVertexFromFaces(Face* f);
	std::vector<Triangle*> LoopSubdivision(std::vector<Vertex>& oldVertexes);
public:
	std::vector<Vertex*> vs;
	std::vector<vector_3i> ind;//index of vertex
	std::vector<vector_3f> normal;
	std::vector<vector_3i> normal_ind;//index of normal
	std::vector<vector_2f> uv_pos;
	std::vector<vector_3i> uv_ind;//index of uv pos
	std::vector<HalfEdge*> h_edges;
	std::vector<Face*> faces;

	std::unordered_map<std::pair<int, int>, HalfEdge*> hashmap_halfedge;
	QImage uv;//diffuse uv
	QImage uv_normal;
	QImage uv_specular;
	int uv_height, uv_width;
	
	//loop subdivison(not used)
	int subdivision_level = 0;
private:
	HalfEdge* create_halfedge(Vertex* v1, Vertex* v2);
};

void split_string(std::string& txt, char separator, std::vector<std::string>& f);

void load_obj(std::vector<model>& objects, std::string& path);

void load_uv(model& obj, QString& path, uv_type type);