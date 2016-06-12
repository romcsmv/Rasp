#include "window.h"
#include "ui_window.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>
#include <QEventLoop>
#include <QFileInfo>
#include <QThread>

#include "calculator.h"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Window)
    , calc_thread(new QThread(this))
    , calculator(new Calculator())
{
    calculator->moveToThread(calc_thread);
    calc_thread->start();

    ui->setupUi(this);
    for (int i=0; i < 6; i++)
    {
        ui->plot->addGraph();
    }

    ui->plot->graph(1)->setPen(QPen(QColor("blue")));
    ui->plot->graph(1)->setPen(QPen(QColor("red")));
    ui->plot->xAxis->setAutoTickCount(ui->x_axis_density->value());

    ui->plot->graph(2)->setPen(QPen(QColor("green")));
    ui->plot->graph(3)->setPen(QPen(QColor("green")));

    ui->plot->graph(4)->setPen(QPen(QBrush(QColor("lime")), 2));
    ui->plot->graph(5)->setPen(QPen(QBrush(QColor("orange")), 2));

    ui->plot->setInteraction(QCP::iRangeDrag);
    ui->plot->setInteraction(QCP::iRangeZoom);

    connect(this, SIGNAL(beginCalc()), calculator, SLOT(start()));
    connect(calculator, SIGNAL(progress(int,int)), this, SLOT(onProgress(int,int)));
    connect(calculator, SIGNAL(done()), this, SLOT(onDone()));
    connect(ui->btnRescale, SIGNAL(released()), this, SLOT(rescale()));
    connect(ui->allow_save_to_file, &QCheckBox::toggled, ui->output_filename, &QLineEdit::setEnabled);
    connect(ui->allow_save_to_file, &QCheckBox::toggled, calculator, &Calculator::setAllowOutToFile);

    connect(ui->slider_double, &DoubleSlider::valueChanged, this, &Window::onDoubleSliderMoved);
    connect(ui->slider_double, &DoubleSlider::altValueChanged, this, &Window::onDoubleSliderAltMoved);

    connect(ui->chck_dispersion_select, &QCheckBox::toggled, this, [this] (bool checked) {
        ui->chck_range_select->blockSignals(true);
        ui->chck_range_select->setChecked(false);
        ui->chck_range_select->blockSignals(false);

        if (checked)
        {
            select_from = ui->spin_dispersion_from;
            select_to = ui->spin_dispersion_to;
            ui->slider_double->show();
        }
        else
        {
            ui->slider_double->hide();
        }
    });

    connect(ui->chck_range_select, &QCheckBox::toggled, this, [this] (bool checked) {
        ui->chck_dispersion_select->blockSignals(true);
        ui->chck_dispersion_select->setChecked(false);
        ui->chck_dispersion_select->blockSignals(false);

        if (checked)
        {
            select_from = ui->spin_from;
            select_to = ui->spin_to;
            ui->slider_double->show();
        }
        else
        {
            ui->slider_double->hide();
        }
    });

    ui->chck_dispersion_select->setChecked(true);

    ui->allow_save_to_file->setChecked(false);
    ui->btnRescale->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
    ui->progressBar->hide();
}

Window::~Window()
{
    QEventLoop loop;

    connect(calculator, SIGNAL(destroyed(QObject*)), calc_thread, SLOT(quit()));
    connect(calc_thread, SIGNAL(finished()), &loop, SLOT(quit()));
    QMetaObject::invokeMethod(calculator, "deleteLater", Qt::QueuedConnection);
    loop.exec();

    delete ui;
}


void Window::on_btnStart_clicked()
{
    {
        QFile file(ui->input_filename->text());
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, ui->input_filename->text(), "Не можу відкрити вхідний файл.", QMessageBox::Ok);
            return;
        }

        QFile file_out(ui->output_filename->text());
        if (ui->allow_save_to_file->isChecked() && !file_out.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, ui->output_filename->text(), "Не можу відкрити вихідний файл.", QMessageBox::Ok);
            return;
        }
    }

    clearAll();

    ui->groupBox->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->show();
    calculator->setData(ui->input_filename->text(),
                        ui->output_filename->text(),
                        ui->spin_from->value(),
                        ui->spin_to->value(),
                        ui->spin_average->value());
    calculator->setInvertPolicy(ui->invert_1->isChecked() && ui->invertion->isChecked(), ui->invert_2->isChecked() && ui->invertion->isChecked());
    emit beginCalc();
}

