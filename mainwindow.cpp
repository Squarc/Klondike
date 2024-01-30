#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    logger = new Logger("01 MainLogger",this);
    logger->file.remove();
    giftLogger = new Logger("02 Gifts",this);
    //qDebug()<<QSslSocket::sslLibraryBuildVersionString();
    auth_key="76d05e46d5278524324f58613d10c823";
    sessionKey="";
    eventId=0;
    startTime=QDateTime::currentMSecsSinceEpoch();
    packageID=392;
    clientVersion="1642079931";
    digestHash="5327913340c975bef8ea64d5d5d551817adab879";
    ui->setupUi(this);
    MapPainter *mapPainter = new MapPainter("main_map",this);
    cookieJar= new QNetworkCookieJar(this);
    host="http://";
    view = new QWebEngineView(this);
    ui->verticalLayout_5->addWidget(view);
    view->page()->profile()->setHttpUserAgent("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:58.0) Gecko/20100101 Firefox/58.0");
    connect(view,SIGNAL(urlChanged(QUrl)),SLOT(webView_urlChanged(QUrl)));
    connect(view,SIGNAL(loadFinished(bool)),SLOT(webView_finished(bool)));
    QWebEngineCookieStore *m_store = view->page()->profile()->cookieStore();
     Q_ASSERT(m_store != nullptr);
     connect(m_store, SIGNAL(cookieAdded(QNetworkCookie)),this,SLOT(webView_cookieAdded(QNetworkCookie)));
     m_store->loadAllCookies();

    diskCache.setCacheDirectory("./cache/");
    netManager.setCache(&diskCache);
    netManagerVK.setCache(&diskCache);
    netManager.setCookieJar(cookieJar);
    netManagerVK.setCookieJar(cookieJar);

    QFile fFriends("./friends.txt");
    if (fFriends.open(QIODevice::ReadOnly | QIODevice::Text))
            ui->txtFriends->setText(fFriends.readAll());
    fFriends.close();
    friendsTimer = new QTimer(this);
    friendsTimer->setInterval(200);
    friendsTimer->setSingleShot(true);
    connect(friendsTimer,SIGNAL(timeout()),SLOT(slot_friendsTimer()));
    gameTimer = new QTimer(this);
    gameTimer->setInterval(30000);
    gameTimer->setSingleShot(true);
    connect(gameTimer,SIGNAL(timeout()),SLOT(slot_gameTimer()));

    //friendsTimer->start();
    ui->verticalLayout_6->addWidget(mapPainter);
    eventTimer = new QTimer(this);
    eventTimer->setInterval(1000);
    eventTimer->setSingleShot(false);
    connect(eventTimer,SIGNAL(timeout()),SLOT(slot_eventTimer()));
    view->load(QUrl("https://vk.com/"));

    //slot_items();
    //s/lot_getStart();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://qt-project.org")));
    QNetworkRequest request;
    request.setUrl(QUrl("http://qt-project.org"));
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

MainWindow::~MainWindow()
{
    QFile fFriends("./friends.txt");
    if (fFriends.open(QIODevice::WriteOnly | QIODevice::Text))
        fFriends.write(ui->txtFriends->toPlainText().toUtf8());
    fFriends.close();
    delete ui;
}


void MainWindow::getDigest(QString hash) {
    request.setUrl(QUrl("https://s-rus.vapp-servers.com/klonevk-res/"+hash+"digests.txt"));
    QNetworkReply *reply = netManager.get(request);
    connect(reply,SIGNAL(finished()),SLOT(slot_getDigest()));
}

void MainWindow::slot_getDigest() {
    QDir _dir;
    QMap<QString,QString> mapOld;
    QString url;
    QNetworkReply *reply=(QNetworkReply*)sender();
    logger->Message(reply->url().toString());
    if(reply->url().toString().startsWith("https://s-rus.vapp-servers.com/klonevk-res/"))
        url="."+reply->url().toString().mid(30);
    QFileInfo inf(url);
    if(inf.fileName().length()>40)
        url.replace(inf.fileName().mid(0,40),"");
    _dir.mkpath(inf.path());
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    QTextStream stream(arr);
    QString str;
    QStringList list;
    QFile oldDigests(url);
    oldDigests.open(QFile::ReadWrite);
    QTextStream stream1(oldDigests.readAll());
    oldDigests.close();
    do {
        QString str1=stream1.readLine();
        if(str1.startsWith("!")) continue;
        if(str1.isEmpty()) continue;
        list = str1.split("=");
        mapOld.insert(list[0],list[1]);
    }while(!stream1.atEnd());

    do {
        str=stream.readLine();
        if(str.startsWith("!")) continue;
        if(str.isEmpty()) continue;

        list = str.split("=");
        if(mapOld.contains(list[0]) && mapOld[list[0]]==list[1]) continue;
        inf.setFile(list[0]);
        QString fullFile="https://s-rus.vapp-servers.com/klonevk-res/"+inf.path()+"/"+list[1]+inf.fileName();
        request.setUrl(QUrl(fullFile));
        QNetworkReply *reply = netManager.get(request);
        connect(reply,SIGNAL(finished()),SLOT(slot_getFile()));
    }while(!stream.atEnd());
    QFile file(url);
    file.remove();
    if(file.open(QFile::WriteOnly)) {
        file.write(arr);
        file.close();
    }
}

void MainWindow::slot_getFile() {
    QDir _dir;
    QString url;
    QNetworkReply *reply=(QNetworkReply*)sender();
    logger->Message(reply->url().toString());
    if(reply->url().toString().startsWith("https://s-rus.vapp-servers.com/klonevk-res/"))
        url="."+reply->url().toString().mid(30);
    QFileInfo inf(url);
    if(inf.fileName().length()>40)
        url.replace(inf.fileName().mid(0,40),"");
    _dir.mkpath(inf.path());
    QFile file(url);
    file.open(QFile::WriteOnly);
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }

    file.write(arr);
    file.close();
}

void MainWindow::webView_cookieAdded(QNetworkCookie cookie) {
    cookieJar->insertCookie(cookie);
    cookies.insert(cookie.name(),cookie.value());
}

void MainWindow::webView_urlChanged(QUrl url) {
    logger->Message(url.toString());
    if(!url.toString().contains("access_token"))
        return;
    ui->txtURL->setText(url.toString().replace("#","?"));
    url.setUrl(url.toString().replace("#","?"));
    QUrlQuery uq(url);
    if(uq.hasQueryItem("access_token") && uq.hasQueryItem("user_id")) {
        access_token=uq.queryItemValue("access_token");
        user_id=uq.queryItemValue("user_id");
        getUsers(user_id);
        getAppUsers();
        getFriends();
        //getDigest(digestHash);

    }
}
void MainWindow::webView_finished(bool ok) {
    if(!ok) return;
    view->page()->toHtml([](QString str) {
        qDebug() << str;
    });
}

