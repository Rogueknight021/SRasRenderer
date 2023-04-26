#include "Window.h"

Window::Window(QWidget *parent)
	: QWidget(parent)
{
	//setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	ui.setupUi(this);
	frame_buf = new uchar[height() * width() * 4];
	canvas = new QImage(frame_buf, width(), height(), QImage::Format_RGBA8888);
    //std::cout << QThread::currentThreadId();
    Init();
}

void Window::set_render_type(render_type type)
{
    r->set_render_type(type);
}

void Window::set_open_viewfrustumcull()
{
    r->set_open_viewfrustumcull();
}

void Window::set_open_viewportcull()
{
    r->set_open_viewportcull();
}

void Window::set_open_facecull()
{
    r->set_open_facecull();
}

void Window::set_anti_aliasing_type(anti_aliasing_type type)
{
    anti_type = "Anti:";
    r->set_anti_aliasing_type(type);
    switch (type)
    {
    case anti_aliasing_type::None:
        anti_type += "None";
        break;
    case anti_aliasing_type::SSAA:
        anti_type += "SSAA";
        break;
    case anti_aliasing_type::MSAA:
        anti_type += "MSAA";
        break;
    case anti_aliasing_type::FXAA:
        anti_type += "FXAA";
        break;
    default:
        break;
    }
}

void Window::set_ts_level(int n)
{
    r->set_ts_level(n);
}

void Window::set_fragment_shader_type(std::function<const vector_3f(std::vector<light>&, const fragment_shader_payload&)> f_shader)
{
    r->set_fragment_shader(f_shader);
}

void Window::set_camera_type(bool type)
{
    r->set_camera_type(type);
    //UpdateShadowmap();
}

void Window::set_camera_rotat(vector_3f& rotat_delta)
{
    r->set_camera_rotat(rotat_delta);
}

void Window::set_camera_size(int& val, short type)
{
    r->set_camera_size(val, type);
}

void Window::set_light_rotat(vector_3f& rotat_delta)
{
    r->set_light_rotat(rotat_delta);
    UpdateShadowmap();
}

void Window::set_light_trans(vector_3f& trans_delta)
{
    r->set_light_trans(trans_delta);
    UpdateShadowmap();
}

void Window::set_light_color(vector_3f& color, lightcolor_type type)
{
    r->set_light_color(color, type);
}

void Window::set_open_multithread()
{
    is_open_multithread = !is_open_multithread;
    if (is_open_multithread)
    {
        r->set_threads_num(2, multithread_type::GEO);
        r->set_threads_num(4, multithread_type::RAS);
    }
    else
    {
        r->set_threads_num(1, multithread_type::GEO);
        r->set_threads_num(1, multithread_type::RAS);
    }
}

void Window::Init()
{
    r = new rasterizer(height(), width());
    main_camera = new camera({ 0,0,5 }, { 0,0,0 }, { 0,1,0 }, 45.f, 1.f, 0.1, 50.f, 1);
    //main_camera = new camera({ 5,20,0 }, { 0,0,0 }, { 1,0,0 }, 45.f, 1.f, 1.f, 50.f, 1);
    //camera* main_camera = new camera({ 0,0,15 }, { 0,0,0 }, {0,1,0}, 200.f, 200.f, 1.f, 50.f, 0);
    //camera* main_camera = new camera(.f,20.f,1.f,50.f,{ 0,0.5,10 }, { 0,0,0 });
    light tmp_l = { {5,0,20},{500,500,500} };
    lights.emplace_back(tmp_l);

    r->set_camera(main_camera);
    r->set_light(tmp_l);
    r->set_fragment_shader(no_shader);
    connect(&timer, &QTimer::timeout, this, &Window::Render);
    ui.FPS->setText("FPS:");
    ui.Anti->setText(anti_type);

    if (is_open_multithread)
    {
        r->set_threads_num(2, multithread_type::GEO);
        r->set_threads_num(4, multithread_type::RAS);
    }
    else
    {
        r->set_threads_num(1, multithread_type::GEO);
        r->set_threads_num(1, multithread_type::RAS);
    }
}