void Window::onProgress(int current, int max)
{
    while (current > max || current < 0)
    {
        if (current > max)
        {
            current -= max;
        }

        if (current < 0)
        {
            current += max;
        }
    }
    ui->progressBar->setValue(double(current)/max*100);
    ui->progressBar->update();
}

void Window::onDone()
{
    ui->plot->graph(0)->setData(calculator->getT(), calculator->getU());
    ui->plot->graph(1)->setData(calculator->getT(), calculator->getI());

    rescale();

    ui->groupBox->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->hide();
}

void Window::rescale()
{
    ui->verticalSlider->blockSignals(true);
    ui->horizontalSlider->blockSignals(true);
    old_y_slider_scale = 100;
    old_x_slider_scale = 100;
    ui->verticalSlider->setValue(100);
    ui->horizontalSlider->setValue(100);
    ui->verticalSlider->blockSignals(false);
    ui->horizontalSlider->blockSignals(false);

    ui->plot->rescaleAxes();
    ui->plot->replot();
}

void Window::on_verticalSlider_valueChanged(int value)
{
    QCPRange range = ui->plot->yAxis->range();

    double real_size = range.size() / old_y_slider_scale;
    double new_size = real_size * value;

    old_y_slider_scale = value;

    ui->plot->yAxis->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->yAxis2->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->replot();
}

void Window::on_horizontalSlider_valueChanged(int value)
{
    QCPRange range = ui->plot->xAxis->range();

    double real_size = range.size() / old_x_slider_scale;
    double new_size = real_size * value;

    old_x_slider_scale = value;

    ui->plot->xAxis->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->xAxis2->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->replot();
}

void Window::on_fileOpenButton_clicked()
{
    static QFileDialog *file_dialog = nullptr;
    if (!file_dialog)
    {
        file_dialog = new QFileDialog(this, "File open", "D:/");
        file_dialog->setFileMode(QFileDialog::ExistingFile);
        file_dialog->setFilter(QDir::Files | QDir::NoDot | QDir::NoDotDot);
        file_dialog->setNameFilters(QStringList() << "*.txt");
    }
    if (file_dialog->exec())
    {
        if (!(file_dialog->selectedFiles().size() > 0))
        {
            return;
        }
        QFileInfo selected(file_dialog->selectedFiles().first());
        if (!selected.exists())
        {
            return;
        }
        ui->input_filename->setText(selected.absoluteFilePath());

        ui->plot->graph(0)->clearData();
        ui->spin_from->setValue(0);
        ui->spin_to->setValue(100);
        clearAll();
        ui->plot->replot();

        QString out("%1/%2_out.txt");
        ui->output_filename->setText(QDir::cleanPath(out
                                                     .arg(selected.path())
                                                     .arg(selected.baseName())));
    }
}

void Window::on_x_axis_density_valueChanged(int value)
{
    ui->plot->xAxis->setAutoTickCount(value);
    ui->plot->replot();
}

