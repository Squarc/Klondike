#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDateTime>
#include <QDir>

class Logger: public QObject
{
    Q_OBJECT
public:
    explicit Logger(QString fileName,QObject *parent = 0);
    ~Logger();
    void Message(QString msg);
    void Data(QString msg);
    void Show(QString msg);
    void File(QString fileName, QByteArray data);
    void Post(QString fileName, QString post, QByteArray data=QByteArray());
    void Recive(QString fileName, QByteArray data);
    QList<int> LastGifts();
    QFile file;
private:
signals:
    void ShowMessage(QString msg);
};

#endif // LOGGER_H