void MainWindow::getZones() {


}

void MainWindow::on_btnOpenJSON_clicked()
{
    ui->treeWidget->clear();
    QString fileName=QFileDialog::getOpenFileName(this,"Открыть файл");
    ui->txtFileName->setText(fileName);
    if(!fileName.isEmpty()) {
        QFile file(fileName);
        if(file.open(QFile::ReadOnly)) {
            QByteArray arr=file.readAll();
            int index=arr.indexOf("$");

            if(index>=0 && index<=48) {
                QString hash=arr.mid(0,index);
                arr=arr.mid(index+1);
                if(Salt(arr)!=hash) {
                    logger->Message("Плохая подпись пакета");
                }
            }
            QJsonDocument doc = QJsonDocument::fromJson(arr);
            if(doc.isArray())
                treeArray(NULL,doc.array());
            else if(doc.isObject())
                treeObject(NULL,doc.object());
            file.close();
            file.setFileName(fileName+".rm");
            if(file.open(QFile::ReadOnly)) {
                QDataStream sr(&file);
                bool f;
                while(!sr.atEnd()){
                    sr>>fileName;
                    QStringList list = fileName.split('.');
                    QTreeWidgetItem *item=NULL;
                    for(int i=0;i<ui->treeWidget->topLevelItemCount();i++) {
                        item=ui->treeWidget->topLevelItem(i);
                        if(item->text(0)==list[0]) break;
                    }
                    for(int i=1;i<list.count();i++) {
                        if(item==NULL) break;
                        f=false;
                        for(int j=0;j<item->childCount();j++) {
                            if(item->child(j)->text(0)==list[i]) {
                                item=item->child(j);
                                f=true;
                                break;
                            }
                        }
                        if(f) continue;
                        else break;
                    }
                    if(f) delete item;
                }
                file.close();
            }
        }
    }
}

void MainWindow::treeObject(QTreeWidgetItem *item, QJsonObject obj) {
    QStringList lst=obj.keys();
    for(int i=0;i<lst.count();i++) {
        QTreeWidgetItem *it;
        if(item==NULL)
            it = new QTreeWidgetItem(ui->treeWidget);
        else
            it = new QTreeWidgetItem(item);
        it->setText(0,lst[i]);
        if(obj[lst[i]].isObject()) {
            it->setText(1,obj[lst[i]].toObject()["type"].toString());
            QIcon icon;
            icon.addFile(QStringLiteral("../Object.png"), QSize(), QIcon::Normal, QIcon::Off);
            it->setIcon(0, icon);
            treeObject(it,obj[lst[i]].toObject());
        }
        else if(obj[lst[i]].isArray()) {
            it->setText(1,QString::number(obj[lst[i]].toArray().count()));
            QIcon icon;
            icon.addFile(QStringLiteral("../Array.png"), QSize(), QIcon::Normal, QIcon::Off);
            it->setIcon(0, icon);
            treeArray(it,obj[lst[i]].toArray());
        }
        else if(obj[lst[i]].isBool())
            it->setText(1,obj[lst[i]].toBool()?"true":"false");
        else if(obj[lst[i]].isDouble())
            it->setText(1,QString::number(obj[lst[i]].toDouble(),'f')+" (Double)");
        else if(obj[lst[i]].isNull())
            it->setText(1,"NULL");
        else if(obj[lst[i]].isString())
            it->setText(1,"\""+obj[lst[i]].toString()+"\"");
        else if(obj[lst[i]].isUndefined())
            it->setText(1,"Undefined");
    }
}
void MainWindow::treeArray(QTreeWidgetItem *item, QJsonArray obj) {
    for(int i=0;i<obj.count();i++) {
        QTreeWidgetItem *it;
        if(item==NULL)
            it = new QTreeWidgetItem(ui->treeWidget);
        else
            it = new QTreeWidgetItem(item);
        it->setText(0,"["+QString::number(i)+"]");
        if(obj[i].isObject()) {
            it->setText(1,obj[i].toObject()["__CLS__"].toString());
            QIcon icon;
            icon.addFile(QStringLiteral("../Object.png"), QSize(), QIcon::Normal, QIcon::Off);
            it->setIcon(0, icon);
            treeObject(it,obj[i].toObject());
        }
        else if(obj[i].isArray()) {
            QIcon icon;
            icon.addFile(QStringLiteral("../Array.png"), QSize(), QIcon::Normal, QIcon::Off);
            it->setIcon(0, icon);
            treeArray(it,obj[i].toArray());
        }
        else if(obj[i].isBool())
            it->setText(1,obj[i].toBool()?"true":"false");
        else if(obj[i].isDouble())
            it->setText(1,QString::number(obj[i].toDouble(),'f')+" (Double)");
        else if(obj[i].isNull())
            it->setText(1,"NULL");
        else if(obj[i].isString())
            it->setText(1,"\""+obj[i].toString()+"\"");
        else if(obj[i].isUndefined())
            it->setText(1,"Undefined");

    }
}
void MainWindow::findItem(QTreeWidgetItem *item, QString text) {
    for(int i=0;i<item->childCount();i++) {
        if(item->child(i)->text(0).contains(text,Qt::CaseInsensitive)||item->child(i)->text(1).contains(text,Qt::CaseInsensitive)) {
            ui->treeWidget->setCurrentItem(item->child(i));
            return;
        }
        else findItem(item->child(i),text);
    }
}

void MainWindow::on_btnFind_clicked()
{
    QTreeWidgetItem *item;
    item=ui->treeWidget->currentItem();
    while(item!=NULL) {
        if(!item->isExpanded()) item->setExpanded(true);
        item=ui->treeWidget->itemBelow(item);
        if(item==NULL) break;
        if(item->text(0).contains(ui->txtFind->text(),Qt::CaseInsensitive)||item->text(1).contains(ui->txtFind->text(),Qt::CaseInsensitive)) {
            ui->treeWidget->setCurrentItem(item);
            break;
        }
    }
}

void MainWindow::on_btnTreeDelete_clicked()
{
    QFile file(ui->txtFileName->text()+".rm");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QDataStream in(&file);
        in<<ui->txtCurrent->text();
        file.close();
    }
    delete ui->treeWidget->currentItem();
}

void MainWindow::on_pushButton_2_clicked()
{
    QFile file(ui->txtFileName->text()+".rm");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QDataStream in(&file);
        in<<ui->txtCurrent->text();
        file.close();
    }
    delete ui->treeWidget->currentItem();
}

