#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

namespace Ui {
class Window;
}

class QThread;
class Calculator;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

signals:
    void beginCalc();

private slots:
    void on_btnStart_clicked();
    void on_fileOpenButton_clicked();
    void on_verticalSlider_valueChanged(int value);

    void onProgress(int current, int max);
    void onDone();
    void rescale();

    void on_horizontalSlider_valueChanged(int value);

    void on_x_axis_density_valueChanged(int value);

    void on_btn_dispersion_clicked();

private:
    Ui::Window *ui;
    QThread *calc_thread;
    Calculator *calculator;

    int old_y_slider_scale = 100;
    int old_x_slider_scale = 100;
};

#endif // WINDOW_H
