#include"model.h"
model::model()
{
	Init();
}

void model::Init()
{
	set_model();
}
void model::Update()
{
	set_model();
}

void model::release()
{
	Release(vs);
	Release(faces);
	Release(h_edges);
}

void model::create_face(Vertex* vs[3])
{
	Face* face = new Face;
	HalfEdge* halfedges[3];
	for (int i = 0; i < 3; i++)
		halfedges[i] = create_halfedge(vs[i % 3], vs[(i + 1) % 3]);
	for (int i = 0; i < 3; i++)
	{
		halfedges[i]->next = halfedges[(i + 1) % 3];
		halfedges[i]->face = face;
		h_edges.emplace_back(halfedges[i]);
	}
	face->half_edge = halfedges[0];
	faces.emplace_back(face);
	return;
}

HalfEdge* model::create_halfedge(Vertex* v1, Vertex* v2)
{
	std::pair<int, int> halfedge_key = { v1->id,v2->id };
	if (hashmap_halfedge.find(halfedge_key) != hashmap_halfedge.end())
		return hashmap_halfedge[halfedge_key];
	HalfEdge* he = new HalfEdge;
	HalfEdge* he_op = new HalfEdge;
	he->v = v2;
	he_op->v = v1;
	he->pair = he_op;
	he_op->pair = he;
	v1->half_edge = he;
	hashmap_halfedge[halfedge_key] = he;
	hashmap_halfedge[{v2->id, v1->id}] = he_op;
	return he;
}

std::vector<HalfEdge*> model::GetHalfEdgesFromVertex(Vertex* v)
{
	std::vector<HalfEdge*> halfedges;
	HalfEdge* he = v->half_edge;
	HalfEdge* phe = he;
	do
	{
		if (!phe->pair->next)//±ß½ç
		{
			halfedges.emplace_back(phe);
			break;
		}
		halfedges.emplace_back(phe);
		phe = phe->pair->next;
	} while (he!=phe);
	if (!phe->pair->next)
	{
		phe = he->next->next;
		do
		{
			if (!phe->pair->next || !phe)
				break;
			halfedges.emplace_back(phe->pair);
			phe = phe->pair->next->next;
		} while (phe->v->id==v->id);
	}
	return halfedges;
	

}

std::vector<Face*> model::GetFacesFromVertex(Vertex* v)
{
	std::vector<Face*> faces;
	auto halfedges = GetHalfEdgesFromVertex(v);
	for (int i = 0; i < halfedges.size(); i++)
		faces.emplace_back(halfedges[i]->face);
	return faces;
}

std::vector<Vertex*> model::GetNeighborsFromVertex(Vertex* v)
{
	std::vector<Vertex*> neighbors;
	auto faces = GetFacesFromVertex(v);
	for (auto &f : faces)
	{
		auto vs = GetVertexFromFaces(f);
		for (auto& vertex : vs)
		{
			bool isExist = false;
			for (auto& neighbor : neighbors)
			{
				if (vertex->id == neighbor->id)
				{
					isExist = true;
					break;
				}
			}
			if (!isExist&&vertex->id!=v->id)
				neighbors.emplace_back(vertex);
		}
	}
	return neighbors;
}

std::vector<Vertex*> model::GetVertexFromFaces(Face* f)
{
	std::vector<Vertex*> vs;
	HalfEdge* he = f->half_edge;
	do
	{
		vs.emplace_back(he->v);
		he = he->next;
	} while (he!=f->half_edge);
	return vs;
}