void MainWindow::on_btnBase64_clicked()
{
    QByteArray conv= QByteArray::fromBase64(ui->txtBase64->toPlainText().toLocal8Bit());
    //QByteArray conv= ui->txtBase64->toPlainText().toLocal8Bit();
    if(conv.length()>0 && conv.at(0)==0x78) {
        unsigned int byteSize = conv.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        conv.prepend(bytes);

        conv = qUncompress(conv);
    }
    ui->txtBase64->setText(conv);
    int index=conv.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=conv.mid(0,index);
        conv=conv.mid(index+1);
        if(Salt(conv)==hash) {
            ui->lblBaseInfo->setText("Подпись пакета <b>ОК!</b>");
        }
        else ui->lblBaseInfo->setText("Подпись пакета <b>Bad</b>");
        QMultiMap<int,QString> map;
        ui->treeWidget->clear();
        QJsonDocument doc = QJsonDocument::fromJson(conv);
        if(doc.isObject()) {
            if(doc.object()["auth"].isString()) {
                QString auth=doc.object()["auth"].toString();
                QString myAuth=QString::number(doc.object()["id"].toDouble(),15,0)+auth_key;
                myAuth=Salt(myAuth);
                if(auth==myAuth) ui->lblBaseInfo->setText("Auth OK!");
            }
            if(doc.object()["sig"].isString()) {
                QString sig=doc.object()["sig"].toString();
                QString num=QString::number(doc.object()["id"].toDouble(),15,0);
                QString mySig=sessionKey+num+auth_key;
                mySig=Salt(mySig);
                if(sig==mySig) ui->lblBaseInfo->setText("Sig OK!");
            }

            if(doc.object()["cmd"].toString()=="EVT") {
                foreach(QJsonValue val, doc.object()["events"].toArray()) {
                    if(val.toObject()["type"].toString()=="playersInfo") {
                        foreach(QJsonValue v,val.toObject()["players"].toArray()) {
                            int accessDate = v.toObject()["accessDate"].toString().toInt();
                            map.insert(+accessDate,v.toObject()["id"].toString());

                        }
                    }
                }
                if(!map.empty()) {
                    QMapIterator<int,QString> iter(map);
                    ui->txtBase64->clear();
                    while(iter.hasNext()) {
                        iter.next();
                        int days=abs((double)iter.key())/60/60/24/1000;
                        int hours=abs((double)iter.key())/(60*60*1000)-days*24;
                        int mins=abs((double)iter.key())/(60*1000)-hours*60-days*24*60;
                        ui->txtBase64->append("id"+iter.value()+" не появлялся "+
                                              QString::number(days)+" д "+
                                              QString::number(hours)+" ч "+
                                              QString::number(mins)+" м "+
                                              QDateTime::currentDateTime().addMSecs(iter.key()).toString("(dd.MM.yyyy hh:mm:ss)"));
                    }
                }
            }
        }
        if(doc.isArray())
            treeArray(NULL,doc.array());
        else if(doc.isObject())
            treeObject(NULL,doc.object());

    }

}
QByteArray MainWindow::solidInit1(QByteArray string) {return QByteArray::number(QString(string).length()) + Hash(string+QString("stufff...").toUtf8());}
QByteArray MainWindow::solidInit2(QByteArray string) {return QByteArray::number(QString(string).length()*13);}
QByteArray MainWindow::solidSumChar(QByteArray oldString,QByteArray newString) {
    QString str(newString);
    uint sum=0;
    for(int i=0;i<str.length();i++) {
        sum+=str.at(i).unicode()&0x7f;
    }
    oldString+=QByteArray::number(sum);
    return oldString;
}
QByteArray MainWindow::solidHash(QByteArray oldString, QByteArray newString) {
    QByteArray rez=oldString;
    rez+=QByteArray::number(QString(oldString).length());
    rez+=QByteArray::number(QString(newString).length());
    return Hash(rez);
}
QByteArray MainWindow::Hash(QByteArray string) {
    return QCryptographicHash::hash(string,QCryptographicHash::Md5).toHex();
}
QString MainWindow::Hash(QString string) {
    return QCryptographicHash::hash(string.toUtf8(),QCryptographicHash::Md5).toHex();
}
QString MainWindow::Salt(QString data) {
    QString salt= QString(solidInit1(data.toUtf8()));
    return Hash(QString(data+salt).toUtf8());
}


void MainWindow::on_btnCalcFriends_clicked()
{
    int index;
    QList<int> friends;
    QJsonDocument doc = QJsonDocument::fromJson(ui->txtAllFriends->toPlainText().toLocal8Bit());

    QJsonArray arr=doc.object()["response"].toObject()["items"].toArray();
    foreach (QJsonValue ref, arr) {
        friends.append(ref.toInt());
    }
    doc = QJsonDocument::fromJson(ui->txtGameFriends->toPlainText().toLocal8Bit());
    arr=doc.object()["response"].toArray();
    foreach (QJsonValue ref, arr) {
        if((index=friends.indexOf(ref.toInt()))!=-1)
            friends.removeAt(index);
    }
    QStringList myFriends = ui->txtFriends->toPlainText().split("\n");
    foreach (QString frnd, myFriends) {
        QStringList data = frnd.split(" ");
        if((index=friends.indexOf(data[0].toInt()))!=-1)
            friends.removeAt(index);
    }
    ui->lblRezFriends->clear();
    foreach (int id, friends) {
        ui->lblRezFriends->setText(ui->lblRezFriends->text()+"<a href='http://vk.com/id"+QString::number(id)+"'>http://vk.com/id"+QString::number(id)+"</a><br/>");
    }

    QFile fFriends("./friends.txt");
    if (fFriends.open(QIODevice::WriteOnly | QIODevice::Text))
        fFriends.write(ui->txtFriends->toPlainText().toUtf8());
    fFriends.close();
}

void MainWindow::on_btnGet1_clicked()
{
    view->load(QUrl("https://oauth.vk.com/authorize?client_id=3083242&scope=friends,photos&redirect_uri=https://oauth.vk.com/blank.html&display=popup&v=5.4&response_type=token"));
}

void MainWindow::getUsers(QString user_id) {
    QUrl u("https://api.vk.com/method/users.get");
    QUrlQuery q;

    q.addQueryItem("api_id", "3083242");
    q.addQueryItem("fields", "bdate,sex,first_name,last_name,city,country,uid,photo");
    q.addQueryItem("user_ids", user_id);
    q.addQueryItem("v", "5.8");
    q.addQueryItem("access_token", access_token);
    u.setQuery(q);
    //view->load(u);
    requestVK.setUrl(u);
    //ui->txtURL->setText(u.toString());
    QNetworkReply *reply=netManagerVK.get(requestVK);
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getUserInfo()));
    logger->Post("users.get.json",u.toString());
}

void MainWindow::getUsers(QList<int> user_ids) {
    QString user_id;
    foreach (int id, user_ids) {
        if(user_id.isEmpty()) user_id+=QString::number(id);
        else user_id+=","+QString::number(id);
    }
    getUsers(user_id);
}

