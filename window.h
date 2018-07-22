#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

namespace Ui {
class Window;
}

class QThread;
class Calculator;
class QDoubleSpinBox;
class SeparatePlot;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

signals:
    void beginCalc();

public slots:
    void onPlotSelectionLeft(double value);
    void onPlotSelectionRight(double value);

private slots:
    void on_btnStart_clicked();
    void on_fileOpenButton_clicked();

    void onProgress(int current, int max);
    void onDone();

    void on_btn_dispersion_clicked();

    void on_btn_scale_clicked();

    void on_btn_prt_scr_clicked();

    void on_btn_plot_iu_clicked();

private:
    void clearAll();

private:
    Ui::Window *ui;
    QThread *calc_thread;
    Calculator *calculator;
    SeparatePlot *ui_plot = nullptr;

    QDoubleSpinBox *select_from = nullptr;
    QDoubleSpinBox *select_to = nullptr;
};

#endif // WINDOW_H