void Window::set_frame_buf(uchar* Frame_buf)
{
    frame_buf = Frame_buf;
    delete canvas;
    canvas = new QImage(frame_buf, width(), height(), QImage::Format_RGBA8888);
}

void Window::paintEvent(QPaintEvent* event)
{
    if (canvas)
    {
        QPainter painter(this);
        painter.drawImage(0, 0, *canvas);
    }
    else
        QWidget::paintEvent(event);
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (!models.empty())
        {
            QPoint pos = event->pos();
            float x = pos.x() - last_pos.x();
            float y = last_pos.y() - pos.y();
            r->MoveModel(x, y);
            last_pos = pos;
        }
    }
    else if (event->buttons() & Qt::RightButton)
    {
        if (!models.empty())
        {
            QPoint pos = event->pos();
            float ratio = 15;
            float x = pos.x() - last_pos.x();
            float y = pos.y() - last_pos.y();
            x = x / (x * x + y * y)*ratio;
            y = y / (x * x + y * y)*ratio;
            vector_3f tmp(y, x, 0);
            models[0].set_rotat(tmp);
            last_pos = pos;
            UpdateShadowmap();
        }
    }
    
}

void Window::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        last_pos = event->pos();
    }
    else if (event->button() == Qt::RightButton)
    {

    }
}

void Window::wheelEvent(QWheelEvent* event)
{
    if (event->delta() > 0)
    {
        vector_3f tmp = -(main_camera->eye_pos - main_camera->look_at).normalized()/10;
        r->set_camera_trans(tmp);
    }
    else
    {
        vector_3f tmp = (main_camera->eye_pos - main_camera->look_at).normalized() / 10;
        r->set_camera_trans(tmp);
    }
    //UpdateShadowmap();
}
//void Window::getAABB()
//{
//    vector_4f minPoint = { FLT_MAX,FLT_MAX ,FLT_MAX ,1}, maxPoint = { FLT_MIN,FLT_MIN ,FLT_MIN ,1};
//    for (auto &model:models)
//    {
//        for (int j = 0; j < model.pos.size(); j++)
//        {
//            vector_4f tmp_pos = model.Model * to_vector_4f(model.pos[j],1);
//            minPoint.x = std::min(tmp_pos.x, minPoint.x);
//            minPoint.y = std::min(tmp_pos.y, minPoint.y);
//            minPoint.z = std::min(tmp_pos.z, minPoint.z);
//            maxPoint.x = std::max(tmp_pos.x, maxPoint.x);
//            maxPoint.y = std::max(tmp_pos.y, maxPoint.y);
//            maxPoint.z = std::max(tmp_pos.z, maxPoint.z);
//        }
//    }
//    world_scene_AABB.first = minPoint;
//    world_scene_AABB.second = maxPoint;
//}
void Window::UpdateShadowmap()
{
    for (auto& l : r->lights)
    {
        std::fill(l.shadowmap.begin(), l.shadowmap.end(), std::vector<float>(l.shadowmap_w, std::numeric_limits<float>::infinity()));
        for (auto& m : models)
            r->Render(m, l);
    }
}