void MainWindow::slot_getUserInfo() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    jsonUserInfo=QJsonDocument::fromJson(arr);
    logger->Recive("users.get.json",arr);
    foreach (QJsonValue val, jsonUserInfo.object()["response"].toArray()) {
        QJsonObject obj=val.toObject();
        User *user;
        if(users.contains(obj["id"].toInt()) && users[obj["id"].toDouble()]!=NULL)
            user=users[obj["id"].toDouble()];
        else {
            user=new User();
            users.insert(obj["id"].toInt(),user);
        }
        user->id=obj["id"].toInt();
        user->first_name=obj["first_name"].toString();
        user->last_name=obj["last_name"].toString();
        user->sex=obj["sex"].toInt();
        user->bdate=obj["bdate"].toString();
        user->city=obj["city"].toObject()["title"].toString();
        user->country=obj["country"].toObject()["title"].toString();
        user->photo=obj["photo"].toString();

    }
    friendsTimer->start();

}

void MainWindow::on_btn_isAppUsers_clicked()
{
    getItems();

}

void MainWindow::slot_isAppUser() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    QString str=QString::fromUtf8(arr);
    view->setContent(str.toLocal8Bit());
    QJsonDocument doc=QJsonDocument::fromBinaryData(arr);
    QJsonObject obj=doc.object();

}

void MainWindow::getItems() {
    QUrl u("https://klone-vk.vapp-servers.com/klonevk/items");
    QByteArray data = "compress=true\nlang=ru";
    request.setUrl(u);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QNetworkReply *reply=netManager.post(request,data);
    connect(reply,SIGNAL(finished()),this,SLOT(slot_items()));

}

void MainWindow::slot_items() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr;
    if(reply==NULL) {
        QFile file("items");
        file.open(QFile::ReadOnly);
        arr=file.readAll();
        file.close();
    }
    else arr = reply->readAll();
    reply->deleteLater();
    QFile file("./klonevk-res/items");
    file.open(QFile::WriteOnly);
    file.write(arr);
    file.close();
    jsonItems=QJsonDocument::fromJson(arr);
    //обработка items
    objectItems.clear();

    foreach (QJsonValue val, jsonItems.array()) {
        objectItems.insert(val.toObject()["id"].toString(), val.toObject());
    }
    getHello();
}

void MainWindow::getHello(QString url) {
    QUrl u(url+"/go");
    //packageID=QDateTime::currentDateTime().toMSecsSinceEpoch()-startTime;
    QString myAuth=QString::number(packageID,15,0)+auth_key;
    myAuth=Salt(myAuth);

    QString data = "{\"clientVersion\":"+clientVersion+",\"type\":\"TIME\",\"pluginVersion\":\"Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0; renderer: gl\",\"id\":"+QString::number(packageID,15,0)+",\"user\":\""+user_id+"\",\"auth\":\""+myAuth+"\"}";
    data="gz=y&crc="+Salt(data)+"&data="+data;
    request.setUrl(u);
    //request.setRawHeader("Host","klone-vk.vapp-servers.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/60.0");
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language","en-GB,en;q=0.5");
    request.setRawHeader("Accept-Encoding","gzip, deflate, br");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getHello()));
    logger->Post("go getHello",data);
}
void MainWindow::slot_getHello() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getHello",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonDocument doc=QJsonDocument::fromJson(arr);
    QJsonObject obj=doc.object();
    if(obj["cmd"].toString()=="REDIRECT") {
        getHello(obj["redirect"].toString());
    }
    else if(obj["cmd"].toString()=="TIME") {
        sessionKey=obj["key"].toString();
        logger->Message("Получили сессионный ключ: "+sessionKey);
        serverTime=obj["time"].toDouble();
        packageID=serverTime;
        QJsonObject obj;
        obj.insert("id",QJsonValue(packageID));
        obj.insert("user",QJsonValue(user_id));
        obj.insert("lang",QJsonValue("ru"));
        obj.insert("serverTime",QJsonValue(serverTime));
        obj.insert("info",QJsonValue(users[user_id.toDouble()]->toJson()));
        obj.insert("ad",QJsonValue("unknown"));
        obj.insert("clientTime",QJsonValue(QDateTime::currentMSecsSinceEpoch()-startTime));
        obj.insert("type",QJsonValue("START_LOADING"));
        obj.insert("sig",QJsonValue(Salt(sessionKey+QString::number(packageID,15,0)+auth_key)));
        QString data = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        data="gz=y&crc="+Salt(data)+"&data="+data;
        QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
        packageID++;
        connect(reply,SIGNAL(finished()),this,SLOT(slot_getHello()));
        logger->Post("go slot_getHello",request.url().toString(),data.toLocal8Bit());
    }
    else if(obj["cmd"].toString()=="START_LOADING") {
        QJsonObject event;
        event.insert("step",QJsonValue(6));
        event.insert("name",QJsonValue("main_map.bin"));
        event.insert("type",QJsonValue("loading"));
        QList<QJsonObject> events;
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        events.append(event);
        QString data = QJsonDocument(loadEvent(events)).toJson(QJsonDocument::Compact);
        data="gz=y&crc="+Salt(data)+"&data="+data;
        QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
        packageID++;
        connect(reply,SIGNAL(finished()),this,SLOT(slot_getStage6()));
        logger->Post("go slot_getHello",request.url().toString(),data.toLocal8Bit());

    }
}

QJsonObject MainWindow::loadEvent(QList<QJsonObject> events) {
    QJsonArray jEvents;
    foreach (QJsonObject event, events) {
        jEvents.append(QJsonValue(event));
    }
    QJsonObject obj;
    obj.insert("events",QJsonValue(jEvents));
    obj.insert("time",QJsonValue(QDateTime::currentMSecsSinceEpoch()-startTime));
    obj.insert("id",QJsonValue(packageID));
    obj.insert("user",QJsonValue(user_id));
    obj.insert("type",QJsonValue("EVT"));
    obj.insert("sig",QJsonValue(Salt(sessionKey+QString::number(packageID,15,0)+auth_key)));
    return obj;
}
QJsonObject MainWindow::loadEvent(QJsonObject event) {
    QList<QJsonObject> events;
    events.append(event);
    return loadEvent(events);
}
QJsonObject MainWindow::loadEvent() {
    QList<QJsonObject> events;
    return loadEvent(events);
}

