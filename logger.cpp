#include "logger.h"

Logger::Logger(QString fileName, QObject *parent) : QObject(parent)
{
    QDir dir("out");
    if(!dir.exists())dir.mkdir("out");
    file.setFileName("out/"+fileName+".log");
    //file.remove();
}

Logger::~Logger() {

}

void Logger::Message(QString msg) {
    if(file.open(QFile::ReadWrite|QFile::Append)) {
        QTextStream stream(&file);

        stream<<QDateTime::currentDateTime().toString("yyyy_MM_dd HH:mm:ss_zzz")+" "+msg+"\n";
        file.close();
    }
}
void Logger::Show(QString msg) {
    if(file.open(QFile::ReadWrite|QFile::Append)) {
        QTextStream stream(&file);
        stream<<QDateTime::currentDateTime().toString("yyyy_MM_dd HH:mm:ss_zzz")+" "+msg+"\n";
        file.close();
    }
    emit ShowMessage(msg);
}
void Logger::File(QString fileName, QByteArray data) {
    QFile file;
    fileName = "out/"+QDateTime::currentDateTime().toString("yyyy_MM_dd HH:mm:ss_zzz")+" "+fileName;
    Message("Сохранен файл '"+fileName+"'");
    file.setFileName(fileName);
    if(file.open(QFile::ReadWrite)) {
        file.write(data);
        file.close();
    }
}

void Logger::Post(QString fileName, QString post, QByteArray data) {
    QFile file;
    fileName = "out/"+QDateTime::currentDateTime().toString("yyyy_MM_dd HH:mm:ss_zzz")+" out "+fileName;
    Message("Сохранен файл '"+fileName+"'");
    file.setFileName(fileName);
    if(file.open(QFile::ReadWrite)) {
        file.write(post.toLocal8Bit()+"\n\n");
        file.write(data);
        file.close();
    }
}

void Logger::Recive(QString fileName, QByteArray data) {
    QFile file;
    fileName = "out/"+QDateTime::currentDateTime().toString("yyyy_MM_dd HH:mm:ss_zzz")+" in "+fileName;
    Message("Сохранен файл '"+fileName+"'");
    file.setFileName(fileName);
    if(file.open(QFile::ReadWrite)) {
        file.write(data);
        file.close();
    }
}
void Logger::Data(QString msg) {
    if(file.open(QFile::ReadWrite|QFile::Append)) {
        QTextStream stream(&file);
        stream<<msg+"\n";
        file.close();
    }
}

QList<int> Logger::LastGifts() {
    QList<int> list;
    list.clear();
    QStringList sl;
    if(file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        do {
            QString str=stream.readLine();
            sl=str.split('|');
            if(sl.count()>=7) list.append(sl[1].toInt());
            if(list.count()>300) list.removeAt(0);

        }while(!stream.atEnd());

        file.close();
    }
    return list;
}
