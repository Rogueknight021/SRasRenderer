#pragma once
#include"matrix.h"
#include<vector>
#include<unordered_map>
typedef struct HalfEdge HalfEdge;
typedef struct Face Face;
struct Vertex
{
public:
	Vertex();
	Vertex(vector_4f& p);
	//~Vertex();
public:
	int id;
	vector_4f pos;
	vector_3f normal;
	//vector_3f color;
	vector_2f uv_pos;
	vector_3f viewspace_pos;
	HalfEdge* half_edge;
};

struct HalfEdge
{
public:
	HalfEdge();
	HalfEdge(Vertex* v1);
	//~HalfEdge();
public:
	Vertex *v;
	HalfEdge* pair;
	HalfEdge* next=NULL;
	Face* face;
};



struct Face
{
public:
	//~Face();
public:
	HalfEdge* half_edge;
};

class Triangle
{
public:
	Triangle();
	Triangle(std::vector<Vertex>& v);
	void set_vertexs(const Vertex& v1, const Vertex& v2, const Vertex& v3);

public:
	std::vector<Vertex> vertex;
};