void MainWindow::getMissions() {
    gameTimer->stop();logger->Message("gameTimer->stop() getMissions");
    QJsonObject event;
    event.insert("action",QJsonValue("getMissions"));
    event.insert("id",QJsonValue(++eventId));
    event.insert("type",QJsonValue("action"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getMissions()));
    logger->Post("go getMissions",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::slot_getMissions() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getMissions",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonDocument doc=QJsonDocument::fromJson(arr);
    QJsonObject obj=doc.object();
    gameTimer->stop();logger->Message("gameTimer->stop() slot_getMissions");
    getInfo();

}

void MainWindow::slot_getStage6() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getStage6",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonObject obj;
    obj.insert("id",QJsonValue(packageID));
    obj.insert("user",QJsonValue(user_id));
    obj.insert("lang",QJsonValue("ru"));
    obj.insert("type",QJsonValue("START"));
    obj.insert("serverTime",QJsonValue(packageID));
    obj.insert("info",QJsonValue(users[user_id.toDouble()]->toJson()));
    obj.insert("ad",QJsonValue("unknown"));
    obj.insert("clientTime",QJsonValue(QDateTime::currentMSecsSinceEpoch()-startTime));
    obj.insert("sig",QJsonValue(Salt(sessionKey+QString::number(packageID,15,0)+auth_key)));
    QString data = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    data="gz=y&crc="+Salt(data)+"&data="+data;
    reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getStart()));
    logger->Post("go slot_getStage6",request.url().toString(),data.toLocal8Bit());
}

void MainWindow::slot_getStart() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr;
    if(reply==NULL) {
        QFile file("START.pkg");
        file.open(QFile::ReadOnly);
        arr=file.readAll();
        file.close();
    }
    else arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getStart",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonDocument doc=QJsonDocument::fromJson(arr);
    QJsonObject obj=doc.object();
    QString currentStation=obj["params"].toObject()["event"].toObject()["locationId"].toString();
    if(currentStation.startsWith("@"))
        currentStation=currentStation.mid(1);

    if(objectItems.contains(currentStation)) {
        ui->lblLocation->setText(objectItems[currentStation]["name"].toString());
    }
    wishlist.clear();
    foreach (QJsonValue val, obj["state"].toObject()["wishlist"].toArray()) {
        if(val.isNull()) continue;
        QString wishString=val.toString();
        if(wishString.startsWith("@")) wishString=wishString.mid(1);
        wishlist.append(wishString);
    }
    QList<int> oldGifts=giftLogger->LastGifts();
    foreach (QJsonValue val, obj["state"].toObject()["gifts"].toArray()) {
        Gift *gift = new Gift();
        gift->count=val.toObject()["count"].toInt();
        gift->free=val.toObject()["free"].toBool();
        gift->id=val.toObject()["id"].toDouble();
        gift->item=val.toObject()["item"].toString();
        if(gift->item.startsWith("@"))
            gift->item=gift->item.mid(1);
        gift->msg=val.toObject()["msg"].toString();
        gift->user=val.toObject()["user"].toString().toInt();
        gift->needed=wishlist.contains(gift->item);
        gifts.append(gift);
        if(!oldGifts.contains(val.toObject()["user"].toString().toInt()))
            giftLogger->Data(gift->toString());
    }
    foreach (QJsonValue val, obj["state"].toObject()["friends"].toArray()) {
        int id = val.toString().toInt();
        if(users.contains(id)) {
            ui->lblRezFriends->setText(ui->lblRezFriends->text()+"Сосед id"+val.toString()+" у вас в друзьях<br/>");
        }
        else {
            User *u=new User();
            u->id=id;
            u->gameFriend=true;
            u->desc="Сосед";
            users.insert(id,u);
        }
    }
    foreach (QJsonValue val, obj["state"].toObject()["freeGiftUsers"].toArray()) {
        if(users.contains(val.toObject()["user"].toString().toDouble()))
                users[val.toObject()["user"].toString().toDouble()]->freeGiftBlock=QDateTime::currentMSecsSinceEpoch()+val.toObject()["blockedUntil"].toString().toDouble();

    }
    foreach (Rabbit *r, rabbits) {
        delete r;
    }
    foreach (Nest *n, nests) {
        delete n;
    }
    foreach (Grass *g, grasses) {
        delete g;
    }
    nests.clear();
    rabbits.clear();
    grasses.clear();
    countAnimals=0;
    foreach (QJsonValue val, obj["params"].toObject()["event"].toObject()["gameObjects"].toArray()) {
        if(val.toObject()["type"].toString()=="hoofed" ||
                val.toObject()["type"].toString()=="plumed"||
                val.toObject()["type"].toString()=="breed")
            countAnimals++;
        if(val.toObject()["item"].toString().mid(1)=="A_RABBIT") {
            Rabbit *rabbit=new Rabbit();
            rabbit->breedCount=val.toObject()["breedCount"].toInt();
            rabbit->digestionEndTime=QDateTime::currentMSecsSinceEpoch()+val.toObject()["digestionEndTime"].toString().toInt();
            rabbit->id=val.toObject()["id"].toInt();
            rabbit->level=val.toObject()["level"].toInt();
            rabbit->outputCount=val.toObject()["outputCount"].toInt();
            rabbit->rotate=val.toObject()["rotate"].toInt();
            rabbit->type=val.toObject()["type"].toString();
            rabbit->x=val.toObject()["x"].toInt();
            rabbit->y=val.toObject()["y"].toInt();
            rabbits.append(rabbit);
        }
//{"cutCount":20,"id":8112,"item":"@T_GRASS_WELL_3_NEW","rotate":0,"type":"grass","x":106,"y":47}
        if(val.toObject()["item"].toString().mid(1).startsWith("T_GRASS_")) {
            Grass *grass = new Grass();
            grass->cutCount=val.toObject()["cutCount"].toInt();
            grass->id=val.toObject()["id"].toDouble();
            grass->rotate=val.toObject()["rotate"].toInt();
            grass->x=val.toObject()["x"].toInt();
            grass->y=val.toObject()["y"].toInt();
            grasses.append(grass);
        }
        if(val.toObject()["type"].toString()=="pickup") {
            Nest *n = new Nest();
            n->id= val.toObject()["id"].toDouble();
            n->x=val.toObject()["x"].toInt();
            n->y=val.toObject()["y"].toInt();
            nests.append(n);
        }
        ui->txtBase64->append((val.toObject().contains("craftNo")?"<font color='red'>":"")+objectItems[val.toObject()["item"].toString().mid(1)]["name"].toString()+(val.toObject().contains("craftNo")?"</font> ":" ")+QString(QJsonDocument(val.toObject()).toJson(QJsonDocument::Compact)));
    }
    ui->txtBase64->append(QString("Общее количество животных: %1").arg(countAnimals));
    foreach (QJsonValue val, obj["state"].toObject()["collectionItems"].toArray()) {
        ;
    }

    gameTimer->start();logger->Message("gameTimer->start()");
    getMissions();
}

