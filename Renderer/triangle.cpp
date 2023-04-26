#include"triangle.h"
Vertex::Vertex(){}
Vertex::Vertex(vector_4f& p) :pos(p){}

HalfEdge::HalfEdge(){}
HalfEdge::HalfEdge(Vertex* v1):v(v1){}

Triangle::Triangle()
{
	vertex.resize(3);
}
Triangle::Triangle(std::vector<Vertex>& v)
{
	vertex = v;
}

void Triangle::set_vertexs(const Vertex &v1,const Vertex &v2,const Vertex &v3)
{
	vertex[0] = v1;
	vertex[1] = v2;
	vertex[2] = v3;
}
