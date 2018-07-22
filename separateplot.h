#ifndef SEPARATEPLOT_H
#define SEPARATEPLOT_H

#include <QWidget>

namespace Ui {
class SeparatePlot;
}

class SeparatePlot : public QWidget
{
    Q_OBJECT

public:
    explicit SeparatePlot(QWidget *parent = nullptr);
    ~SeparatePlot();

    double plotSelectionLeft() const
    {
        return plot_selection_left;
    }

    double plotSelectionRight() const
    {
        return plot_selection_right;
    }

signals:
    void plotSelectionLeftChanged(double value);
    void plotSelectionRightChanged(double value);

public slots:
    void rescale();

    void setIScale(double scale) {
        i_scale = scale;
    }
    void setUScale(double scale) {
        u_scale = scale;
    }

    void setAxisLabels(const QString &x_axis_label, const QString &y_axis_label);
    void setAxisLabels(const QString &x_axis_label, const QString &y_axis_label, const QString &y2_axis_label);

    void set_x_axis_density(int value);

    void clearAll();
    void replot();

    void showSelectorSlider();
    void hideSelectorSlider();

    void setPlotData(int plot, const QVector<double> &x, const QVector<double> &y);
    void showPlotAsDots(int plot);

    void setYAxisScale(double scale);

    void printScr(double image_part);

private slots:
    void onDoubleSliderMoved(int val);
    void onDoubleSliderAltMoved(int val);

    void on_verticalSlider_valueChanged(int value);

    void on_horizontalSlider_valueChanged(int value);

    void on_btnRescale_clicked();

private:
    Ui::SeparatePlot *ui;

    int old_y_slider_scale = 100;
    int old_x_slider_scale = 100;

    double i_scale = 1;
    double u_scale = 1;

    double plot_selection_left;
    double plot_selection_right;
};

#endif // SEPARATEPLOT_H
