#include "Renderer.h"
static inline QString GenerateStyleSheet(QColor color)
{
    return "background-color: rgb("
        + QString::number(static_cast<int>(color.red())) + ','
        + QString::number(static_cast<int>(color.green())) + ','
        + QString::number(static_cast<int>(color.blue())) + ");";
}

Renderer::Renderer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QString tmp = "x_angle:0";
    ui.camera_r_x->setText(tmp);
    tmp = "y_angle:0";
    ui.camera_r_y->setText(tmp);
    tmp = "z_angle:0";
    ui.camera_r_z->setText(tmp);
}

void Renderer::on_load_model_triggered()
{
    std::string file_name = QFileDialog::getOpenFileName(this,"choose obj","","OBJ (*.obj)").toStdString().data();
    if (file_name.empty())
        return;
    load_obj(ui.widget->models, file_name);
    ui.widget->UpdateShadowmap();
    ui.widget->timer.start(1);
}

void Renderer::on_load_diffuseuv_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "choose uv", "", "png (*.png);;tif (*.tif);;jpg (*.jpg)");
    if(!ui.widget->models.empty())
        load_uv(ui.widget->models[0], file_name,uv_type::DIFFUSE);
}

void Renderer::on_load_normaluv_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "choose uv", "", "png (*.png);;tif (*.tif);;jpg (*.jpg)");
    if (!ui.widget->models.empty())
        load_uv(ui.widget->models[0], file_name, uv_type::NORMAL);
}

void Renderer::on_load_specularuv_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "choose uv", "", "png (*.png);;tif (*.tif);;jpg (*.jpg)");
    if (!ui.widget->models.empty())
        load_uv(ui.widget->models[0], file_name, uv_type::SPECULAR);
}
void Renderer::on_load_cubemap_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "choose cubemap", "", "png (*.png);;tif (*.tif)");
    load_cubemap(ui.widget->skybox, file_name);
}

void Renderer::on_load_example_triggered()
{
    ui.widget->test();
}

void Renderer::on_actionSSAA_triggered(bool signal)
{
    if (signal)
    {
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::SSAA);
        ui.actionMSAA->setChecked(false);
        ui.actionFXAA->setChecked(false);
    }
    else
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::None);
}

void Renderer::on_actionMSAA_triggered(bool signal)
{
    if (signal)
    {
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::MSAA);
        ui.actionSSAA->setChecked(false);
        ui.actionFXAA->setChecked(false);
    }
    else
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::None);
}

void Renderer::on_actionFXAA_triggered(bool signal)
{
    if (signal)
    {
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::FXAA);
        ui.actionSSAA->setChecked(false);
        ui.actionMSAA->setChecked(false);
    }
    else
        ui.widget->set_anti_aliasing_type(anti_aliasing_type::None);
}

void Renderer::on_pipeline_cull_facecull_triggered()
{
    ui.widget->set_open_facecull();
}

void Renderer::on_pipeline_cull_viewfrustumcull_triggered()
{
    ui.widget->set_open_viewfrustumcull();
}

void Renderer::on_pipeline_cull_viewportcull_triggered()
{
    ui.widget->set_open_viewportcull();
}

void Renderer::on_setting_multithread_triggered()
{
    ui.widget->set_open_multithread();
}

void Renderer::on_ambient_color_clicked()
{
    QColor color = QColorDialog::getColor(ambient_color, this, "Select Specular Color");
    if (color.isValid())
    {
        ui.AmbientColor->setStyleSheet(GenerateStyleSheet(color));
        vector_3f tmp(
            (float)color.red() / 255,
            (float)color.green() / 255,
            (float)color.blue() / 255
        );
        ui.widget->set_light_color(tmp, lightcolor_type::AMBIENT);
    }
}

void Renderer::on_diffuse_color_clicked()
{
    QColor color = QColorDialog::getColor(diffuse_color, this, "Select Specular Color");
    if (color.isValid())
    {
        ui.DiffuseColor->setStyleSheet(GenerateStyleSheet(color));
        vector_3f tmp(
            (float)color.red() / 255,
            (float)color.green() / 255,
            (float)color.blue() / 255
        );
        ui.widget->set_light_color(tmp, lightcolor_type::DIFFUSE);
    }
}

void Renderer::on_specular_color_clicked()
{
    QColor color = QColorDialog::getColor(specular_color, this, "Select Specular Color");
    if (color.isValid())
    {
        ui.SpecularColor->setStyleSheet(GenerateStyleSheet(color));
        vector_3f tmp(
            (float)color.red()/255,
            (float)color.green()/255,
            (float)color.blue()/255
        );
        ui.widget->set_light_color(tmp, lightcolor_type::SPECULAR);
    }
}

void Renderer::on_camera_persp_clicked()
{
    ui.stackedWidget->setCurrentIndex(0);
    ui.widget->set_camera_type(1);
}