void MainWindow::getInfo() {
    gameTimer->stop();logger->Message("gameTimer->stop() getInfo");
    QJsonArray players;
    foreach (quint64 id, users.keys()) {
        User *u=users[id];
        if(u==NULL) {
            u=new User();
            u->id=id;
            users.insert(id,u);
        }
        if(u->level==-1)
            players.append(QJsonValue(QString::number(id)));
        if(players.count()>=100)
            break;
    }
    if(players.count()==0) {
        friendsTimer->start();
        eventTimer->start();
        gameTimer->start();logger->Message("gameTimer->start()");
        return;
    }
    QJsonObject event;
    event.insert("action",QJsonValue("getInfo"));
    event.insert("id",QJsonValue(++eventId));
    event.insert("players",QJsonValue(players));
    event.insert("type",QJsonValue("players"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getInfo()));
    logger->Post("go getInfo",request.url().toString(),data.toLocal8Bit());

}
void MainWindow::slot_getInfo() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getInfo",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonDocument doc=QJsonDocument::fromJson(arr);
    QJsonObject obj=doc.object();
    // {"cmd":"EVT","id":"1527658109137","events":[{"type":"evt","action":"ping1"}]}
    gameTimer->start();logger->Message("gameTimer->start()");
    if(obj["cmd"].toString()=="EVT") {
        foreach (QJsonValue val, obj["events"].toArray()) {
            if(val.toObject()["type"].toString()=="evt" && val.toObject()["action"].toString()=="ping1") {
                QTime time;
                time.start();
                for(;time.elapsed() < 100;)
                {
                 qApp->processEvents();
                }
                QString data = QJsonDocument(loadEvent()).toJson(QJsonDocument::Compact);
                data="gz=y&crc="+Salt(data)+"&data="+data;
                QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
                packageID++;
                connect(reply,SIGNAL(finished()),this,SLOT(slot_getInfo()));
                logger->Post("go slot_getInfo",request.url().toString(),data.toLocal8Bit());
                return;
            }
            if(val.toObject()["type"].toString()=="evt" && val.toObject()["action"].toString()=="ping2") {
                QTime time;
                time.start();
                for(;time.elapsed() < 500;)
                {
                 qApp->processEvents();
                }

                QString data = QJsonDocument(loadEvent()).toJson(QJsonDocument::Compact);
                data="gz=y&crc="+Salt(data)+"&data="+data;
                QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
                packageID++;
                connect(reply,SIGNAL(finished()),this,SLOT(slot_getInfo()));
                logger->Post("go slot_getInfo",request.url().toString(),data.toLocal8Bit());
                return;
            }
            if(val.toObject()["type"].toString()=="playersInfo" && val.toObject()["action"].toString()=="getInfo") {
                foreach (QJsonValue user, val.toObject()["players"].toArray()) {
                    quint64 userId=user.toObject()["id"].toString().toDouble();
                    User *u=users[userId];
                    if(u==NULL) {
                        u=new User();
                        users.insert(userId,u);
                    }
                    u->accessDate=user.toObject()["accessDate"].toString();
                    u->achievements=user.toObject()["achievements"].toString();
                    u->exp=user.toObject()["exp"].toInt();
                    u->grade=user.toObject()["grade"].toInt();
                    u->level=user.toObject()["level"].toInt();
                    u->id=user.toObject()["id"].toString().toInt();
                    u->liteGameState.giftLimit=user.toObject()["liteGameState"].toObject()["giftLimit"].toInt();
                    u->liteGameState.haveTreasure=user.toObject()["liteGameState"].toObject()["haveTreasure"].toBool();
                    u->liteGameState.male=user.toObject()["liteGameState"].toObject()["playerSettings"].toObject()["male"].toBool();
                    u->liteGameState.userName=user.toObject()["liteGameState"].toObject()["playerSettings"].toObject()["userName"].toString();
                    u->wishlist.clear();
                    foreach (QJsonValue wl, user.toObject()["liteGameState"].toObject()["wishlist"].toArray()) {
                        if(wl.isNull()) continue;
                        QString wishString=wl.toString();
                        if(wishString.startsWith("@")) wishString=wishString.mid(1);
                        u->wishlist.append(wishString);
                    }
                    u->name=user.toObject()["name"].toString();
                    u->partners.clear();
                    foreach (QJsonValue partner, user.toObject()["partners"].toArray()) {
                        u->partners.append(partner.toString().toInt());
                        if(!users.contains(partner.toString().toInt())) {
                            User *u=new User();
                            u->id=partner.toString().toInt();
                            u->desc="Партнер игрока (Отсутствует)"+QString::number(u->id);
                            if(u->id==0)
                                qDebug()<<"0\n";
                            users.insert(partner.toString().toInt(),u);
                        }
                    }
                    //users.insert(user.toObject()["id"].toInt(),u);
                }
                getInfo();
            }
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    QTableWidgetItem *item;
    //ui->tblUsers->clear();
    ui->tblUsers->setRowCount(users.count());
    int i=0;
    QStringList myFriends = ui->txtFriends->toPlainText().split("\n");
    foreach (quint64 id, users.keys()) {
        item=ui->tblUsers->item(i,0); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,0,item);} item->setText(QString::number(users[id]->id));
        foreach (QString frnd, myFriends) {
            QStringList data = frnd.split(" ");
            if(data[0].toDouble()==id) {
                item->setBackgroundColor(Qt::red);
                break;
            }
        }
        item=ui->tblUsers->item(i,1); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,1,item);} item->setText(users[id]->first_name);
        item=ui->tblUsers->item(i,2); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,2,item);} item->setText(users[id]->last_name);
        item=ui->tblUsers->item(i,3); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,3,item);} item->setText(users[id]->name);
        item=ui->tblUsers->item(i,4); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,4,item);} item->setText(QString::number(users[id]->level));
        item=ui->tblUsers->item(i,5); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,5,item);} item->setText(QString::number(users[id]->grade).rightJustified(6,'0'));
        if(users[id]->accessDate.startsWith("-")) {
            double v=users[id]->accessDate.mid(1).toDouble();
            int days=(int)(v/60/60/24/1000);
            int hours=(int)(v/(60*60*1000)-days*24);
            int mins=(int)(v/(60*1000)-hours*60-days*24*60);


            item=ui->tblUsers->item(i,6); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,6,item);} item->setText(QString("%1 д %2 ч %3 м").arg(QString::number(days).rightJustified(3,'0')).arg(QString::number(hours).rightJustified(2,'0')).arg(QString::number(mins).rightJustified(2,'0')));
        }
        item=ui->tblUsers->item(i,7); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,7,item);} item->setText(QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()+users[id]->accessDate.toDouble()).toString("yyyy:MM:dd HH:mm:ss"));
        item=ui->tblUsers->item(i,8); if(item==NULL) {item = new QTableWidgetItem(); ui->tblUsers->setItem(i,8,item);} item->setText(users[id]->desc);
        i++;
    }
    ui->tblUsers->resizeColumnsToContents();
    ui->tblUsers->resizeRowsToContents();

}

