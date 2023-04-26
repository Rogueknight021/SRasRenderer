#include"cubemap.h"
cubemap::cubemap()
{
	set_model();
	size = 0.f;
	ind.resize(12);
	//back
	ind[0] = { 5,1,4 };
	ind[1] = { 5,4,8 };
	//left
	ind[2] = { 3,7,8 };
	ind[3] = { 3,8,4 };
	//front
	ind[4] = { 2,6,3 };
	ind[5] = { 6,7,3 };
	//right
	ind[6] = { 1,5,2 };
	ind[7] = { 5,6,2 };
	//top
	ind[8] = { 5,8,6 };
	ind[9] = { 8,7,6 };
	//bottom
	ind[10] = { 1,2,3 };
	ind[11] = { 1,3,4 };
}
void cubemap::Init()
{
	pos.resize(8);
	pos[0] = { 1,-1,-1 };
	pos[1] = { 1,-1,1 };
	pos[2] = { -1,-1,1 };
	pos[3] = { -1,-1,-1 };
	pos[4] = { 1,1,-1 };
	pos[5] = { 1,1,1 };
	pos[6] = { -1,1,1 };
	pos[7] = { -1,1,-1 };
	for (auto& v : pos)
		v *= size;
	uv_pos.resize(24);
	uv_ind.resize(12);
	//сп╢М
	//top
	uv_pos[0] = { size,0 };
	uv_pos[1] = { 2*size-1,0 };
	uv_pos[2] = { 2*size-1,size-1 };
	uv_pos[3] = { size,size - 1 };
	//buttom
	uv_pos[4] = { size,2*size };
	uv_pos[5] = { 2 * size - 1,2*size };
	uv_pos[6] = { 2 * size - 1,3 * size - 1 };
	uv_pos[7] = { size,3 * size - 1 };
	//left
	uv_pos[8] = { 0,size };
	uv_pos[9] = { size - 1,size };
	uv_pos[10] = { size - 1,2 * size - 1 };
	uv_pos[11] = { 0,2 * size - 1 };
	//front
	uv_pos[12] = { size,size };
	uv_pos[13] = { 2*size - 1,size };
	uv_pos[14] = { 2*size - 1,2 * size - 1 };
	uv_pos[15] = { size,2 * size - 1 };
	//right
	uv_pos[16] = { 2*size,size };
	uv_pos[17] = { 3 * size - 1,size };
	uv_pos[18] = { 3 * size - 1,2 * size - 1 };
	uv_pos[19] = { 2*size,2 * size - 1 };
	//back
	uv_pos[20] = { 3 * size,size };
	uv_pos[21] = { 4 * size - 1,size };
	uv_pos[22] = { 4 * size - 1,2 * size - 1 };
	uv_pos[23] = { 3 * size,2 * size - 1 };
	//front
	uv_ind[4] = { 15,14,16 };
	uv_ind[5] = { 14,13,16 };
	//left
	uv_ind[2] = { 11,10,9 };
	uv_ind[3] = { 11,9,12 };
	//back
	uv_ind[0] = { 21,24,23 };
	uv_ind[1] = { 21,23,22 };
	//right
	uv_ind[6] = { 19,18,20 };
	uv_ind[7] = { 18,17,20 };
	//top
	uv_ind[8] = { 2,1,3 };
	uv_ind[9] = { 1,4,3 };
	//bottom
	uv_ind[10] = { 6,7,8 };
	uv_ind[11] = { 6,8,5 };
}
void cubemap::Update()
{

}

void cubemap::Clear()
{
	uv = QImage();
	pos.clear();
	uv_pos.clear();
	uv_ind.clear();
}

void load_cubemap(cubemap& cm,QString& path)
{
	if (path.isEmpty())
		return;
	cm.uv.load(path);
	cm.size = (cm.uv.height())/3;
	cm.Init();
}