std::vector<Triangle*> model::LoopSubdivision(std::vector<Vertex>& oldVertexes)
{
	/*
	* Because I don't save the all pos of new vertexes and 
	* other information like normal,uv pos after loopsubdivision,
	* the object only can iterate once
	*/
	//if (subdivision_level == 0)
	//{
	//	std::vector<Triangle*> newTriangles(1);
	//	Triangle* t = new Triangle(oldVertexes);
	//	newTriangles[0] = t;
	//	return newTriangles;
	//}
	std::vector<Triangle*> newTriangles(4);
	std::vector<Vertex*> newVertexes(3);
	int id = 0;
	//calculate the pos of old vertex
	for (int i = 0; i < 3; i++)
	{
		vector_4f newPos(0, 0, 0, 1);
		if (oldVertexes[i].half_edge->pair->next)//inside
		{
			auto neighbours = GetNeighborsFromVertex(&oldVertexes[i]);
			int n = neighbours.size();
			float u = (5.0 / 8.0 - pow(3.0 / 8.0 + 1.0 / 4.0 * std::cos(2 * M_PI / n), 2)) / n;
			vector_4f neighbourPosSum(0, 0, 0, 1);
			for (auto& neighbour : neighbours)
				neighbourPosSum += neighbour->pos;
			newPos = (1 - n * u) * oldVertexes[i].pos + u * neighbourPosSum;
		}
		else//edge
		{
			auto Neighbor = GetNeighborsFromVertex(&oldVertexes[i]);
			vector_4f neighbourPosSum = Neighbor[0]->pos + Neighbor[1]->pos;
			newPos = 0.75 * oldVertexes[i].pos + 0.125 * neighbourPosSum;
		}
		newPos.w = 1;
		Vertex *newVertex=new Vertex(newPos);
		newVertex->id = id++;
		newVertex->normal = oldVertexes[i].normal;
		newVertex->uv_pos = oldVertexes[i].uv_pos;
		newVertexes[i] = newVertex;
	}
	//calculate the pos of new vertex
	std::unordered_map<std::pair<int, int>, Vertex*> map;

	for (int i = 0; i < 3; i++)
	{
		HalfEdge* he = hashmap_halfedge[{oldVertexes[i].id, oldVertexes[(i + 1) % 3].id}];
		Vertex* v1 = he->pair->v;
		Vertex* v2 = he->v;
		//the vertex has been created
		if (map.find({ v1->id, v2->id }) != map.end())
			continue;
		vector_4f newPos;
		if (!he->pair->next)//edge
			newPos = (v1->pos + v2->pos) / 2.f;
		else//inside
		{
			Vertex* v3 = he->next->v;
			Vertex* v4 = he->pair->next->v;
			newPos = (3.0 / 8.0)*(v1->pos + v2->pos) + (1.0 / 8.0)*(v3->pos + v4->pos);
		}
		Vertex *newVertex=new Vertex(newPos);
		newVertex->id = id++;
		newVertex->normal = 0.5 * oldVertexes[i].normal + 0.5 * oldVertexes[(i + 1) % 3].normal;
		newVertex->uv_pos = 0.5 * oldVertexes[i].uv_pos + 0.5 * oldVertexes[(i + 1) % 3].uv_pos;
		map[{v1->id, v2->id}] = newVertex;
		map[{v2->id, v1->id}] = newVertex;
	}
	//build relationship
	std::vector<Vertex*> center(3);
	for (int i = 0; i < 3; i++)
		center[i] = map[{oldVertexes[i].id, oldVertexes[(i + 1) % 3].id}];
	std::vector<Vertex> tmp(3);
	tmp[0] = *center[0];
	tmp[1] = *center[1];
	tmp[2] = *center[2];
	Triangle* t = new Triangle(tmp);
	newTriangles[0]=t;
	for (int i = 0; i < 3; i++)
	{
		std::vector<Vertex> tmp(3);
		tmp[0] = *newVertexes[i];
		tmp[1] = *center[i];
		tmp[2] = *center[(i + 2) % 3];
		Triangle* t = new Triangle(tmp);
		newTriangles[i+1]=t;
	}
	//release resourse
	Release(newVertexes);
	Release(center);

	return newTriangles;

}

