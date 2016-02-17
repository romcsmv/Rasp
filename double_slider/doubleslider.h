#include <QLabel>
#include <QSlider>

class SecondSliderHandle;

class DoubleSlider: public QSlider
{
  Q_OBJECT
  friend class SuperSliderEventFilter;
public:
  DoubleSlider(QWidget *parent = 0);

  int altValue();

public slots:
  void setAltValue(int value);

signals:
  void altValueChanged(int);

private:
  SecondSliderHandle *alt_handle;
};

class SecondSliderHandle: public QLabel
{
    Q_OBJECT
    friend class DoubleSlider;
public:
    SecondSliderHandle(DoubleSlider *parent = 0);

    int value();

signals:
    void valueChanged(int value);

public slots:
    void setValue(int value);

protected:
  bool event(QEvent * e) override;
  bool eventFilter(QObject *o, QEvent *e);

private:
  void adjustPos();

private:
  int _value;

  bool mousePressed = false;

  QSlider *slider;
};
