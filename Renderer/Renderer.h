#pragma once
#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include<QColorDialog>
#include "Window.h"
#include "ui_Renderer.h"
class Renderer : public QMainWindow
{
    Q_OBJECT

public:
    explicit Renderer(QWidget *parent = Q_NULLPTR);
private:
    Ui::RendererClass ui;

    int last_lightpos_value = 0;
    int last_lightpos_z_value = 0;
    int last_camera_value = 0;
    vector_3f camera_rotat = { 0,0,0 };

    QColor ambient_color;
    QColor diffuse_color;
    QColor specular_color;
private slots:
    void on_load_model_triggered();

    void on_load_diffuseuv_clicked();

    void on_load_normaluv_clicked();

    void on_load_specularuv_clicked();

    void on_load_cubemap_clicked();

    void on_load_example_triggered();

    void on_actionSSAA_triggered(bool);

    void on_actionMSAA_triggered(bool);

    void on_actionFXAA_triggered(bool);

    void on_pipeline_cull_facecull_triggered();

    void on_pipeline_cull_viewfrustumcull_triggered();

    void on_pipeline_cull_viewportcull_triggered();

    void on_setting_multithread_triggered();

    void on_ambient_color_clicked();

    void on_diffuse_color_clicked();

    void on_specular_color_clicked();

    void on_camera_persp_clicked();

    void on_camera_ortho_clicked();

    void on_camera_reset_clicked();

    void on_Delete_clicked();

    void on_delete_cubemap_clicked();

    void when_combobox_fragment_shader_activated(int);

    void when_combobox_render_type_activated(int);

    void when_vertical_slider_persp_camera_fov_sliderMoved(int);

    void when_vertical_slider_persp_camera_near_sliderMoved(int);

    void when_vertical_slider_persp_camera_ratio_sliderMoved(int);

    void when_vertical_slider_ortho_camera_width_sliderMoved(int);

    void when_vertical_slider_ortho_camera_height_sliderMoved(int);

    void when_dial_lightpos_sliderMoved(int);

    void when_horizontal_slider_lightposz_sliderMoved(int);

    void when_dial_camera_anglex_sliderMoved(int);

    void when_dial_camera_angley_sliderMoved(int);

    void when_dial_camera_anglez_sliderMoved(int);

    void on_subdivision_level_sliderMoved(int);

    void test();
};
