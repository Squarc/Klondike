#include "MapPainter.h"

MapPainter::MapPainter(QString mapName, QWidget *parent) : QWidget(parent)
{
    //setAcceptHoverEvents(true);
    //setAcceptedMouseButtons(Qt::AllButtons);
    //setFlag(ItemAcceptsInputMethod, true);
    this->mapName=mapName;
    QFile map("./klonevk-res/"+mapName+".bin");
    map.open(QFile::ReadOnly);
    arr=map.readAll();
    map.close();
    int value;
    int pos=0;
    int x=0,y=0;
    x=((uint)(unsigned char)arr[pos]<<24)+((uint)(unsigned char)arr[pos+1]<<16)+((uint)(unsigned char)arr[pos+2]<<8)+(uint)(unsigned char)arr[pos+3];
    pos+=4;
    y=((uint)(unsigned char)arr[pos]<<24)+((uint)(unsigned char)arr[pos+1]<<16)+((uint)(unsigned char)arr[pos+2]<<8)+(uint)(unsigned char)arr[pos+3];
    pos+=4;
    this->setMinimumSize(x*5,y*5);
    this->setMaximumSize(x*5,y*5);

}

void MapPainter::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setBackground(QBrush(Qt::white));
    p.setPen(QPen(Qt::black,1,Qt::SolidLine));
    p.setBrush(QBrush(Qt::white));
    int value;
    int pos=0;
    int x=((uint)(unsigned char)arr[pos]<<24)+((uint)(unsigned char)arr[pos+1]<<16)+((uint)(unsigned char)arr[pos+2]<<8)+(uint)(unsigned char)arr[pos+3];
    pos+=4;
    int y=((uint)(unsigned char)arr[pos]<<24)+((uint)(unsigned char)arr[pos+1]<<16)+((uint)(unsigned char)arr[pos+2]<<8)+(uint)(unsigned char)arr[pos+3];
    pos+=4;
    for(int i=0;i<y;i++)
        for(int j=0;j<x;j++) {
            value=((uint)(unsigned char)arr[pos]<<24)+((uint)(unsigned char)arr[pos+1]<<16)+((uint)(unsigned char)arr[pos+2]<<8)+(uint)(unsigned char)arr[pos+3];
            pos+=4;
            if(value!=0)
                p.drawRect(j*5,i*5,4,4);
        }

    //p.setRenderHint(QPainter::Antialiasing);
    //p.setRenderHint(QPainter::TextAntialiasing);


}

void MapPainter::mouseMoveEvent(QMouseEvent *event) {
    QPainter p(this);
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 12,50));
    p.translate(2,2);
    p.drawText(QRect(
                   0,
                   30,
                   40,
                   40),
               "5",QTextOption(Qt::AlignCenter));

}
