#include "calculator.h"

#include <functional>

#include <QTextStream>
#include <QLocale>
#include <QFile>
#include <QDebug>

struct group1
{
    double a, b, c;
};

double get_a(const group1& g)
{
    return g.a;
}

double get_b(const group1& g)
{
    return g.b;
}

double get_c(const group1& g)
{
    return g.c;
}

double average(const QList<group1>& g, const std::function<double(const group1& g)> &f)
{
    int s = g.size();
    double sum = 0;
    foreach(const group1 &gg, g)
    {
        sum += f(gg);
    }
    return sum/s;
}

Calculator::Calculator(QObject *parent) : QObject(parent)
{

}

void Calculator::setData(const QString &file_in_, const QString &file_out_, double from_, double to_, double avg_)
{
    file_in = file_in_;
    file_out = file_out_;
    from = from_;
    to = to_;
    avg = avg_;

}

void Calculator::start()
{
    T.clear();
    U.clear();
    I.clear();

    QFile file(file_in);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit done();
        return;
    }

    QFile file_output(file_out);
    if (!file_output.open(QIODevice::WriteOnly))
    {
        emit done();
        return;
    }

    QTextStream out(&file_output);
    out.setLocale(QLocale(QLocale::Ukrainian, QLocale::Ukraine));
    out.setRealNumberPrecision(8);

    QTextStream file_stream(&file);
    QString buf = file_stream.readLine().trimmed();
    if (buf.isEmpty())
    {
        buf = file_stream.readLine().trimmed();
    }

    QList<group1> list1;

    int count = 0;

    while(!buf.isEmpty())
    {
        count++;
        emit progress(count, 30000);


        QTextStream s1(&buf);
        group1 tmp;
        s1 >> tmp.a >> tmp.b >> tmp.c;

        list1.push_front(tmp);

        if (tmp.a >= from && tmp.a <= to)
        {
            double
                    t = average(list1, get_a),
                    u = average(list1, get_b),
                    i = std::abs(average(list1, get_c));
            out
                    << t
                    << "\t"
                    << u
                    << "\t"
                    << i
                    << "\r\n";
            T << t;
            U << u;
            I << i;
        }

        if (list1.size() >= avg)
        {
            list1.pop_back();
        }

        buf = file_stream.readLine().trimmed();
    }
    out.flush();
    emit done();
}
