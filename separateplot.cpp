#include "separateplot.h"
#include "ui_separateplot.h"

SeparatePlot::SeparatePlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SeparatePlot)
{
    ui->setupUi(this);

    {
        auto font = ui->plot->font();
        font.setPointSize(18);
        ui->plot->setFont(font);
        ui->plot->xAxis->setTickLabelFont(font);
        ui->plot->yAxis->setTickLabelFont(font);
        ui->plot->yAxis2->setTickLabelFont(font);
        font.setItalic(true);
        ui->plot->xAxis->setLabelFont(font);
        ui->plot->yAxis->setLabelFont(font);
        ui->plot->yAxis2->setLabelFont(font);
    }
    ui->plot->addGraph();
    ui->plot->addGraph(ui->plot->xAxis, ui->plot->yAxis2);

    connect(ui->plot->yAxis, static_cast<void(QCPAxis::*)(const QCPRange &, const QCPRange &)>(&QCPAxis::rangeChanged),  [this] (const QCPRange &newRange, const QCPRange &oldRange)
    {
        double scale_coef = ui->plot->yAxis2->range().size() / oldRange.size();

        double d_size = newRange.size() / oldRange.size();
        double d_center = (newRange.center() - oldRange.center()) * scale_coef;

        double new_size = ui->plot->yAxis2->range().size() * d_size;
        double new_center = ui->plot->yAxis2->range().center() + d_center;

        ui->plot->yAxis2->setRange(new_center - new_size/2, new_center + new_size/2);
    });

    for (int i=2; i < 5; i++)
    {
        ui->plot->addGraph();
    }
    ui->plot->addGraph(ui->plot->xAxis, ui->plot->yAxis2);

    ui->plot->graph(1)->setPen(QPen(QColor("blue")));
    ui->plot->graph(1)->setPen(QPen(QColor("red")));

    ui->plot->graph(2)->setPen(QPen(QColor("green")));
    ui->plot->graph(3)->setPen(QPen(QColor("green")));

    ui->plot->graph(4)->setPen(QPen(QBrush(QColor("lime")), 2));
    ui->plot->graph(5)->setPen(QPen(QBrush(QColor("orange")), 2));

    ui->plot->setInteraction(QCP::iRangeDrag);
    ui->plot->setInteraction(QCP::iRangeZoom);

    connect(ui->slider_double, &DoubleSlider::valueChanged, this, &SeparatePlot::onDoubleSliderMoved);
    connect(ui->slider_double, &DoubleSlider::altValueChanged, this, &SeparatePlot::onDoubleSliderAltMoved);

    ui->btnRescale->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
}

SeparatePlot::~SeparatePlot()
{
    delete ui;
}

void SeparatePlot::rescale()
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
    {
        auto range_u = ui->plot->yAxis->range();
        range_u.lower = std::min(range_u.lower, ui->plot->yAxis2->range().lower * u_scale / i_scale);
        auto range_i = range_u;
        {
            range_i.lower *= i_scale/u_scale;
            range_i.upper *= i_scale/u_scale;
        }

        ui->plot->yAxis->setRange(range_u);
        ui->plot->yAxis2->setRange(range_i);
    }

    ui->plot->replot();
}

void SeparatePlot::setAxisLabels(const QString &x_axis_label, const QString &y_axis_label)
{
    ui->plot->xAxis->setLabel(x_axis_label);
    ui->plot->yAxis->setLabel(y_axis_label);
}

void SeparatePlot::setAxisLabels(const QString &x_axis_label, const QString &y_axis_label, const QString &y2_axis_label)
{
    setAxisLabels(x_axis_label, y_axis_label);
    ui->plot->yAxis2->setVisible(true);
    ui->plot->yAxis2->setLabel(y2_axis_label);
}

void SeparatePlot::set_x_axis_density(int value)
{
    ui->plot->xAxis->setAutoTickCount(value);
    ui->plot->replot();
}

void SeparatePlot::onDoubleSliderMoved(int val)
{
    QCPRange x_range = ui->plot->xAxis->range();
    QCPRange y_range = ui->plot->yAxis->range();

    plot_selection_left = x_range.lower + (x_range.size() * val) / (ui->slider_double->maximum() - ui->slider_double->minimum());
    emit plotSelectionLeftChanged(plot_selection_left);

    QVector<double> keys, values;
    keys << plot_selection_left << plot_selection_left;
    values << y_range.lower << y_range.upper;
    ui->plot->graph(2)->setData(keys, values);

    ui->plot->replot();
}

