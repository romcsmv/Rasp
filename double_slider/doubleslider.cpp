#include "doubleslider.h"

#include <QMouseEvent>
#include <QProxyStyle>

class SliderProxy : public QProxyStyle
{
public:
  int pixelMetric ( PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const
  {
    switch(metric) {
    case PM_SliderThickness  : return 24;
    case PM_SliderLength     : return 24;
    default                  : return (QProxyStyle::pixelMetric(metric,option,widget));
    }
  }
};

DoubleSlider::DoubleSlider(QWidget *parent)
  : QSlider(parent)
  , alt_handle(new SecondSliderHandle(this))
{
  setOrientation(Qt::Horizontal);
  setStyleSheet("QSlider::handle { image: url(:/handle.png); }");

  SliderProxy *aSliderProxy = new SliderProxy();
  setStyle(aSliderProxy);

  setAltValue(maximum());

  connect(alt_handle, &SecondSliderHandle::valueChanged, this, &DoubleSlider::altValueChanged);

  connect(this, &DoubleSlider::valueChanged, [this] (int new_value) {
      bool move_left = (new_value == maximum());

      if (new_value >= altValue())
      {
          setAltValue(new_value + 1);
      }
      if (move_left)
      {
          QMetaObject::invokeMethod(this, "setValue", Qt::QueuedConnection, Q_ARG(int, maximum()-2));
      }
  });
}

int DoubleSlider::altValue()
{
    return alt_handle->value();
}

SecondSliderHandle::SecondSliderHandle(DoubleSlider *parent)
  : QLabel(parent)
{
    slider = parent;
    _value = slider->maximum();

    resize(18, 18);
    QPixmap pix = QPixmap(":/handle.png").scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setPixmap(pix);

    connect(slider, &QSlider::rangeChanged, [this] (int, int) {
       setValue(_value);
    });

    parent->installEventFilter(this);
}

void DoubleSlider::setAltValue(int new_value)
{
    alt_handle->setValue(new_value);
}

int SecondSliderHandle::value()
{
    return _value;
}

void SecondSliderHandle::setValue(int new_value)
{
    if (new_value <= slider->minimum())
    {
        new_value = slider->minimum() + 1;
    }
    if (new_value >= slider->maximum())
    {
        new_value = slider->maximum();
    }

    if (new_value <= slider->value())
    {
        slider->setValue(new_value - 1);
    }

    if (new_value != _value)
    {
        _value = new_value;
        emit valueChanged(_value);
    }
    adjustPos();
}

void SecondSliderHandle::adjustPos()
{
    double w = width();
    double left_side = w;
    double right_side = (slider->width() - w/4);
    int x_pos = left_side + double(_value) / (slider->maximum() - slider->minimum()) * (right_side - left_side);
    move(x_pos - w/2, 0);
}

bool SecondSliderHandle::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonRelease:
        mousePressed = false;
        break;
    case QEvent::MouseButtonPress:
    {
        slider->clearFocus();
        mousePressed = true;
        // no break here
    }
    case QEvent::MouseMove:
    {
        if (!mousePressed)
        {
            break;
        }
        int x = slider->mapFromGlobal(QCursor::pos()).x();

        double w = width();
        double left_side = w;
        double right_side = (slider->width() - w/4);
        if (x >= left_side && x <= right_side)
        {
            int new_val = double(x - left_side) / (right_side - left_side) * (slider->maximum() - slider->minimum());
            setValue(new_val);
        }

        return true;
    }
    default:
        break;
    }
    return QLabel::event(e);
}

bool SecondSliderHandle::eventFilter(QObject *o, QEvent *e)
{
    bool res = QLabel::eventFilter(o, e);
    if (e->type() == QEvent::Resize)
    {
         adjustPos();
    }
    return res;
}
