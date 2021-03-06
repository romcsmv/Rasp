#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QObject>
#include <QString>
#include <QVector>

class Calculator : public QObject
{
    Q_OBJECT
public:
    explicit Calculator(QObject *parent = nullptr);

    void setData(const QString &file_in_,
                 const QString &file_out_,
                 double from_,
                 double to_,
                 double avg_);

    double u_coef = 100;
    double i_coef = 1000;
    const QVector<double> &getT() const { return T; }
    const QVector<double> &getU() const { return U; }
    const QVector<double> &getI() const { return I; }

signals:
    void progress(int current, int max);
    void done();

public slots:
    void start();

    void setAllowOutToFile(bool allow);
    void setInvertPolicy(bool invert_first, bool invert_second);

private:
    void saveToFile(const QString &file_out);

private:
    QString file_in;
    QString file_out;
    double from;
    double to;
    double avg;
    QVector<double> T, U, I;

    bool allow_out_to_file = false;
    struct {
        bool invert_first = false;
        bool invert_second = false;
    } invert_policy;
};

#endif // CALCULATOR_H