void Renderer::on_camera_ortho_clicked()
{
    ui.stackedWidget->setCurrentIndex(1);
    ui.widget->set_camera_type(0);
}

void Renderer::on_camera_reset_clicked()
{
    vector_3f rotat=-camera_rotat;
    camera_rotat = { 0,0,0 };
    ui.widget->set_camera_rotat(rotat);
    QString tmp = "x_angle:0";
    ui.camera_r_x->setText(tmp);
    tmp = "y_angle:0";
    ui.camera_r_y->setText(tmp);
    tmp = "z_angle:0";
    ui.camera_r_z->setText(tmp);
}

void Renderer::on_Delete_clicked()
{
    for (auto& model : ui.widget->models)
        model.release();
    ui.widget->models.clear();
}
void Renderer::on_delete_cubemap_clicked()
{
    ui.widget->skybox.Clear();
}

void Renderer::when_combobox_fragment_shader_activated(int index)
{
    switch (index)
    {
    case 0:
        ui.widget->set_fragment_shader_type(no_shader);
        break;
    case 1:
        ui.widget->set_fragment_shader_type(phong_shader);
        break;
    default:
        break;
    }
}

void Renderer::when_combobox_render_type_activated(int index)
{
    switch (index)
    {
    case 0:
        ui.widget->set_render_type(render_type::Dot);
        break;
    case 1:
        ui.widget->set_render_type(render_type::Line);
        break;
    case 2:
        ui.widget->set_render_type(render_type::Triangle);
        break;
    default:
        break;
    }
}

void Renderer::when_vertical_slider_persp_camera_fov_sliderMoved(int value)
{
    ui.widget->set_camera_size(value, 0);
}

void Renderer::when_vertical_slider_persp_camera_near_sliderMoved(int value)
{
    ui.widget->set_camera_size(value, 1);
}

void Renderer::when_vertical_slider_persp_camera_ratio_sliderMoved(int value)
{
    ui.widget->set_camera_size(value, 2);
}

void Renderer::when_vertical_slider_ortho_camera_width_sliderMoved(int value)
{
    ui.widget->set_camera_size(value, 0);
}

void Renderer::when_vertical_slider_ortho_camera_height_sliderMoved(int value)
{
    ui.widget->set_camera_size(value, 1);
}

void Renderer::when_dial_lightpos_sliderMoved(int value)
{
    int delta = value - last_lightpos_value;
    vector_3f rotat(0,0,0);
    (delta > 0 && delta < 90) ? (rotat.z = -3.6) : (rotat.z = 3.6);
    ui.widget->set_light_rotat(rotat);
    last_lightpos_value = value;
}

void Renderer::when_horizontal_slider_lightposz_sliderMoved(int value)
{
    int delta = value - last_lightpos_z_value;
    vector_3f trans(0, 0, 0);
    (delta > 0) ? (trans.z = 0.1) : (trans.z = -0.1);
    ui.widget->set_light_trans(trans);
    last_lightpos_z_value = value;
}

void Renderer::when_dial_camera_anglex_sliderMoved(int value)
{
    int delta = value - last_camera_value;
    vector_3f rotat(0, 0, 0);
    (delta > 0 && delta < 90) ? (rotat.x = -3.6) : (rotat.x = 3.6);
    camera_rotat += rotat;
    ui.widget->set_camera_rotat(rotat);
    last_camera_value = value;
    QString tmp = "x_angle:";
    tmp.append(QString::number(camera_rotat.x));
    ui.camera_r_x->setText(tmp);
}

void Renderer::when_dial_camera_angley_sliderMoved(int value)
{
    int delta = value - last_camera_value;
    vector_3f rotat(0, 0, 0);
    (delta > 0 && delta < 90) ? (rotat.y = -3.6) : (rotat.y = 3.6);
    camera_rotat += rotat;
    ui.widget->set_camera_rotat(rotat);
    last_camera_value = value;
    QString tmp = "y_angle:";
    tmp.append(QString::number(camera_rotat.y));
    ui.camera_r_y->setText(tmp);
}

void Renderer::when_dial_camera_anglez_sliderMoved(int value)
{
    int delta = value - last_camera_value;
    vector_3f rotat(0, 0, 0);
    (delta > 0 && delta < 90) ? (rotat.z = -3.6) : (rotat.z = 3.6);
    camera_rotat += rotat;
    ui.widget->set_camera_rotat(rotat);
    last_camera_value = value;
    QString tmp = "z_angle:";
    tmp.append(QString::number(camera_rotat.z));
    ui.camera_r_z->setText(tmp);
}

void Renderer::on_subdivision_level_sliderMoved(int value)
{
    ui.widget->set_ts_level(value);
    /*
    * you can use the loop subdivision by below code;
    */
    //if(!ui.widget->models.empty())
    //    ui.widget->models[0].subdivision_level = value;
}

void Renderer::test()
{
    
}