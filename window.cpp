#include "window.h"
#include "ui_window.h"

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QFileDevice>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPixmap>
#include <QStandardPaths>
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

    ui->plot->setAxisLabels("t", "U", "I");
    ui->plot->set_x_axis_density(ui->x_axis_density->value());
    connect(ui->x_axis_density, &QSlider::valueChanged, ui->plot, &SeparatePlot::set_x_axis_density);

    connect(ui->plot, &SeparatePlot::plotSelectionLeftChanged, this, &Window::onPlotSelectionLeft);
    connect(ui->plot, &SeparatePlot::plotSelectionRightChanged, this, &Window::onPlotSelectionRight);

    connect(this, SIGNAL(beginCalc()), calculator, SLOT(start()));
    connect(calculator, SIGNAL(progress(int,int)), this, SLOT(onProgress(int,int)));
    connect(calculator, SIGNAL(done()), this, SLOT(onDone()));
    connect(ui->allow_save_to_file, &QCheckBox::toggled, ui->output_filename, &QLineEdit::setEnabled);
    connect(ui->allow_save_to_file, &QCheckBox::toggled, calculator, &Calculator::setAllowOutToFile);


    connect(ui->chck_dispersion_select, &QCheckBox::toggled, this, [this] (bool checked) {
        ui->chck_range_select->blockSignals(true);
        ui->chck_range_select->setChecked(false);
        ui->chck_range_select->blockSignals(false);

        if (checked)
        {
            select_from = ui->spin_dispersion_from;
            select_to = ui->spin_dispersion_to;
            ui->plot->showSelectorSlider();
        }
        else
        {
            ui->plot->hideSelectorSlider();
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
            ui->plot->showSelectorSlider();
        }
        else
        {
            ui->plot->hideSelectorSlider();
        }
    });

    ui->chck_dispersion_select->setChecked(true);

    ui->allow_save_to_file->setChecked(false);
    ui->progressBar->hide();
}

Window::~Window()
{
    delete ui_plot;

    {
        QEventLoop loop;

        connect(calculator, SIGNAL(destroyed(QObject*)), calc_thread, SLOT(quit()));
        connect(calc_thread, SIGNAL(finished()), &loop, SLOT(quit()));
        QMetaObject::invokeMethod(calculator, "deleteLater", Qt::QueuedConnection);
        loop.exec();
    }

    delete ui;
}

void Window::onPlotSelectionLeft(double value)
{
    select_from->setValue(value);
}

void Window::onPlotSelectionRight(double value)
{
    select_to->setValue(value);
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
    ui->plot->replot();

    ui->groupBox->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->show();
    calculator->setData(ui->input_filename->text(),
                        ui->output_filename->text(),
                        ui->spin_from->value(),
                        ui->spin_to->value(),
                        ui->spin_average->value());
    calculator->u_coef = ui->spin_u_coef->value();
    calculator->i_coef = ui->spin_i_coef->value();
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
    ui->progressBar->setValue(int(double(current)/max*100.));
    ui->progressBar->update();
}

void Window::onDone()
{
    ui->plot->setPlotData(0, calculator->getT(), calculator->getU());
    ui->plot->setPlotData(1, calculator->getT(), calculator->getI());

    ui->plot->rescale();

    ui->groupBox->setEnabled(true);
    ui->progressBar->setValue(100);
    ui->progressBar->hide();
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
    ui->plot->setPlotData(4, array_t, u_curve);
    ui->plot->setPlotData(5, array_t, i_curve);

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

void Window::clearAll()
{
    ui->plot->clearAll();
    ui->spin_dispersion_from->setValue(ui->spin_from->value());
    ui->spin_dispersion_to->setValue(ui->spin_to->value());
    ui->lb_dispersion->setText("");
    ui->text_dispersions->clear();
}

void Window::on_btn_scale_clicked()
{
    ui->plot->setYAxisScale(ui->spin_set_scale->value());
}

void Window::on_btn_prt_scr_clicked()
{
    ui->plot->printScr(ui->spin_prt_scr_part->value());
}

void Window::on_btn_plot_iu_clicked()
{
    if (!ui_plot)
    {
        ui_plot = new SeparatePlot();
        ui_plot->setAxisLabels("U", "I");
        ui_plot->set_x_axis_density(ui->x_axis_density->value());
        ui_plot->showPlotAsDots(0);
    }

    ui_plot->clearAll();
    ui_plot->setPlotData(0, calculator->getU(), calculator->getI());
    ui_plot->rescale();
    ui_plot->show();
    ui_plot->raise();
}
