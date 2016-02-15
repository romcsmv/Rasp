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

private:
    Ui::Window *ui;
    QThread *calc_thread;
    Calculator *calculator;

    int old_slider_scale = 100;
};

#endif // WINDOW_H