void MainWindow::getAppUsers() {
    QUrl u("https://api.vk.com/method/friends.getAppUsers");
    QUrlQuery q;
    q.addQueryItem("api_id", "3083242");
    q.addQueryItem("v", "5.8");
    q.addQueryItem("access_token", access_token);
    u.setQuery(q);
    requestVK.setUrl(u);
    QNetworkReply *reply=netManagerVK.get(requestVK);
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getAppUsers()));
}
void MainWindow::getFriends() {
    QUrl u("https://api.vk.com/method/friends.get");
    QUrlQuery q;
    q.addQueryItem("api_id", "3083242");
    q.addQueryItem("v", "5.8");
    q.addQueryItem("access_token", access_token);
    u.setQuery(q);
    requestVK.setUrl(u);
    reply=netManagerVK.get(requestVK);
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getFriends()));
}

void MainWindow::slot_getAppUsers() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(arr);
    foreach (QJsonValue val, doc.object()["response"].toArray()) {
        users.insert(val.toInt(),new User());
    }
    ui->txtGameFriends->setText(QString::fromUtf8(arr));
    if(ui->txtAllFriends->toPlainText().length()>10)
        on_btnCalcFriends_clicked();
    getItems();
}

void MainWindow::slot_getFriends() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    QString str=QString::fromUtf8(arr);
    ui->txtAllFriends->setText(str);
    if(ui->txtGameFriends->toPlainText().length()>10)
        on_btnCalcFriends_clicked();
    friendsTimer->start();

}

void MainWindow::slot_friendsTimer() {
    QList<int> need;
    int count=0;
    ui->pgbLoad->setMaximum(users.count());
    foreach (quint64 key, users.keys()) {
        if(users[key]==NULL || users[key]->first_name.isEmpty()) {
            need.append(key);
            if(need.length()>=100) {
                getUsers(need);
                need.clear();
                ui->pgbLoad->setValue(count);
                ui->pgbLoad->update();
                return;
            }
        }
        else
            count++;
    }
    //ui->pgbLoad->setValue(count);
    if(need.length()==0) {
        //friendsTimer->stop();
        //ui->pgbLoad->setMaximum(100);
        //ui->pgbLoad->setValue(100);
        //ui->pgbLoad->update();
        startTime=QDateTime::currentMSecsSinceEpoch();

    }
    else {
        getUsers(need);
        need.clear();
        return;

    }
}

void MainWindow::on_tblUsers_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem *item;
    item=ui->tblUsers->item(row,0);

    if(column==0) {
        QDesktopServices::openUrl(QUrl("http://vk.com/id"+item->text()));
    }
    if(column==6 && users[item->text().toDouble()]->gameFriend) {
        deleteFriend(item->text().toInt());
    }
}

