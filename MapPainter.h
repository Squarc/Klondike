#ifndef MAPPAINTER_H
#define MAPPAINTER_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QPainter>
#include <QMouseEvent>

class MapPainter : public QWidget
{
    Q_OBJECT
public:
    explicit MapPainter(QString mapName, QWidget *parent = 0);
    int x,y;
    QByteArray arr;
    QString mapName;
protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent* event);

};

#endif // MAPPAINTER_H