void Window::on_btn_dispersion_clicked()
{
    QVector<double> array_u, array_i, array_t;
    QVector<double> u_curve, i_curve;

    double from = ui->spin_dispersion_from->value();
    double to = ui->spin_dispersion_to->value();

    const QVector<double> &T = calculator->getT();
    const QVector<double> &U = calculator->getU();
    const QVector<double> &I = calculator->getI();
    for (int i = 0; i < T.size(); i++)
    {
        if (T[i] >= from && T[i] <= to)
        {
            array_u.push_back(U[i]);
            array_i.push_back(I[i]);
            array_t.push_back(T[i]);
        }
    }
    auto n = array_t.size();

    double a_i, a_u, b_i, b_u;
    {
        double sumx = 0;
        double sumy_u = 0;
        double sumy_i = 0;
        double sumx2 = 0;
        double sumxy_u = 0;
        double sumxy_i = 0;
        for(int i = 0; i < n; i++) {
          sumx += array_t[i];
          sumy_u += array_u[i];
          sumy_i += array_i[i];
          sumx2 += array_t[i]*array_t[i];
          sumxy_u += array_t[i]*array_u[i];
          sumxy_i += array_t[i]*array_i[i];
        }
        a_u = (n*sumxy_u - (sumx*sumy_u))/(n*sumx2-sumx*sumx);
        b_u = (sumy_u - a_u*sumx)/n;

        a_i = (n*sumxy_i - (sumx*sumy_i))/(n*sumx2-sumx*sumx);
        b_i = (sumy_i - a_i*sumx)/n;
    }

    ui->text_dispersions->clear();
    auto u_aver = [a=a_u, b=b_u] (double x) { return a * x + b; };
    auto i_aver = [a=a_i, b=b_i] (double x) { return a * x + b; };
    ui->text_dispersions->setPlainText(QString("Average U: %1 * x + %2 \n Average I: %3 * x + %4")
                                       .arg(a_u, 8, 'f', 5)
                                       .arg(b_u, 8, 'f', 5)
                                       .arg(a_i, 8, 'f', 5)
                                       .arg(b_i, 8, 'f', 5));

    for(int i = 0; i < n; i++)
    {
        u_curve.push_back(u_aver(array_t[i]));
        i_curve.push_back(i_aver(array_t[i]));
    }
    ui->plot->graph(4)->setData(array_t, u_curve);
    ui->plot->graph(5)->setData(array_t, i_curve);

    double dispersion_u = 0;
    double dispersion_i = 0;
    for(auto i = 0; i < n; i++)
    {
        auto val_u = array_u[i];
        auto val_i = array_i[i];
        double sub_u = val_u - u_aver(array_t[i]);
        double sub_i = val_i - i_aver(array_t[i]);
        dispersion_u += sub_u * sub_u;
        dispersion_i += sub_i * sub_i;
    }
    dispersion_u /= n;
    dispersion_i /= n;

    ui->lb_dispersion->setText(QString("U: %1; I: %2;")
                               .arg(dispersion_u, 0, 'f', 5)
                               .arg(dispersion_i, 0, 'f', 5));

    ui->plot->replot();
}

void Window::onDoubleSliderMoved(int val)
{
    QCPRange x_range = ui->plot->xAxis->range();
    QCPRange y_range = ui->plot->yAxis->range();

    double line_pos = x_range.lower + (x_range.size() * val) / (ui->slider_double->maximum() - ui->slider_double->minimum());
    select_from->setValue(line_pos);

    QVector<double> keys, values;
    keys << line_pos << line_pos;
    values << y_range.lower << y_range.upper;
    ui->plot->graph(2)->setData(keys, values);

    ui->plot->replot();
}

void Window::onDoubleSliderAltMoved(int val)
{
    QCPRange x_range = ui->plot->xAxis->range();
    QCPRange y_range = ui->plot->yAxis->range();

    double line_pos = x_range.lower + (x_range.size() * val) / (ui->slider_double->maximum() - ui->slider_double->minimum());
    select_to->setValue(line_pos);

    QVector<double> keys, values;
    keys << line_pos << line_pos;
    values << y_range.lower << y_range.upper;
    ui->plot->graph(3)->setData(keys, values);

    ui->plot->replot();
}

void Window::clearAll()
{
    for (int i = 0; i < ui->plot->graphCount(); i++)
    {
        ui->plot->graph(i)->clearData();
        ui->spin_dispersion_from->setValue(ui->spin_from->value());
        ui->spin_dispersion_to->setValue(ui->spin_to->value());
        ui->lb_dispersion->setText("");
        ui->text_dispersions->clear();
    }
}
