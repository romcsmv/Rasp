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
    ui->plot->addGraph();
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(QColor("red")));
    ui->plot->xAxis->setAutoTickCount(30);

    ui->plot->setInteraction(QCP::iRangeDrag);
    ui->plot->setInteraction(QCP::iRangeZoom);

    connect(this, SIGNAL(beginCalc()), calculator, SLOT(start()));
    connect(calculator, SIGNAL(progress(int,int)), this, SLOT(onProgress(int,int)));
    connect(calculator, SIGNAL(done()), this, SLOT(onDone()));
    connect(ui->btnRescale, SIGNAL(released()), this, SLOT(rescale()));

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
        if (!file_out.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, ui->output_filename->text(), "Не можу відкрити вихідний файл.", QMessageBox::Ok);
            return;
        }
    }

    ui->groupBox->setEnabled(false);
    ui->progressBar->setValue(0);
    ui->progressBar->show();
    calculator->setData(ui->input_filename->text(),
                        ui->output_filename->text(),
                        ui->spin_from->value(),
                        ui->spin_to->value(),
                        ui->spin_average->value());
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
    QMessageBox::information(this, qApp->applicationName(), "Пораховано!", QMessageBox::Ok);
    ui->progressBar->hide();
}

void Window::rescale()
{
    ui->verticalSlider->blockSignals(true);
    old_slider_scale = 100;
    ui->verticalSlider->setValue(100);
    ui->verticalSlider->blockSignals(false);

    ui->plot->rescaleAxes();
    ui->plot->replot();
}

void Window::on_verticalSlider_valueChanged(int value)
{
    QCPRange range = ui->plot->yAxis->range();

    double real_size = range.size() / old_slider_scale;
    double new_size = real_size * value;

    old_slider_scale = value;

    ui->plot->yAxis->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->yAxis2->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
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
        if (!file_dialog->selectedFiles().size() > 0)
        {
            return;
        }
        QFileInfo selected(file_dialog->selectedFiles().first());
        if (!selected.exists())
        {
            return;
        }
        ui->input_filename->setText(selected.absoluteFilePath());

        QString out("%1/%2_out.txt");
        ui->output_filename->setText(QDir::cleanPath(out
                                                     .arg(selected.path())
                                                     .arg(selected.baseName())));
    }
}