void MainWindow::deleteFriend(int id) {
    gameTimer->stop();logger->Message("gameTimer->stop() deleteFriend");
    //[{"action":"delete","type":"friend","user":"109903214"}
    QJsonObject event;
    event.insert("action",QJsonValue("delete"));
    event.insert("user",QJsonValue(QString::number(id)));
    event.insert("type",QJsonValue("friend"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go deleteFriend",request.url().toString(),data.toLocal8Bit());
}

void MainWindow::slot_gameTimer() {
    QString data = QJsonDocument(loadEvent()).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go slot_gameTimer",request.url().toString(),data.toLocal8Bit());

}

void MainWindow::slot_getEvent() {
    QNetworkReply *reply=(QNetworkReply*)sender();
    QByteArray arr = reply->readAll();
    reply->deleteLater();
    if(arr.length()>0 && arr.at(0)==0x78) {
        unsigned int byteSize = arr.size();
        QByteArray bytes;
        bytes.append(byteSize >> 24);
        bytes.append(byteSize >> 16);
        bytes.append(byteSize >> 8);
        bytes.append(byteSize);
        arr.prepend(bytes);

        arr = qUncompress(arr);
    }
    logger->File("go slot_getEvent",arr);
    int index=arr.indexOf("$");

    if(index>=0 && index<=48) {
        QString hash=arr.mid(0,index);
        arr=arr.mid(index+1);
        if(Salt(arr)!=hash) {
            logger->Message("Плохая подпись пакета");
        }
    }
    QJsonDocument doc=QJsonDocument::fromJson(arr);
    QJsonObject obj=doc.object();
    // {"cmd":"EVT","id":"1527658109137","events":[{"type":"evt","action":"ping1"}]}
    QString cmd=obj["cmd"].toString();
    if(cmd=="ERR") {
        eventTimer->stop(); logger->Message("eventTimer->stop()");
        return;
    }
    if(cmd=="EVT") {
        foreach (QJsonValue val, obj["events"].toArray()) {
            if(val.toObject()["type"].toString()=="gift" && val.toObject()["action"].toString()=="addGift") {
                Gift *gift = new Gift();
                gift->count=val.toObject()["gift"].toObject()["count"].toInt();
                gift->free=val.toObject()["gift"].toObject()["free"].toBool();
                gift->id=val.toObject()["gift"].toObject()["id"].toDouble();
                gift->item=val.toObject()["gift"].toObject()["item"].toString();
                if(gift->item.startsWith("@"))
                    gift->item=gift->item.mid(1);
                gift->msg=val.toObject()["gift"].toObject()["msg"].toString();
                gift->user=val.toObject()["gift"].toObject()["user"].toString().toInt();
                gift->needed=wishlist.contains(gift->item);
                gifts.append(gift);
                giftLogger->Data(gift->toString());

            }
            if(val.toObject()["type"].toString()=="gameObject" && val.toObject()["action"].toString()=="addObject") {
                if(val.toObject()["gameObject"].toObject()["item"].toString()=="@A_RABBIT") {
                    Rabbit *rabbit=new Rabbit();
                    rabbit->breedCount=val.toObject()["gameObject"].toObject()["breedCount"].toInt();
                    rabbit->digestionEndTime=QDateTime::currentMSecsSinceEpoch()+val.toObject()["gameObject"].toObject()["digestionEndTime"].toString().toInt();
                    rabbit->id=val.toObject()["gameObject"].toObject()["id"].toInt();
                    rabbit->level=val.toObject()["gameObject"].toObject()["level"].toInt();
                    rabbit->outputCount=val.toObject()["gameObject"].toObject()["outputCount"].toInt();
                    rabbit->rotate=val.toObject()["gameObject"].toObject()["rotate"].toInt();
                    rabbit->type=val.toObject()["gameObject"].toObject()["type"].toString();
                    rabbit->x=val.toObject()["gameObject"].toObject()["x"].toInt();
                    rabbit->y=val.toObject()["gameObject"].toObject()["y"].toInt();
                    rabbits.append(rabbit);
                    logger->Message("Новый кролик");
                }
                if(val.toObject()["gameObject"].toObject()["item"].toString().startsWith("@SC_NEST_"))  {
                    Nest *n=new Nest();
                    n->id=val.toObject()["gameObject"].toObject()["id"].toDouble();
                    n->x=val.toObject()["gameObject"].toObject()["x"].toInt();
                    n->y=val.toObject()["gameObject"].toObject()["y"].toInt();
                    nests.append(n);
                    logger->Message("Новое гнездо");
                }
            }
            if(val.toObject()["type"].toString()=="pickup" && val.toObject()["action"].toString()=="add") {
                gameTimer->stop();logger->Message("gameTimer->stop() slot_getEvent");
                QList<QJsonObject> events;
                foreach (QJsonValue val, val.toObject()["pickups"].toArray()) {
                    QJsonObject event;
                    QJsonArray arr;
                    arr.append(val.toObject());
                    event.insert("pickups",QJsonValue(arr));
                    event.insert("type",QJsonValue("pickup"));
                    event.insert("action",QJsonValue("pick"));
                    events.append(event);
                }
                if(events.length()==0) return;
                QString data = QJsonDocument(loadEvent(events)).toJson(QJsonDocument::Compact);
                data="gz=y&crc="+Salt(data)+"&data="+data;
                QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
                packageID++;
                connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
                logger->Post("go slot_getEvent pick",request.url().toString(),data.toLocal8Bit());

            }
        }
    }
    gameTimer->start();logger->Message("gameTimer->start()");
}

void MainWindow::breedRabbit(Rabbit *r1, Rabbit *r2) {
    gameTimer->stop();logger->Message("gameTimer->stop() breedRabbit");
    QJsonObject event;
    event.insert("id",QJsonValue(++eventId));
    event.insert("x",QJsonValue(r1->x));
    event.insert("y",QJsonValue(r1->y));
    event.insert("objId",QJsonValue(r1->id));
    event.insert("partnerId",QJsonValue(r2->id));
    event.insert("type",QJsonValue("breed"));
    event.insert("action",QJsonValue("breed"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go breedRabbit",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::moveItem(Rabbit *r,int x,int y) {
    gameTimer->stop();logger->Message("gameTimer->stop() moveItem");
    QJsonObject event;
    event.insert("x",QJsonValue(x));
    event.insert("y",QJsonValue(y));
    event.insert("objId",QJsonValue(r->id));
    event.insert("type",QJsonValue("item"));
    event.insert("action",QJsonValue("move"));
    event.insert("rotate",QJsonValue(r->rotate));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go breedRabbit",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::moveAllRabits(int x, int y) {
    QList<QJsonObject> events;
    foreach (Rabbit *r, rabbits) {
        if(r->x!=x || r->y!=y) {
            r->x=x;
            r->y=y;
            QJsonObject event;
            event.insert("x",QJsonValue(x));
            event.insert("y",QJsonValue(y));
            event.insert("objId",QJsonValue(r->id));
            event.insert("type",QJsonValue("item"));
            event.insert("action",QJsonValue("move"));
            event.insert("rotate",QJsonValue(r->rotate));
            events.append(event);
        }
    }
    if(events.count()==0) return;
    gameTimer->stop();logger->Message("gameTimer->stop() moveAllRabits");
    QString data = QJsonDocument(loadEvent(events)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go moveAllRabits",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::pickItem(Rabbit *r) {
    gameTimer->stop();logger->Message("gameTimer->stop() pickItem");
    QJsonObject event;
    event.insert("rotate",QJsonValue(r->rotate));
    event.insert("itemId",QJsonValue());
    event.insert("x",QJsonValue(r->x));
    event.insert("y",QJsonValue(r->y));
    event.insert("objId",QJsonValue(r->id));
    event.insert("type",QJsonValue("item"));
    event.insert("action",QJsonValue("pick"));
    rabbits.removeOne(r);
    delete r;
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go pickItem rabbit",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::pickItem(int id) {
    gameTimer->stop();logger->Message("gameTimer->stop() pickItem");
    QJsonObject event;
    event.insert("type",QJsonValue("item"));
    event.insert("action",QJsonValue("pick"));
    event.insert("objId",QJsonValue(id));

    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go pickItem nests",request.url().toString(),data.toLocal8Bit());
}
void MainWindow::slot_eventTimer() {
    foreach (Rabbit *r, rabbits) {
        if(r->breedCount==1 && r->outputCount==26) {
            pickItem(r);
            return;
        }
    }
    moveAllRabits(69,42);
    if(gifts.count()>10) {
        pickGifts();
        return;
    }
    if(countAnimals>450) {
        foreach (Rabbit *r, rabbits) {
            if(r->breedCount==0 && r->outputCount==0) {
                sellItem(r->id);
                rabbits.removeOne(r);
                delete r;
                return;
            }
        }
    }
    if(nests.count()>0) {
        foreach (Nest *n, nests) {
            nests.removeOne(n);
            pickItem(n->id);
            delete n;
            return;
        }
    }
}

void MainWindow::pickGifts() {
    gameTimer->stop();logger->Message("gameTimer->stop() pickGifts");
    QList<QJsonObject> events;
    foreach (Gift *r, gifts) {
            QJsonObject event;
            QJsonObject giftId;
            giftId.insert("id",QJsonValue(r->id));
            event.insert("gift",QJsonValue(giftId));
            event.insert("type",QJsonValue("gift"));
            event.insert("action",QJsonValue("applyGift"));
            events.append(event);
            gifts.removeOne(r);
            delete r;
            if(events.count()>19) break;
    }

    if(events.length()==0) return;
    QString data = QJsonDocument(loadEvent(events)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go pickGifts",request.url().toString(),data.toLocal8Bit());
}

void MainWindow::sellItem(int itemId) {
    gameTimer->stop();logger->Message("gameTimer->stop() sellItem");
    QJsonObject event;
    event.insert("objId",QJsonValue(itemId));
    event.insert("type",QJsonValue("item"));
    event.insert("action",QJsonValue("sell"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go sellItem",request.url().toString(),data.toLocal8Bit());

}
void MainWindow::placeFromStorage() {
//{"type":"EVT","user":"15721129","id":1530102424344,"events":[{"x":75,"y":47,"itemId":"T_GRASS_WELL_3_NEW","type":"item","objId":8155,"action":"placeFromStorage"}],"time":461589,"sig":"ef1541dc6452c7d886b4e84bce2c4ace"}
    gameTimer->stop();logger->Message("gameTimer->stop() sellItem");
    QJsonObject event;
    event.insert("objId",QJsonValue(0));
    event.insert("type",QJsonValue("item"));
    event.insert("action",QJsonValue("sell"));
    QString data = QJsonDocument(loadEvent(event)).toJson(QJsonDocument::Compact);
    data="gz=y&crc="+Salt(data)+"&data="+data;
    QNetworkReply *reply=netManager.post(request,data.toLocal8Bit());
    packageID++;
    connect(reply,SIGNAL(finished()),this,SLOT(slot_getEvent()));
    logger->Post("go sellItem",request.url().toString(),data.toLocal8Bit());

}
