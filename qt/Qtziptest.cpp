#include <QtCore/QCoreApplication>
#include <QFile>
#include <QByteArray>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QFile inFile("in.txt");
    inFile.open(QIODevice::ReadOnly);
    QByteArray ba1 = inFile.readAll();
    qDebug() << ba1.size();
    QByteArray ba2 = qCompress(ba1,9);
    qDebug() << ba2.size();
    QByteArray ba3 = qUncompress(ba2);
    qDebug() << ba3.size();

    ba2.remove(0,4);

    QFile outFile("out.gz");
    outFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    outFile.write(ba2);

    //a.exit(0);
    return a.exec();
}
