#include "sv_area.h"

extern SvSQLITE *SQLITE;
//extern SvBeaconEditor* BEACONEDITOR_UI;

inline QPointF geo2point(const area::AREA_DATA *area_data, qreal lon, qreal lat);
inline QPointF point2geo(const area::AREA_DATA *area_data, qreal x, qreal y);

QPointF geo2point(const area::AREA_DATA* area_data, qreal lon, qreal lat)
{
//  qDebug() << "geo2point" << area_data->geo_bounds.max_lat << lat;
  qreal x = area_data->area_curr_size.width() - (area_data->geo_bounds.max_lon - lon) * area_data->koeff.lon/** area_data->koeff2*/;
  qreal y = (area_data->geo_bounds.max_lat - lat) * area_data->koeff.lat /** area_data->koeff2*/;
//  qDebug() << x << y;
  return QPointF(x, y);
}

QPointF point2geo(const area::AREA_DATA* area_data, qreal x, qreal y)
{
  qreal lon = area_data->geo_bounds.min_lon + x / area_data->koeff.lon;
  qreal lat = area_data->geo_bounds.max_lat - y / area_data->koeff.lon;
  
//  qreal x = area_data->area_curr_size.width() - (area_data->geo_bounds.max_lon - lon) * area_data->koeff.lon;
//  qreal y = (area_data->geo_bounds.max_lat - lat) * area_data->koeff.lat;
  return QPointF(lon, lat);
}



