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

    QStringList lines;
    {
        QTextStream file_stream(&file);
        QString buf = file_stream.readAll();
        lines = buf.split('\n');
    }

    QList<group1> list1;

    int count = 0;


    std::function<double(double)> first_changer, second_changer;
    {
        auto invertor = [] (double x) { return -x; };
        auto noop = [] (double x) { return x; };
        if (invert_policy.invert_first)
        {
            first_changer = invertor;
        }
        else
        {
            first_changer = noop;
        }

        if (invert_policy.invert_second)
        {
            second_changer = invertor;
        }
        else
        {
            second_changer = noop;
        }
    }

    foreach(QString buf, lines)
    {
        count++;
        emit progress(count, lines.size());

        QTextStream s1(&buf);
        group1 tmp;
        s1 >> tmp.a >> tmp.b >> tmp.c;

        tmp.b *= u_coef;
        tmp.c *= i_coef;

        list1.push_front(tmp);

        if (tmp.a >= from && tmp.a <= to)
        {
            double
                    t = average(list1, get_a),
                    u = first_changer(average(list1, get_b)),
                    i = second_changer(average(list1, get_c));
            T << t;
            U << u;
            I << i;
        }

        if (list1.size() >= avg)
        {
            list1.pop_back();
        }
    }

    if (allow_out_to_file)
    {
        saveToFile(file_out);
    }

    emit done();
}

void Calculator::setAllowOutToFile(bool allow)
{
    allow_out_to_file = allow;
}

void Calculator::setInvertPolicy(bool invert_first, bool invert_second)
{
    invert_policy.invert_first = invert_first;
    invert_policy.invert_second = invert_second;
}

void Calculator::saveToFile(const QString &file_out)
{
    QFile file_output(file_out);
    if (!file_output.open(QIODevice::WriteOnly))
    {
        return;
    }

    QTextStream out(&file_output);
    out.setLocale(QLocale(QLocale::Ukrainian, QLocale::Ukraine));
    out.setRealNumberPrecision(8);

    for (int i = 0; i < T.size() && i < U.size() && i < I.size(); i++)
    {
        out
                << T[i]
                << "\t"
                << U[i]
                << "\t"
                << I[i]
                << "\r\n";
    }
    out.flush();
}