void SeparatePlot::onDoubleSliderAltMoved(int val)
{
    QCPRange x_range = ui->plot->xAxis->range();
    QCPRange y_range = ui->plot->yAxis->range();

    plot_selection_right = x_range.lower + (x_range.size() * val) / (ui->slider_double->maximum() - ui->slider_double->minimum());
    emit plotSelectionRightChanged(plot_selection_right);

    QVector<double> keys, values;
    keys << plot_selection_right << plot_selection_right;
    values << y_range.lower << y_range.upper;
    ui->plot->graph(3)->setData(keys, values);

    ui->plot->replot();
}

void SeparatePlot::clearAll()
{
    for (int i = 0; i < ui->plot->graphCount(); i++)
        ui->plot->graph(i)->clearData();
}

void SeparatePlot::replot()
{
    ui->plot->replot();
}

void SeparatePlot::showSelectorSlider()
{
    ui->slider_double->show();
}

void SeparatePlot::hideSelectorSlider()
{
    ui->slider_double->hide();
}

void SeparatePlot::setPlotData(int plot, const QVector<double> &x, const QVector<double> &y)
{
    ui->plot->graph(plot)->setData(x, y);
}

void SeparatePlot::showPlotAsDots(int plot)
{
    ui->plot->graph(plot)->setLineStyle(QCPGraph::lsNone);
    ui->plot->graph(plot)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
}

void SeparatePlot::setYAxisScale(double scale)
{
    auto range = ui->plot->yAxis->range();
    range.lower = 0;
    range.upper = scale * 2;
    ui->plot->yAxis->setRange(range);
    ui->plot->replot();
}

void SeparatePlot::printScr(double image_part)
{
    QRect new_rect = ui->plot->rect();
    new_rect.setHeight(int(new_rect.height() * image_part));

    QPixmap pixmap(new_rect.size());
    new_rect.moveTop(ui->plot->height() - new_rect.height());

    ui->plot->render(&pixmap, QPoint(), new_rect);
    static QFileDialog *file_save_dialog = nullptr;
    if (!file_save_dialog)
    {
        file_save_dialog = new QFileDialog(this, tr("Save srint screen As..."), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
        file_save_dialog->setFileMode(QFileDialog::AnyFile);
        file_save_dialog->setOption(QFileDialog::ShowDirsOnly, false);
        file_save_dialog->setOption(QFileDialog::DontResolveSymlinks, true);
        file_save_dialog->setAcceptMode(QFileDialog::AcceptSave);
        file_save_dialog->setLabelText(QFileDialog::Accept, tr("Save"));
        file_save_dialog->setNameFilter("Jpeg files (*.jpg *.jpeg)");
        file_save_dialog->selectFile("Print.jpeg");
    }
    if (file_save_dialog->exec())
    {
        QString filename = file_save_dialog->selectedFiles().first();
        if (QFile::exists(filename))
        {
            QFile::remove(filename);
        }
        QFile save_file(filename);
        if (!save_file.open(QFile::ReadWrite))
            QMessageBox::critical(this, "Error", "Cannot create file!");
        if (!pixmap.save(&save_file, "JPEG"))
            QMessageBox::critical(this, "Error", "Cannot save screen shoot!");
    }
}

void SeparatePlot::on_verticalSlider_valueChanged(int value)
{
    QCPRange range = ui->plot->yAxis->range();

    double real_size = range.size() / old_y_slider_scale;
    double new_size = real_size * value;

    old_y_slider_scale = value;

    ui->plot->yAxis->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->yAxis2->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->replot();
}

void SeparatePlot::on_horizontalSlider_valueChanged(int value)
{
    QCPRange range = ui->plot->xAxis->range();

    double real_size = range.size() / old_x_slider_scale;
    double new_size = real_size * value;

    old_x_slider_scale = value;

    ui->plot->xAxis->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->xAxis2->setRange(QCPRange(range.center() - new_size / 2, range.center() + new_size / 2));
    ui->plot->replot();
}

void SeparatePlot::on_btnRescale_clicked()
{
    rescale();
}
