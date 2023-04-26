#pragma once
#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QPainter>
#include <QRunnable>
#include <QmouseEvent>
#include "rasterizer.h"
#include "ui_Window.h"
class Window : public QWidget
{
	Q_OBJECT

public:
	explicit Window(QWidget* parent = Q_NULLPTR);

	//dot,line,triangle
	void set_render_type(render_type type);
	//culling
	void set_open_viewfrustumcull();
	void set_open_viewportcull();
	void set_open_facecull();
	//antialiasing
	void set_anti_aliasing_type(anti_aliasing_type type);
	//tessellation shader
	void set_ts_level(int n);
	//fragment shader
	void set_fragment_shader_type(std::function<const vector_3f(std::vector<light>&, const fragment_shader_payload&)> f_shader);
	
	//control camera
	void set_camera_type(bool type);
	void set_camera_rotat(vector_3f& rotat_delta);
	void set_camera_size(int& val, short type);
	//control light
	void set_light_rotat(vector_3f& rotat_delta);
	void set_light_trans(vector_3f& trans_delta);
	void set_light_color(vector_3f& color, lightcolor_type type);

	//Multithread
	void set_open_multithread();

	void UpdateShadowmap();
	void test();

public:
	std::vector<model> models;
	cubemap skybox;
	QTimer timer;

private:
	void Init();
	void set_frame_buf(uchar* Frame_buf);

	//Input event
	void paintEvent(QPaintEvent*) override;
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

	//void getAABB();
private:
	Ui::Window ui;
	const uchar* frame_buf;
	QImage* canvas;
	QTime time;//calculate duration

	//std::pair<vector_4f, vector_4f> world_scene_AABB;//first:minPoint,second:maxPoint

	rasterizer* r;//core
	QString anti_type = "Anti:None";
	camera* main_camera;
	std::list<light> lights;

	QPoint last_pos;
	bool is_open_multithread = true;
private slots:
	void Render();
};