#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QString>
#include <QVector>

class Calculator : public QObject
{
    Q_OBJECT
public:
    explicit Calculator(QObject *parent = 0);

    void setData(const QString &file_in_,
                 const QString &file_out_,
                 double from_,
                 double to_,
                 double avg_);

    const QVector<double> &getT() const { return T; }
    const QVector<double> &getU() const { return U; }
    const QVector<double> &getI() const { return I; }

signals:
    void progress(int current, int max);
    void done();

public slots:
    void start();

private:
    QString file_in;
    QString file_out;
    double from;
    double to;
    double avg;
    QVector<double> T, U, I;
};

#endif // CALCULATOR_H