area::SvArea::SvArea(QWidget *parent) :
  QWidget(parent)
{
  setParent(parent);
  setObjectName(QStringLiteral("widgetArea"));
    
  hlayMain = new QHBoxLayout(this);
  hlayMain->setSpacing(2);
  hlayMain->setContentsMargins(11, 11, 11, 11);
  hlayMain->setObjectName(QStringLiteral("hlayMain"));
  hlayMain->setContentsMargins(2, 2, 2, 2);
  
  /* левая панель с кнопками */
  frameLeft = new QFrame(this);
  frameLeft->setObjectName(QStringLiteral("frameLeft"));
  frameLeft->setFrameShape(QFrame::Box);
  frameLeft->setFrameShadow(QFrame::Sunken);
  vlayFrameLeft = new QVBoxLayout(frameLeft);
  vlayFrameLeft->setSpacing(6);
  vlayFrameLeft->setContentsMargins(11, 11, 11, 11);
  vlayFrameLeft->setObjectName(QStringLiteral("vlayFrameLeft"));
  vlayFrameLeft->setContentsMargins(2, 2, 2, 2);
  
  buttonsLeft.append(new AreaButton(frameLeft, bntAlignToLeftTop, "", "bnAlignToLeftTop", QSize(40, 40), QIcon(":/buttons/Icons/Fullscreen.ico")));
  buttonsLeft.append(new AreaButton(frameLeft, bntDropBeacon, "", "bnDropBeacon", QSize(40, 40), QIcon(":/buttons/Icons/Download.ico")));
  buttonsLeft.append(new AreaButton(frameLeft, bntReadSocket, "", "bnReadSocket", QSize(40, 40), QIcon(":/buttons/Icons/Globe.ico")));
  buttonsLeft.last()->setCheckable(true);
  buttonsLeft.append(new AreaButton(frameLeft, bntAddBeacon, "", "bnAddBeacon", QSize(40, 40), QIcon(":/buttons/Icons/AddBeacon.ico")));
  buttonsLeft.last()->setCheckable(true);
  buttonsLeft.append(new AreaButton(frameLeft, bntTrackAirplane, "", "bnTrackAirplane", QSize(40, 40), QIcon(":/buttons/Icons/Link.ico")));
  buttonsLeft.last()->setCheckable(true);
  buttonsLeft.append(new AreaButton(frameLeft, bntCenterOnAirplane, "", "bnCenterOnAirplane", QSize(40, 40), QIcon(":/buttons/Icons/Plane.ico")));
  buttonsLeft.append(new AreaButton(frameLeft, bntNone, "o", "bnLeft5", QSize(40, 25)));
  
  foreach (AreaButton* button, buttonsLeft) {
    vlayFrameLeft->addWidget(button);
    connect(button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
  }
  
  /* правая панель с кнопками */
  frameRight = new QFrame(this);
  frameRight->setObjectName(QStringLiteral("frameRight"));
  frameRight->setFrameShape(QFrame::Box);
  frameRight->setFrameShadow(QFrame::Sunken);
  vlayFrameRight = new QVBoxLayout(frameRight);
  vlayFrameRight->setSpacing(6);
  vlayFrameRight->setContentsMargins(11, 11, 11, 11);
  vlayFrameRight->setObjectName(QStringLiteral("vlayFrameRight"));
  vlayFrameRight->setContentsMargins(2, 2, 2, 2);
  
  buttonsRight.append(new AreaButton(frameRight, bntZoomIn, "", "bnZoomIn", QSize(40, 40), QIcon(":/buttons/Icons/ZoomIn.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntZoomOriginal, "", "bnZoomOriginal", QSize(40, 40), QIcon(":/buttons/Icons/Search.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntZoomOut, "", "bnZoomOut", QSize(40, 40), QIcon(":/buttons/Icons/ZoomOut.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntMoveUp, "", "bnMoveUp", QSize(40, 40), QIcon(":/buttons/Icons/Up.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntMoveDown, "", "bnMoveDown", QSize(40, 40), QIcon(":/buttons/Icons/Down.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntMoveRight, "", "bnMoveRight", QSize(40, 40), QIcon(":/buttons/Icons/Right.ico")));
  buttonsRight.append(new AreaButton(frameRight, bntMoveLeft, "", "bnMoveLeft", QSize(40, 40), QIcon(":/buttons/Icons/Left.ico")));
  
  
  foreach (AreaButton* button, buttonsRight) {
    vlayFrameRight->addWidget(button);
    connect(button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
  }
 
  /* виджет карты */
  widgetMap = new QWidget(this);
  widgetMap->setObjectName(QStringLiteral("widgetMap"));
  widgetMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  widgetMap->setStyleSheet(QStringLiteral("background-color: rgb(255, 250, 255);"));
 
  hlayMain->addWidget(frameLeft);
  hlayMain->addWidget(widgetMap);
  hlayMain->addWidget(frameRight);
}

void area::SvArea::setUp(QString areaName)
{  
  qDebug() << _area_data.geo_bounds.max_lat << _area_data.geo_bounds.max_lon << _area_data.geo_bounds.min_lat << _area_data.geo_bounds.min_lon;
  
  _area_data.area_name = areaName;
  
  // сколько градусов в 1ом метре вдоль долготы
//  qDebug() << "area";
  qreal dst = geo::lat1_lat2_distance(_area_data.geo_bounds.min_lat, _area_data.geo_bounds.max_lat, 
                                      (_area_data.geo_bounds.max_lon + _area_data.geo_bounds.min_lon) / 2.0);
  
  _area_data.lon_angles_in_1m = (_area_data.geo_bounds.max_lat - _area_data.geo_bounds.min_lat) / dst / 1000.0;
  
//  qDebug() << "min_lat max_lat dst" << _area_data.geo_bounds.min_lat << _area_data.geo_bounds.max_lat << dst;                          
  
  // сколько градусов в 1ом метре вдоль широты
  _area_data.lat_angles_in_1m = (_area_data.geo_bounds.max_lon - _area_data.geo_bounds.min_lon) / 
                                geo::lon1_lon2_distance(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lon, 
                                                        (_area_data.geo_bounds.max_lat + _area_data.geo_bounds.min_lat) / 2) / 1000.0;
  
  qDebug() << "lon_a lat_a:" << _area_data.lon_angles_in_1m << _area_data.lat_angles_in_1m; 
  
  /* определяем размер экрана */
  QScreen *scr = QGuiApplication::primaryScreen();
  _area_data.area_base_size = scr->availableSize();
  
  qreal lon_lat = (_area_data.geo_bounds.max_lon - _area_data.geo_bounds.min_lon) / (_area_data.geo_bounds.max_lat - _area_data.geo_bounds.min_lat);
  qreal lat_lon = (_area_data.geo_bounds.max_lat - _area_data.geo_bounds.min_lat) / (_area_data.geo_bounds.max_lon - _area_data.geo_bounds.min_lon);
  
  qreal lon2lon = geo::lon1_lon2_distance(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lon, (_area_data.geo_bounds.max_lat + _area_data.geo_bounds.min_lat) / 2);
  qreal lat2lat = geo::lat1_lat2_distance(_area_data.geo_bounds.min_lat, _area_data.geo_bounds.max_lat, (_area_data.geo_bounds.max_lon + _area_data.geo_bounds.min_lon) / 2);
  
//  _area_data.koeff2 = lon2lon / lat2lat > 1 ? lat2lat / lon2lon : lon2lon / lat2lat;
  
  /* если ширина карты больше высоты, то задаем ширину, а высоту подгоняем */
  if(lon2lon / lat2lat > 1) {
    
//    qDebug() << lon2lon << lat2lat << _area_data.koeff2;
    _area_data.area_base_size.setWidth(_area_data.area_base_size.width() - 350);
    _area_data.area_base_size.setHeight(_area_data.area_base_size.width() * (lat2lat / lon2lon)); 
    
//    _area_data.koeff2
    
//    qreal k = qreal(_area_data.area_base_size.width()) / 256.0;
    
//    qreal lon2lon = geo::lon1_lon2_distance(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lon, (_area_data.geo_bounds.max_lat + _area_data.geo_bounds.min_lat) / 2);
//    qreal lat2lat = geo::lon1_lon2_distance(_area_data.geo_bounds.min_lat, _area_data.geo_bounds.max_lat, (_area_data.geo_bounds.max_lon + _area_data.geo_bounds.min_lon) / 2);
    
//    /* высоту карты определяем исходя из расчетов для проекции меркатора используя формулу (29) отсюда
//     * http://kadastrua.ru/kartografiya/344-ravnougolnaya-tsilindricheskaya-proektsiya.html 
//     * для более точных вычислений можно использовать формулу (30). для вычисления U использовать формулу (22) отсюда
//     * http://kadastrua.ru/kartografiya/343-kartograficheskie-proektsii-obzornykh-kart.html  */
//    qreal mod=0.4343;
//    qreal R = 6356863;  // радиус земли в метрах
    
//    qreal x1 = _area_data.area_base_size.width() / M_PI * (qDegreesToRadians(_area_data.geo_bounds.min_lon) + M_PI);
//    qreal x2 = _area_data.area_base_size.width() / M_PI * (qDegreesToRadians(_area_data.geo_bounds.max_lon) + M_PI);
    
//    qreal w = qAbs(x1 - x2);
//    qreal dw = _area_data.area_base_size.width() / w;
    
//    /* т.к. формула позволяет вычислить длину меридиана от  широты 0 до X, то вычисляем два значения и находим разницу */
//    qreal tan1 = qTan(M_PI / 4 + qDegreesToRadians(_area_data.geo_bounds.max_lat) / 2.0);
//    qreal tan2 = qTan(M_PI / 4 + qDegreesToRadians(_area_data.geo_bounds.min_lat) / 2.0);
    
//    qreal y1 = /*(1.0 / mod) * R **/ _area_data.area_base_size.width() / M_PI * (M_PI - qLn(tan1)); ///3.0;
//    qreal y2 = /*(1.0 / mod) * R **/ _area_data.area_base_size.width() / M_PI * (M_PI - qLn(tan2)); ///3.0;
    
//    qreal h = qAbs(y1 - y2); // расстояние в метрах, между минимальной и максимальной широтами
    
    
//    qDebug() << "lon2lon(km): " << lon2lon << "lat2lat(km):" << lat2lat 
//             << "x1:" << x1 << "x2:" << x2 << "y1:" << y1 << "y2:" << y2 
//             << "w:" << w << "h:" << h 
//             << "base_size.width:" << _area_data.area_base_size.width() << "dw:" << dw;
    
//    _area_data.area_base_size.setHeight(h * dw);
    
//    _area_data.koeff2 = w/h; 
    
    
//    _area_data.area_curr_size = _area_data.area_base_size; // Width(_area_data.area_base_size.width());
//    _area_data.area_curr_size.setHeight(_area_data.area_base_size.height());
    
  }
  else {/* иначе, если высота карты больше ширины, то задаем высоту, а ширину подгоняем */

//    _area_data.area_base_size.setHeight(scr->availableSize().height() - 150);
    
//    qreal x1 = _area_data.area_base_size.height() / M_PI * (qDegreesToRadians(_area_data.geo_bounds.min_lon) + M_PI);
//    qreal x2 = _area_data.area_base_size.height() / M_PI * (qDegreesToRadians(_area_data.geo_bounds.max_lon) + M_PI);
    
//    qreal w = qAbs(x1 - x2);
//    qreal dw = _area_data.area_base_size.width() / w;
    
//    /* т.к. формула позволяет вычислить длину меридиана от  широты 0 до X, то вычисляем два значения и находим разницу */
//    qreal tan1 = qTan(M_PI / 4 + qDegreesToRadians(_area_data.geo_bounds.max_lat) / 2.0);
//    qreal tan2 = qTan(M_PI / 4 + qDegreesToRadians(_area_data.geo_bounds.min_lat) / 2.0);
    
//    qreal y1 = /*(1.0 / mod) * R **/ _area_data.area_base_size.height() / M_PI * (M_PI - qLn(tan1)); ///3.0;
//    qreal y2 = /*(1.0 / mod) * R **/ _area_data.area_base_size.height() / M_PI * (M_PI - qLn(tan2)); ///3.0;
    
//    qreal h = qAbs(y1 - y2); // расстояние в метрах, между минимальной и максимальной широтами
    
    
////    qDebug() << "lon2lon(km): " << lon2lon << "lat2lat(km):" << lat2lat 
////             << "x1:" << x1 << "x2:" << x2 << "y1:" << y1 << "y2:" << y2 
////             << "w:" << w << "h:" << h 
////             << "base_size.width:" << _area_data.area_base_size.width() << "dw:" << dw;
    
//    _area_data.area_base_size.setWidth(h * dw);
    
//    _area_data.koeff2 = w/h;
    
    
    /**
    _area_data.area_base_size.setWidth(scr->availableSize().height() * lon_lat);
    
    _area_data.area_curr_size.setHeight(_area_data.area_base_size.height());
    _area_data.area_curr_size.setWidth(_area_data.area_base_size.height() * lon_lat);
    **/
  }
  
  
  scene = new SvAreaScene(&_area_data);
  view = new SvAreaView(widgetMap, &_area_data);
  view->setAreaScene(scene);
  
  /* задаем масштаб */
  _area_data.scale = 1;
  
  /* подбираем шаг сетки */
//  getGridStep(&_area_data);

  connect(view, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  connect(view, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
  connect(view, SIGNAL(mouseReleased(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
  
  setScale(_area_data.scale);
  
//  udp = new SvUdpReader("172.16.4.106", 35600, this);
////  connect(udp, SIGNAL(), socker, SLOT(terminate())
//  connect(udp, SIGNAL(dataReaded(qreal,qreal,int)), this, SLOT(trackAirplane(qreal,qreal,int)));
  
//  _trackPen.setColor(QColor(30, 130, 230, 255));
  _trackBrush.setColor(QColor(170, 85, 0, 170));
  _trackBrush.setStyle(Qt::SolidPattern);
  _trackPen.setStyle(Qt::NoPen); // SolidLine);
  _track_secs = _track_time.hour() * 3600 + _track_time.minute() * 60 + _track_time.second();
  
//  connect(this, SIGNAL(editing(bool)), this, SLOT(editing(bool)));
  
}

area::SvArea::~SvArea()
{
//  view->~SvAreaView();
//  scene->~SvAreaScene();
  
  AppParams::saveWindowParams(this, size(), pos(), windowState(), "AREA_" + _area_data.area_name);
  
  deleteLater();
}


void area::SvArea::trackAirplane(qreal lon, qreal lat, int angle)
{
  /**
  QPointF newPoint = geo2point(&_area_data, lon, lat);
  
  SvMapObjectRadius* mapRadius = nullptr;
  SvMapObjectAirplane* airplane = nullptr;
//  qreal min_distance = 0xFFFFFFFF;
  
  foreach(SvMapObject *obj, scene->mapObjects()) {
    
    switch (obj->type()) {
      
      case motAirplane: 
        
        airplane = (SvMapObjectAirplane*)obj;
        airplane->setGeo(lon, lat);
        airplane->setPos(newPoint);
//        qDebug() << "air"<< newPoint;
        airplane->setAngle(angle);
        
        break;
        
//      case motBeaconPlanned: // находим ближайший буй
//      {
//        qreal ab = qSqrt(qPow(newPoint.x() - obj->x(), 2.0) + qPow(newPoint.y() - obj->y(), 2.0));
////        if(qIsNaN(ab) || qIsInf(ab))
////          qDebug() << QString("newPoint.x()=%1  obj->x()=%2  newPoint.y()=%3  obj->y()=%4")
////                      .arg(newPoint.x()).arg(obj->x()).arg(newPoint.y()).arg(obj->y());
//        if(ab < min_distance)
//        {
//          min_distance = ab;
//          nearestBeacon = (SvMapObjectBeaconPlanned*)obj;
//        }
        
//        break;
//      }
        
      case motRadius:
        
        mapRadius = (SvMapObjectRadius*)obj;
        mapRadius->setPos(newPoint);
        
        break;
    }
  }
//  qreal distance = geo1_geo2_distance(lon, lat, nearestBeacon->lon(), nearestBeacon->lat());
  // находим ближайший буй
  SvMapObjectBeaconPlanned* nearestBeacon = findNearestPlanned(newPoint);
  if(airplane && mapRadius && nearestBeacon)
    mapRadius->setup(airplane, nearestBeacon); // nearestBeacon->pos().x(), nearestBeacon->pos().y(), distance, angle);
  
  // новая точка пути 
  SvMapObjectDirection* dir = new SvMapObjectDirection(this, lon, lat);
  dir->setPos(newPoint);
  dir->setAngle(angle);
  dir->setBrush(QBrush(QColor(0, 255, 100, 200))); //_trackBrush
  dir->setPen(QColor(0, 255, 100, 200).dark(), Qt::SolidLine, 1);
  scene->addMapObject(dir);
  
  _trackPoints.insert(geo::POSITION(lon, lat, angle, QDateTime::currentDateTime()), dir); // scene->addEllipse(newPoint.x(), newPoint.y(), 3, 3, _trackPen, _trackBrush));
  
  // лишние точки убираем 
  foreach (geo::POSITION geopos, _trackPoints.keys()) {
    if(geopos.utc().secsTo(QDateTime::currentDateTime()) > _track_secs)
    {
      scene->removeMapObject(_trackPoints.value(geopos)); // MapObject(_trackPoints.value(geo));
      _trackPoints.value(geopos)->~QGraphicsItem();
      _trackPoints.remove(geopos);      
    }
  }
  
  if(_trackAirplane)
    centerAirplane();
  **/
  scene->update();
}

SvMapObjectBeaconPlanned* area::SvArea::findNearestPlanned(QPointF fromPoint)
{
  SvMapObjectBeaconPlanned* nearestBeacon = nullptr;
  qreal min_distance = 0xFFFFFFFF;
  
  foreach(SvMapObject *obj, scene->mapObjects()) {
    if(obj->type() == motBeaconPlanned) {
      qreal ab = qSqrt(qPow(fromPoint.x() - obj->x(), 2.0) + qPow(fromPoint.y() - obj->y(), 2.0));

      if(ab < min_distance) {
        min_distance = ab;
        nearestBeacon = (SvMapObjectBeaconPlanned*)obj;
      }
    }
  }
  
  return nearestBeacon;
}

bool area::SvArea::readBounds(QString &fileName)
{
  
  /* читаем xml */
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Ошибка чтения XML"),
                           tr("Не удается прочитать файл %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return false;
  }
  
  QXmlStreamReader xml;
  xml.setDevice(&file);
  
  bool result = true;
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "bounds")
    {
      bool k;
      _area_data.geo_bounds.min_lat = xml.attributes().value("minlat").toDouble(&k); result &= k;
      _area_data.geo_bounds.max_lat = xml.attributes().value("maxlat").toDouble(&k); result &= k;
      _area_data.geo_bounds.min_lon = xml.attributes().value("minlon").toDouble(&k); result &= k;
      _area_data.geo_bounds.max_lon = xml.attributes().value("maxlon").toDouble(&k); result &= k;
      
      break;
    }
  }
  
  return result;
}

bool area::SvArea::readMap(QString &fileName)
{
  bool result = false;
  
  /* читаем xml */
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Ошибка чтения XML"),
                           tr("Не удается прочитать файл %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return false;
  }
  
  QXmlStreamReader xml;
  xml.setDevice(&file);
  
  /* читаем все точки */
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "node")
    {
      bool o = true; bool k;
      quint64 node_id = xml.attributes().value("id").toLongLong(&k); o &= k;
      qreal lon = xml.attributes().value("lon").toDouble(&k); o &= k;
      qreal lat = xml.attributes().value("lat").toDouble(&k); o &= k;
      
      if(!o) {
        qDebug() << "wrong node" << xml.attributes().value("id");
        continue;
      }
      
      _area_data.NODES.insert(node_id, qMakePair(lon, lat));
    }
  }
  xml.clear();

  /* читаем пути */
  file.reset();
  file.flush();
  xml.setDevice(&file);
  
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "way")
    {
      bool ok;

      quint64 way_id = xml.attributes().value("id").toLongLong(&ok);
      
      if(!ok) {
        qDebug() << "wrong way" << xml.attributes().value("id");
        continue;
      }
      
      xml.readNextStartElement();
      
      QList<QPair<qreal, qreal>> nodes_list;
      nodes_list.clear();
      
      while(!((xml.isEndElement() && xml.name() == "way") || xml.atEnd()))
      {
        if(xml.isStartElement() && xml.name() == "nd")
        {
          quint64 node_id = xml.attributes().value("ref").toLongLong(&ok);
          if(!ok)
            qDebug() << "wrong node reference" << xml.attributes().value("id");
          
          else
            nodes_list.append(_area_data.NODES.value(node_id));

        }
        xml.readNextStartElement();
        
      }
      _area_data.WAYS.insert(way_id, nodes_list);

    }
  }

  if (xml.hasError())
  {
    return false;
  }
  
  return true;
}

void area::SvArea::buttonPressed()
{
  AreaButton* button = (AreaButton*)(sender());
  
  switch (button->type()) {
    
    case bntZoomIn:
      scaleInc();
      break;
      
    case bntZoomOut:
      scaleDec();
      break;
      
    case bntZoomOriginal:
      _area_data.scale = 1;
      setScale(1);
      view->move(0, 0);
      break;
    
    case bntMoveLeft:
      moveHorizontal(-25);
      break;
      
    case bntMoveRight:
      moveHorizontal(25);
      break;
      
    case bntMoveUp:
      moveVertical(-25);
      break;
      
    case bntMoveDown:
      moveVertical(25);
      break;
      
    case bntAlignToLeftTop:
      view->move(0, 0);
//      view->repaint();
      break;
      
    case bntAddBeacon:
    {
      foreach (AreaButton *btn, buttonsLeft) {
        btn->setEnabled(_editMode || (btn == button));
      }
      
      button->setChecked(_editMode);
      button->setIcon(!_editMode ? QIcon(":/buttons/Icons/Save.ico") : QIcon(":/buttons/Icons/Pen.ico"));
      
      _editMode = !_editMode;
//      qDebug() << hint;
      if(hint) {
        hint->deleteLater();
        hint = nullptr;
      }
      else {
        hint = new QLabel("Щелкните на карте для добавления нового буя", nullptr, Qt::ToolTip);
        hint->move(QApplication::activeWindow()->pos() + QPoint(QApplication::activeWindow()->width() / 2 - 150, 100));
        hint->resize(300, 30);
        hint->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        hint->setStyleSheet("background-color: rgb(255, 255, 127);");
        hint->setVisible(true);
      }
      
      break;
     }
      
    case bntReadSocket:
      
//      if(udp->isActive()) {
//        udp->stop();
        
//        while(udp->isActive()) QApplication::processEvents();
//      }
      
//      else 
//        udp->start();
      
//      button->setChecked(_socketIsActive);
      
//      _socketIsActive = !_socketIsActive;
//      qDebug() << "udp socket is NOT active" << _socketIsActive;

      break;
      
    case bntCenterOnAirplane:
      centerAirplane();
      break;
      
    case bntTrackAirplane:
      button->setChecked(_trackAirplane);
      _trackAirplane = !_trackAirplane;
      break;
      
     
    default:
      QMessageBox::information(this, "OGO", "Не назначено", QMessageBox::Ok);
      break;
  }
}

void area::SvArea::editing(bool editMode)
{
  switch (editMode) {
    case true:
      
      break;
      
    default:
      break;
  }
}

void area::SvArea::moveVertical(int val)
{
  view->move(view->x(), view->y() + val);
}

void area::SvArea::moveHorizontal(int val)
{
  view->move(view->x() + val, view->y());
}

void area::SvArea::mousePressed(QMouseEvent * event)
{
//  if(!event->modifiers().testFlag(Qt::ShiftModifier))
//    return;

  _mouseButton = event->button();
  _mousePos = event->pos();
  
//  if(_editMode && (_mouseButton == Qt::LeftButton))
//  {
//    QPointF coord = point2geo(&_area_data, _mousePos.x(), _mousePos.y());
    
//    BEACONEDITOR_UI = new SvBeaconEditor(this, -1, coord.x(), coord.y());
//    if(BEACONEDITOR_UI->exec() == QDialog::Accepted)
//    {
//      SvMapObjectBeaconPlanned* beacon = new SvMapObjectBeaconPlanned(this);
//      beacon->setId(BEACONEDITOR_UI->t_id);
//      beacon->setGeo(BEACONEDITOR_UI->t_lon, BEACONEDITOR_UI->t_lat);
//      beacon->setUid(BEACONEDITOR_UI->t_uid);
//      beacon->setDateTime(BEACONEDITOR_UI->t_date_time);
      
//      scene->addMapObject(beacon);
//      setScale(_area_data.scale);      
//    }
//    BEACONEDITOR_UI->~SvBeaconEditor();
    
//  }
}

void area::SvArea::mouseReleased(QMouseEvent * event)
{
  _mouseButton = Qt::NoButton;
}

void area::SvArea::mouseMoved(QMouseEvent * event)
{
  switch (_mouseButton) {
    case Qt::NoButton:
      break;
      
    case Qt::LeftButton:
      view->move((view->x() + event->pos().x() - _mousePos.x()), view->y() + event->pos().y() - _mousePos.y());
      break;
  }

  QWidget::mouseMoveEvent(event);
  
}

void area::SvArea::scaleInc()
{
  if(_area_data.scale >= 16)
    return;
  
  _area_data.scale *= 1.25;
  setScale(_area_data.scale);
  
  if(_trackAirplane)
    centerAirplane();
}

void area::SvArea::scaleDec()
{
  if(_area_data.scale <= 0.5)
    return;
  
  _area_data.scale /= 1.25;
  setScale(_area_data.scale);

  if(_trackAirplane)
    centerAirplane();
}

void area::SvArea::setScale(qreal scale)
{
  _area_data.area_curr_size.setWidth(_area_data.area_base_size.width() * _area_data.scale);
  _area_data.area_curr_size.setHeight(_area_data.area_base_size.height() * _area_data.scale);
  
  _area_data.gridYstep = MINOR_VGRID_DISTANCE / scale;
  _area_data.gridXstep = MINOR_VGRID_DISTANCE / scale;
  
//  qDebug() << _area_data.area_curr_size.width() << _area_data.area_curr_size.height();
  _area_data.koeff.lon = qreal(_area_data.area_curr_size.width())  / (_area_data.geo_bounds.max_lon - _area_data.geo_bounds.min_lon);
  _area_data.koeff.lat = qreal(_area_data.area_curr_size.height()) / (_area_data.geo_bounds.max_lat - _area_data.geo_bounds.min_lat);
//  qDebug() << _area_data.area_curr_size.height() << _area_data.area_curr_size.width() << _area_data.koeff.lon << _area_data.koeff.lat;
  
  qreal m2p = geo::meridian2parallel_km_in_1degree((_area_data.geo_bounds.max_lon + _area_data.geo_bounds.min_lon) / 2, (_area_data.geo_bounds.max_lat + _area_data.geo_bounds.min_lat) / 2);
  qDebug() << "m2p" << m2p;
//  if(m2p > 1) _area_data.koeff.lon *= m2p;
//  else _area_data.koeff.lat *= m2p;
  
  /* подбираем шаг сетки */
  updateGridStep();
  
  /* обновляем сцену */
  scene->setSceneRect(QRectF(0, 0, _area_data.area_curr_size.width(), _area_data.area_curr_size.height()));

  /* квадратики по углам */
  qreal cs = _area_data.gridCellStep;
  scene->ltsq->setPos(2, 2);                                                                                     
  scene->rtsq->setPos(_area_data.area_curr_size.width() - cs - 2, 2);                                           
  scene->lbsq->setPos(2, _area_data.area_curr_size.height() - cs - 2);                                          
  scene->rbsq->setPos(_area_data.area_curr_size.width() - cs - 2, _area_data.area_curr_size.height() - cs - 2);

  view->resize(_area_data.area_curr_size.width(), _area_data.area_curr_size.height());
  
  
  // пересчитываем координаты всех объектов 
  foreach (SvMapObject* item, scene->mapObjects()) {
    const QPointF p = geo2point(&_area_data, item->geoPosition().longtitude, item->geoPosition().latitude);
    item->setPos(p);
  }
  
}

void area::SvArea::centerAirplane()
{
  /* центруем экран на свое судно */
  QPointF self_pos;
  foreach (SvMapObject* item, scene->mapObjects()) {
    if(item->type() == motSelfVessel) {
      self_pos = item->pos();
      break;
    }
  }
  
  /* находим середину виджета */
  qreal xc = this->parentWidget()->width() / 2;
  qreal yc = this->parentWidget()->height() / 2;
  
  view->move(xc - self_pos.x(), yc - self_pos.y());
}

void area::SvArea::updateGridStep()
{
  // находим расстояние до следующей линии на карте с учетом масштаба, шага сетки
  // для горизонтальных линий направление 180 градусов
  geo::GEOPOSITION pos = geo::GEOPOSITION(_area_data.geo_bounds.max_lat, _area_data.geo_bounds.min_lon, 180, 0);
  pos = geo::get_next_geoposition(pos, MINOR_VGRID_DISTANCE / _area_data.scale);
  
  _area_data.gridYstep = geo2point(&_area_data, pos.longtitude, pos.latitude).y();
  
  // для вертикальных линий направление 90 градусов
  pos = geo::GEOPOSITION(_area_data.geo_bounds.max_lat, _area_data.geo_bounds.min_lon, 90, 0);
  pos = geo::get_next_geoposition(pos, MINOR_VGRID_DISTANCE / _area_data.scale);
    
  _area_data.gridXstep = geo2point(&_area_data, pos.longtitude, pos.latitude).x();
    
    
  /**
  // ширина области в метрах 
  qreal area_width_meters = 1000 * geo::lon1_lon2_distance(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lon, (_area_data.geo_bounds.min_lat + _area_data.geo_bounds.max_lat) / 2.0);

  int line_count = area_width_meters / (MINOR_VGRID_DISTANCE * _area_data.scale);
  _area_data.gridCellStep = quint64(trunc(_area_data.area_curr_size.width() / line_count));
**/
    
  /**
  qreal area_meters = 1000 * geo::lat1_lat2_distance(_area_data.geo_bounds.min_lat, _area_data.geo_bounds.max_lat, _area_data.geo_bounds.max_lon);

  int line_count = _area_data.area_curr_size.width() / MINOR_LINE_INTERVAL;
  _area_data.gridCellDistance = quint64(trunc(area_meters / line_count));
  
  while(_area_data.gridCellDistance % 50)
    _area_data.gridCellDistance++;
  
  _area_data.gridCellStep = _area_data.gridCellDistance * _area_data.area_curr_size.width() / area_meters;
  **/
}

/** ****** AREA SCENE ****** **/
area::SvAreaScene::SvAreaScene(AREA_DATA *area_data)
{
  _area_data = area_data;
  
  qreal cs = _area_data->gridCellStep;
 
//  lttxt = addText("1");
//  rttxt = addText("2");
//  lbtxt = addText("3");
//  rbtxt = addText("4");
  
  rbsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(2, 2, cs, cs);
  ltsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(_area_data->area_curr_size.width() - cs - 4, 2, cs, cs);                                           
  rtsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(2, _area_data->area_curr_size.height() - cs - 4, cs, cs);                                       
  lbsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(_area_data->area_curr_size.width() - cs - 4, _area_data->area_curr_size.height() - cs - 4, cs, cs);
      
//  resizeScene();
}

void area::SvAreaScene::setMapObjectPos(SvMapObject* mapObject, const geo::GEOPOSITION& geopos)
{
  QPointF new_pos = geo2point(_area_data, geopos.longtitude, geopos.latitude);
  mapObject->setPos(new_pos.x()/* * _area_data->koeff2*/, new_pos.y());
  mapObject->setRotation(geopos.course);
}

/** ****** AREA VIEW ******* **/
area::SvAreaView::SvAreaView(QWidget *parent, AREA_DATA *area_data) :
  QGraphicsView(parent)
{
  _area_data = area_data;
  
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
//  this->setAlignment(Qt::AlignTop | Qt::AlignLeft); //!!
//  this->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    
  setRenderHint(QPainter::Antialiasing, true);
//  this->setDragMode(QGraphicsView::ScrollHandDrag); //  ScrollHandDrag RubberBandDrag  NoDrag);
  setOptimizationFlags(QGraphicsView::DontSavePainterState);
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  setStyleSheet("background-color: rgb(255, 255, 250)");
  setFocusPolicy(Qt::ClickFocus);
  
  _gridBorderColor = QColor(0, 0, 0, 255);
  _gridMinorColor = QColor(0, 0, 255, 30);
  _gridMajorColor = QColor(0, 0, 255, 50);
  _mapCoastColor = QColor(0, 85, 127, 150);
  _fontColor = QColor(0, 0, 255, 200);

  _penBorder.setColor(_gridBorderColor);
  _penBorder.setStyle(Qt::SolidLine);
  _penBorder.setWidth(1);

  _penMajorLine.setColor(_gridMajorColor);
  _penMajorLine.setStyle(Qt::DashLine);
  _penMajorLine.setWidth(1);

  _penMinorLine.setColor(_gridMinorColor);
  _penMinorLine.setStyle(Qt::DotLine);
  _penMinorLine.setWidth(1);

  _pen_coastLine.setColor(_mapCoastColor);
  _pen_coastLine.setStyle(Qt::SolidLine);
  _pen_coastLine.setWidth(2);  
  
  setVisible(true);
  
}


void area::SvAreaView::drawBackground(QPainter *painter, const QRectF &r)
{
  painter->setPen(_pen_coastLine);
  
  foreach(quint64 way, _area_data->WAYS.keys())
  {
    QList<QPair<qreal, qreal>> nodes = _area_data->WAYS.value(way);
    
    QPainterPath path;
    
    /* пересчитываем географические в экранные координаты */
    QPointF p0 = geo2point(_area_data, nodes.value(0).first, nodes.value(0).second);
    path.moveTo(p0);
    
    for(int i = 1; i < nodes.count(); i++) {
      
      QPointF p1 = geo2point(_area_data, nodes.value(i).first, nodes.value(i).second);
      path.lineTo(p1);

    }
    
    painter->drawPath(path);

  }
  
  painter->setPen(_penBorder);
  painter->setBrush(QBrush());
  painter->drawRect(QRect(0, 0, _area_data->area_curr_size.width(), _area_data->area_curr_size.height()));
  
  /* вертикальные линии */
  int i = 0;
  qreal x = 0; //_area_data->geo_bounds.min_lon;
  
  while(x < _area_data->area_curr_size.width()) { 
    
    painter->setPen((i % 5 == 0) ? _penMajorLine : _penMinorLine);
    painter->drawLine(x, 0, x, _area_data->area_curr_size.height());
    
    i ++;
    x += _area_data->gridXstep;
 
  }

  /* горизонтальные линии */
  i = 0;
  qreal y = 0;
  
  while(y < _area_data->area_curr_size.height()) {
    
    painter->setPen((i % 5 == 0) ? _penMajorLine : _penMinorLine);
    painter->drawLine(0, y, _area_data->area_curr_size.width(), y);
    
    i ++;
    y += _area_data->gridYstep;
    
  }

  /* рисуем шкалу масштаба */
//  painter->setPen(Qt::NoPen);
  int font_size = 9;
  painter->setPen(_penBorder);
//  painter->setBrush(QBrush(QColor(255, 255, 255, 150)));
//  painter->drawRect(_area_data->gridCellStep * 1.5, _area_data->gridCellStep * 1.5, _area_data->gridCellStep * 5.5, _area_data->gridCellStep * 0.5 + font_size + 10);
  
  QPainterPath path;
  
  qreal x1 = _area_data->gridCellStep;
  qreal y1 = _area_data->gridCellStep * 2;
  qreal y2 = _area_data->gridCellStep * 2 + 4;

  path.moveTo(x1 * 2, y1);
  path.lineTo(x1 * 6, y1);
  
  for(int i = 2; i < 7; i++){
    path.moveTo(x1 * i, y1);
    path.lineTo(x1 * i, y2);
  }

  path.addText(x1 * 2, y2 + 10, QFont("Courier New", font_size), QString("%1 м.").arg(_area_data->gridCellDistance * 5));
  
  painter->drawPath(path);
  
  
}

void area::SvAreaView::mousePressEvent(QMouseEvent * event)
{
  emit mousePressed(event);
  QGraphicsView::mousePressEvent(event);
}

void area::SvAreaView::mouseReleaseEvent(QMouseEvent * event)
{
  emit mouseReleased(event);
  QGraphicsView::mouseReleaseEvent(event);
}

void area::SvAreaView::mouseMoveEvent(QMouseEvent * event)
{
  emit mouseMoved(event);
  QGraphicsView::mouseMoveEvent(event);
//  update();
}

/** ******** RULERS ****** **/
area::SvHRuler::SvHRuler(QWidget *parent, float *scale, QSize* areaSize)
{


}

void area::SvHRuler::paintEvent(QPaintEvent * event)
{

}

area::SvVRuler::SvVRuler(QWidget *parent, float *scale, QSize *areaSize)
{

}

void area::SvVRuler::paintEvent(QPaintEvent * event)
{

}