void Window::test()
{
    models.clear();
    main_camera = new camera({ 0,0,15 }, { 0,0,0 }, { 0,1,0 }, 45.f, 1.f, 0.1, 50.f, 1);
    r->set_camera(main_camera);
    r->set_fragment_shader(phong_shader);
    set_render_type(render_type::Triangle);
    //set_anti_aliasing_type(anti_aliasing_type::SSAA);
    std::string path = "C:/Users/86183/Desktop/course/graduation design/Renderer/Renderer/models/spot/spot_triangulated_good.obj";
    //std::string path = "C:/Users/86183/Desktop/course/graduation design/Renderer/Renderer/models/cube/cube.obj";
    load_obj(models, path);
    //models[0].subdivision_level = 1;
    //vector_3i tmp= models[0].ind[0];
    //std::vector<Vertex> tmp_vs = { *models[0].vs[tmp.x - 1] ,*models[0].vs[tmp.y - 1] ,*models[0].vs[tmp.z- 1] };
    //models[0].LoopSubdivision(tmp_vs);
    //auto neighbours = models[0].GetNeighborsFromVertex(models[0].vs[0]);
    //QString cubemap_path = "C:/Users/86183/Desktop/course/graduation design/Renderer/Renderer/cubemap/cubemap.png";
    //load_cubemap(skybox, cubemap_path);

    //model plane1;
    //plane1.pos = { {1,-1,1},{1,-1,-1},{-1,-1,1},{-1,-1,-1} };
    //plane1.ind = { {1,2,3},{3,2,4} };
    //plane1.normal = { {0,1,0} };
    //plane1.normal_ind = { {1,1,1},{1,1,1} };
    model plane;
    vector_4f v = { 5,5,-8 ,1 };
    plane.vs.push_back(new Vertex(v));
    v = { 5,-5,-8 ,1 };
    plane.vs.push_back(new Vertex(v));
    v = { -5,-5,-5 ,1 };
    plane.vs.push_back(new Vertex(v));
    v = { -5,5,-5 ,1 };
    plane.vs.push_back(new Vertex(v));
    plane.ind = { {3,2,1},{3,1,4 } };
    plane.normal = { {0,0,1} };
    plane.normal_ind = { {1,1,1},{1,1,1} };
    //model plane2;
    //plane2.pos = { {-5,-5,5},{5,-5,-5}, {-5,-5,-5} };
    //plane2.ind = { {1,2,3} };
    //plane2.normal = { {0,1,0} };
    //plane2.normal_ind = { {1,1,1}};
    //model plane3;
    //plane3.pos = { {5,-5,5},{5,-5,-5},{0,-5,0},{-5,-5,-5} };
    //plane3.ind = { {1,2,4} };
    //plane3.normal = { {0,1,0} };
    //plane3.normal_ind = { {1,1,1} };
    //model triangle;
    //triangle.pos = { {0,0,0},{0,5,0},{5,0,0} };
    //triangle.ind = { {1,2,3} };
    //models.emplace_back(plane1);
    models.emplace_back(plane);
    //models.emplace_back(plane2);
    //getAABB();
    UpdateShadowmap();
    timer.start(1);
}

void Window::Render()
{
    time.start();
    r->clear();//clear buffer

    /*
    * save shadowmap
    */
    //uchar* fm;
    //fm = new uchar[500*500];
    //for (int i = 0; i < 500; i++)
    //    for (int j = 0; j < 500; j++)
    //        fm[i*500+j] = (int)(r->lights[0].shadowmap[i][j]);
    //QImage tmp=QImage(fm, width(), height(), QImage::Format_Grayscale8);
    //tmp.save("C:/Users/86183/Desktop/shadow.bmp", "BMP", 100);
    //delete fm;

    /*
    * renderpass
    */
    r->UpdateLights();
    for (int i = 0; i < models.size(); i++)
    {
        models[i].Update();
        r->obj_idx = i;
        if (!models[i].uv.isNull())
            r->set_uv(models[i].uv, uv_type::DIFFUSE);
        if (!models[i].uv_normal.isNull())
            r->set_uv(models[i].uv_normal, uv_type::NORMAL);
        if (!models[i].uv_specular.isNull())
            r->set_uv(models[i].uv_specular, uv_type::SPECULAR);
        r->Multithread(models[i]);
        r->clearUV();
    }
    //Render skybox
    if (!skybox.uv.isNull())
    {
        r->set_uv(skybox.uv, uv_type::DIFFUSE);
        r->Render(skybox);
        r->clearUV();
    }
    //FXAA
    if (r->a_t == anti_aliasing_type::FXAA)
        r->FXAA();

    set_frame_buf(r->frame_buf);
    //caculate fps
    QString fps = "FPS:";
    (time.elapsed() == 0) ? fps.append("Max") : fps.append(QString::number((int)(1 / (time.elapsed() / 1000.f))));
    ui.FPS->setText(fps);
    ui.Anti->setText(anti_type);

    update();

}