void split_string(std::string& txt, char separator, std::vector<std::string>& f)
{
	int start = 0;
	int length = txt.length();
	for (int i = 0; i < length; i++)
	{
		if (txt[i] == separator)
		{
			f.emplace_back(txt.substr(start, i - start));
			start = i + 1;
		}
		else if (i == length - 1)
			f.emplace_back(txt.substr(start, i - start + 1));
	}
}
void load_obj(std::vector<model>& objects, std::string& path)
{
	std::fstream in(path);
	std::string txt = "";
	;
	if (in)
	{
		model obj;
		while (std::getline(in, txt))
		{
			std::vector<std::string> f;
			if (txt[0] == 'v')
			{
				if (txt[1] == ' ')//vertex pos
				{
					txt.erase(0, 2);
					split_string(txt, ' ', f);
					std::string a = f[0].c_str();
					std::string b = f[1].c_str();
					std::string c = f[2].c_str();
					Vertex* v = new Vertex;
					v->pos = { (float)atof(a.c_str()), (float)atof(b.c_str()), (float)atof(c.c_str()) ,1.f };
					obj.vs.emplace_back(v);

				}
				else if (txt[1] == 't')//uv pos
				{
					txt.erase(0, 3);
					split_string(txt, ' ', f);
					std::string a = f[0].c_str();
					std::string b = f[1].c_str();
					float u = (float)atof(a.c_str());
					float v = (float)atof(b.c_str());
					if (u < 0)
						u =0;
					else if (u > 1)
						u = 1;
					if (v < 0)
						v =0;
					else if (v > 1)
						v = 1;
					v = 1 - v;
					obj.uv_pos.emplace_back(u,v);
				}
				else if (txt[1] == 'n')//normal
				{
					txt.erase(0, 3);
					split_string(txt, ' ', f);
					std::string a = f[0].c_str();
					std::string b = f[1].c_str();
					std::string c = f[2].c_str();
					obj.normal.push_back({ (float)atof(a.c_str()), (float)atof(b.c_str()), (float)atof(c.c_str()) });
				}
			}
			else if (txt[0] == 'f')
			{
				txt.erase(0, 2);
				split_string(txt, ' ', f);
				std::vector<std::string> v;
				std::vector<std::string> u;
				std::vector<std::string> n;
				for (int i = 0; i < f.size(); i++)
				{
					std::vector<std::string> num;
					split_string(f[i], '/', num);
					v.emplace_back(num[0]);
					if (num.size() > 2)
					{
						u.emplace_back(num[1]);
						n.emplace_back(num[2]);
					}
					else if (num.size() > 1)
						u.emplace_back(num[1]);
				}
				for (int i = 0; i < f.size() - 2; i++)
				{
					vector_3i ind = { atoi(v[0].c_str()),atoi(v[i + 1].c_str()),atoi(v[i + 2].c_str()) };
					obj.ind.emplace_back(ind);
					//create half struct
					Vertex* vs[3] = { obj.vs[ind.x-1],obj.vs[ind.y-1] ,obj.vs[ind.z-1] };
					vs[0]->id = ind.x;
					vs[1]->id = ind.y;
					vs[2]->id = ind.z;
					obj.create_face(vs);

					if (!u.empty())
						obj.uv_ind.push_back({ atoi(u[0].c_str()),atoi(u[i+1].c_str()),atoi(u[i+2].c_str()) });
					if (!n.empty())
						obj.normal_ind.push_back({ atoi(n[0].c_str()),atoi(n[i+1].c_str()),atoi(n[i+2].c_str()) });
				}
			}
		}
		objects.emplace_back(obj);
	}

}
void load_uv(model& obj, QString& path,uv_type type)
{
	if (path.isEmpty())
		return;
	switch (type)
	{
	case uv_type::DIFFUSE:
		obj.uv.load(path);
		obj.uv_height = obj.uv.height();
		obj.uv_width = obj.uv.width();
		break;
	case uv_type::NORMAL:
		obj.uv_normal.load(path);
		obj.uv_height = obj.uv_normal.height();
		obj.uv_width = obj.uv_normal.width();
		break;
	case uv_type::SPECULAR:
		obj.uv_specular.load(path);
		obj.uv_height = obj.uv_specular.height();
		obj.uv_width = obj.uv_specular.width();
		break;
	default:
		break;
	}
}
