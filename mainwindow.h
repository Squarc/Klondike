#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTreeWidgetItem>
#include "logger.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QUrlQuery>
#include <QSettings>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QDebug>
#include <QTableWidgetItem>
#include <QDesktopServices>
#include "MapPainter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    struct Rabbit {
        int breedCount;
        quint64 digestionEndTime;
        int id;
        int level;
        int outputCount;
        int rotate;
        QString type;
        int x,y;
    };
    QList<Rabbit*> rabbits;
    struct LiteGameState
    {
        int giftLimit;
        bool haveTreasure,male;
        QString userName;

    };
    struct User
    {
        int id,sex;
        QString first_name,last_name,bdate,city,country,photo, desc;

        QString accessDate,achievements,name,railway;
        int exp,grade,level,ticket;
        LiteGameState liteGameState;
        QStringList wishlist;
        QList<int> partners;
        bool tutorial;
        bool gameFriend;
        quint64 freeGiftBlock;
        QJsonObject toJson() {
            QJsonObject obj;
            obj.insert("bdate",QJsonValue(bdate));
            obj.insert("id",QJsonValue(id));
            obj.insert("first_name",QJsonValue(first_name));
            obj.insert("last_name",QJsonValue(last_name));
            obj.insert("uid",QJsonValue(id));
            obj.insert("sex",QJsonValue(sex));
            obj.insert("city",QJsonValue(city));
            obj.insert("country",QJsonValue(country));
            return obj;
        }
        User() {
            freeGiftBlock=0;
            level=-1;
            gameFriend=false;
        }
    };
    struct Grass {
        int cutCount;
        quint64 id;
        int rotate,x,y;
    };
    struct Gift {
        int count;
        bool free;
        qint64 id;
        QString item;
        QString msg;
        int user;
        QString toString() {
            return QString("%1|%2|%3|%4|%5|%6|%7|%8").arg(QDateTime::currentDateTime().toTime_t()).arg(user).arg(item).arg(count).arg(free).arg(id).arg(needed).arg(msg);
        }
        bool needed;
    };
    struct Nest {
        qint64 id;
        int x,y;
    };
    QList<Gift*> gifts;
    QStringList wishlist;
    QList<Nest*> nests;
    QList<Grass*> grasses;
    void getZones();
    void treeObject(QTreeWidgetItem* item, QJsonObject obj);
    void treeArray(QTreeWidgetItem* item, QJsonArray obj);
    void findItem(QTreeWidgetItem *item,QString text);
    QByteArray solidInit1(QByteArray string);
    QByteArray solidInit2(QByteArray string);
    QByteArray solidSumChar(QByteArray oldString, QByteArray newString);
    QByteArray solidHash(QByteArray oldString, QByteArray newString);
    QByteArray Hash(QByteArray string);
    QString Hash(QString string);
    QString Salt(QString data);
    Logger *logger, *giftLogger;
    QWebEngineView *view;
    QNetworkAccessManager netManager,netManagerVK;
    QNetworkDiskCache diskCache;
    QNetworkRequest request,requestVK;
    QNetworkReply* reply;
    QNetworkCookieJar *cookieJar;
    QNetworkProxy proxy;
    double packageID,serverTime,eventId;
    QString access_token, user_id,auth_key,sessionKey,clientVersion,digestHash;
    QString host;
    QMap<QByteArray,QByteArray> cookies;
    QJsonDocument jsonItems,jsonUserInfo;
    QMap<QString,QJsonObject> objectItems;
    QMap<quint64,User*> users;
    QTimer *friendsTimer,*gameTimer,*eventTimer;
    QMap<QString,int> collectionItems;
    QMap<QString,int> storage;
    void getItems();
    void pickGifts();
    void getUsers(QString user_id);
    void getUsers(QList<int> user_ids);
    void getAppUsers();
    void getFriends();
    void getHello(QString url=QString("https://klone-vk.vapp-servers.com/klonevk"));
    void getDigest(QString hash);
    void getMissions();
    void getInfo();
    void deleteFriend(int id);
    void breedRabbit(Rabbit *r1, Rabbit *r2);
    void moveItem(Rabbit *r, int x, int y);
    void moveAllRabits(int x,int y);
    void pickItem(Rabbit *r);
    void pickItem(int id);
    void sellItem(int itemId);
    void placeFromStorage();
    QJsonObject loadEvent();
    QJsonObject loadEvent(QJsonObject event);
    QJsonObject loadEvent(QList<QJsonObject> events);
    qint64 startTime;
    int countAnimals;

private slots:
    void on_btnOpenJSON_clicked();
    void on_btnFind_clicked();
    void on_btnTreeDelete_clicked();
    void on_pushButton_2_clicked();
    void on_btnBase64_clicked();
    void on_btnCalcFriends_clicked();
    void on_btnGet1_clicked();
    void on_btn_isAppUsers_clicked();
    void on_pushButton_clicked();

    void on_tblUsers_cellDoubleClicked(int row, int column);

public slots:
    void webView_cookieAdded(QNetworkCookie);
    void webView_urlChanged(QUrl);
    void webView_finished(bool ok);

    void slot_getMissions();
    void slot_getUserInfo();
    void slot_isAppUser();
    void slot_items();
    void slot_getAppUsers();
    void slot_getFriends();
    void slot_getDigest();
    void slot_getFile();
    void slot_friendsTimer();
    void slot_gameTimer();
    void slot_getHello();
    void slot_getStage6();
    void slot_getStart();
    void slot_getInfo();
    void slot_getEvent();
    void slot_eventTimer();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
