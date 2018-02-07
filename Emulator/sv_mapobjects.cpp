#include "sv_mapobjects.h"

extern SvSQLITE* SQLITE;
extern SvBeaconEditor* BEACONEDITOR_UI;

SvMapObject::SvMapObject(QWidget* parent, qreal lon, qreal lat)
{
  setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemClipsToShape);
  setAcceptHoverEvents(true);
  
  _lon = lon;
  _lat = lat;
  
  _path = new QPainterPath();
}

SvMapObject::~SvMapObject()
{
  delete _path;
}

void SvMapObject::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
  this->setToolTip(QString("Долгота: %1\n Широта: %2").arg(_lon).arg(_lat));
  this->update();
}

void SvMapObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
  this->update();
}

void SvMapObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(widget);

//  if(!_brush.gradient())
//  {
//    QColor brushColor = option->state & QStyle::State_Selected ? _brush.color().dark(): _brush.color();
//    brushColor = option->state & QStyle::State_MouseOver ? _brush.color().light(): brushColor;
//    _brush.setColor(brushColor);
//  }
//  else
//    painter->setBrush(QBrush(gradient));
  
//  painter->setPen(QPen(_penColor, 2, _penStyle, Qt::RoundCap, Qt::RoundJoin)); 

//  int w = option->state & QStyle::State_MouseOver ? _pen.width() * 2 : 2;_pen.color().dark()
  
//  QColor penColor = option->state & QStyle::State_Selected ? Qt::magenta : _pen.color();
//  penColor = option->state & QStyle::State_MouseOver ? Qt::cyan : penColor;
//  _pen.setColor(penColor);  
//  _pen.setWidth(w);

  if(option->state & QStyle::State_Selected)
    painter->setBrush(QBrush(_selectionColor.dark()/*, _brush.style()*/));
  
  else if(option->state & QStyle::State_MouseOver)
    painter->setBrush(QBrush(_selectionColor.light()/*, _brush.style()*/));
  
  else
    painter->setBrush(_brush);
  
  painter->setPen(_pen);
  
  painter->drawPath(*_path);
      
}

QRectF SvMapObject::boundingRect() const
{
  return QRectF(_path->boundingRect()); /**  !!! **/
}

void SvMapObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
}

void SvMapObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);
}

void SvMapObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
}

void SvMapObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
}

QPainterPath SvMapObject::shape() const
{
  return *_path;
}


/** ************************************************/
/** ************** AIRPLANE ************************/
/** ************************************************/

SvMapObjectAirplane::SvMapObjectAirplane(QWidget *parent, qreal lon, qreal lat):
  SvMapObject(parent, lon, lat)
{
  /* формируем контур фигуры */
  path()->moveTo(points[0]);  
  for(int i = 1; i < 22; i++)
    path()->lineTo(points[i]);
  
}


/** ************************************************/
/** ************** BEACON **************************/
/** ************************************************/

SvMapObjectBeaconAbstract::SvMapObjectBeaconAbstract(QWidget* parent, const qreal lon, const qreal lat,
                                                     const int id, const QString &uid, const QDateTime &dateTime):
  SvMapObject(parent, lon, lat)
{
  _id = id;
  _uid = uid;
  _date_time = dateTime;
}

/** ************** BEACON PLANNED ****************** **/
SvMapObjectBeaconPlanned::SvMapObjectBeaconPlanned(QWidget *parent):
  SvMapObjectBeaconAbstract(parent)
{
  path()->moveTo(_points[0]);
  path()->lineTo(_points[1]);
  path()->lineTo(_points[2]);
  path()->lineTo(_points[0]);
  path()->addEllipse(_points[3], 10, 10);

  path()->setFillRule(Qt::OddEvenFill);
  path()->addEllipse(QPointF(0.0, -16.0), 3, 3);
  
  editAction = new QAction(QIcon(":/buttons/Icons/Pen.ico"), QString("Редактировать"), 0);
  removeAction = new QAction(QIcon(":/buttons/Icons/Cancel.ico"), QString("Удалить"), 0);
}

void SvMapObjectBeaconPlanned::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  BEACONEDITOR_UI = new SvBeaconEditor(nullptr, id());
  if(BEACONEDITOR_UI->exec() == QDialog::Accepted)
  {
 
  }
  BEACONEDITOR_UI->~SvBeaconEditor();

  SvMapObject::mouseDoubleClickEvent(event);

}

void SvMapObjectBeaconPlanned::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QMenu menu;
  menu.addAction(editAction);
  menu.addSeparator();
  menu.addAction(removeAction);
  
  QAction *a = menu.exec(event->screenPos());
  if(removeAction == a)
  {
    QSqlError err = SQLITE->execSQL(QString("delete from plan where id=%1").arg(id()));
    if(err.type() != QSqlError::NoError)
     QMessageBox::critical(0, "Ошибка", err.databaseText(), QMessageBox::Ok);
    
    this->scene()->removeItem(this);
  }
  else if (editAction == a)
  {
    BEACONEDITOR_UI = new SvBeaconEditor(nullptr, id());
    if(BEACONEDITOR_UI->exec() == QDialog::Accepted)
    {
     
    }
    BEACONEDITOR_UI->~SvBeaconEditor();
  }
}

/** ************** BEACON ACTIVE ****************** **/
SvMapObjectBeaconActive::SvMapObjectBeaconActive(QWidget *parent, int number):
  SvMapObjectBeaconAbstract(parent)
{
  path()->moveTo(_points[0]);
  path()->lineTo(_points[1]);
  path()->lineTo(_points[2]);
  path()->lineTo(_points[0]);
  path()->addEllipse(_points[3], 10, 10);
  
  _number = number;
  
  QFont font("Courier New", 8);
  QFontMetrics fm(font);
  QString caption = QString::number(_number);

  _pathNum.addText(-fm.width(caption)/2, -18.0, font, caption); 
  
  gradient = QRadialGradient(_points[3], 20, _points[3]);
  gradient.setColorAt(0, _signalColor);
}

void SvMapObjectBeaconActive::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  gradient.setColorAt(_signalLevel, _defaultColor);
  
  setBrush(QBrush(gradient), _defaultColor);
  SvMapObject::paint(painter, option, widget);

  painter->setPen(QPen(_textColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin)); 
  painter->drawPath(_pathNum);
}


/** ************************************************/
/** ************** RADIUS **************************/
/** ************************************************/

SvMapObjectRadius::SvMapObjectRadius(QWidget *parent, qreal lon, qreal lat):
  SvMapObject(parent, lon, lat)
{
  setFlags(0);
  setAcceptHoverEvents(false);

  /* формируем контур фигуры */
//  _path.moveTo(0, 0); 
//  _path.lineTo(1, 1);
//  _path.addEllipse(0.0, 0.0, 1, 1);

}

//void SvMapObjectRadius::setup(qreal x, qreal y, qreal distance, qreal angle)
void SvMapObjectRadius::setup(SvMapObjectAirplane* airplane, SvMapObjectBeaconPlanned* beacon)
{
  /* расстояние между двумя точками */
  _radius = qSqrt(qFabs(qPow(beacon->pos().x() - this->x(), 2.0) + qPow(beacon->pos().y() - this->y(), 2.0)));
    
//  if(qIsNaN(_radius) || qIsInf(_radius))
//  {
//    qDebug() << QString("this.x=%1  this.y=%2  x=%3  y=%4").arg(this->x()).arg(this->y()).arg(beacon->pos().x()).arg(beacon->pos().y());
//    qDebug() << QString("air.lon=%1  air.lat=%2  beacon.lon=%3  beacon.lat=%4  dist=%5")
//                .arg(airplane->lon()).arg(airplane->lat()).arg(beacon->lon()).arg(beacon->lat()).arg(distance);

//    return;
//  }
  
  _pathCircle = QPainterPath();
  _pathRadius = QPainterPath();
  _pathCourse = QPainterPath();
  _pathText = QPainterPath();
  
  _pathRadius.lineTo(beacon->pos().x() - this->x(), beacon->pos().y() - this->y());
  _pathCircle.addEllipse(-_radius, -_radius, _radius * 2, _radius * 2);
  _pathCourse.lineTo(0, -_radius);
  _course = airplane->angle();
  
  /* выводим расстояние */
  qreal distance = geo1_geo2_distance(airplane->lon(), airplane->lat(), beacon->lon(), beacon->lat());
  QString lbl = distance > 1000 ? QString("%1 км.").arg(distance / 1000.0, 0, 'g', 2) : QString("%1 м.").arg(int(distance));
  _pathText.addText(beacon->pos().x() - this->x(), beacon->pos().y() - this->y() + 12, QFont("Courier New", 10), lbl);

  update();
}

void SvMapObjectRadius::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(widget);

  painter->setBrush(QBrush(Qt::NoBrush));

  painter->setPen(QPen(_penCircleColor, 1, _penCircleStyle)); 
  painter->drawPath(_pathCircle);
  
  painter->setPen(QPen(_penRadiusColor, 1, _penRadiusStyle));
  painter->drawPath(_pathRadius);

  painter->setPen(QPen(_penCourseColor, 1, _penCourseStyle));  
  painter->rotate(_course);
  painter->drawPath(_pathCourse);
  
  painter->setPen(QPen(_penTextColor, 1, _penTextStyle));
  painter->rotate(-_course);
  painter->drawPath(_pathText);
      
}

QRectF SvMapObjectRadius::boundingRect() const
{
  return QRectF(-_radius/2, -_radius/2, _radius/2, _radius/2); /**  !!! **/
}

QPainterPath SvMapObjectRadius::shape() const
{
  return _pathCircle + _pathRadius + _pathText;
}


/** ************************************************/
/** ************** DIRECTION ***********************/
/** ************************************************/

SvMapObjectDirection::SvMapObjectDirection(QWidget* parent, qreal lon, qreal lat):
  SvMapObject(parent, lon, lat)
{
  setFlags(QGraphicsItem::ItemStacksBehindParent);
  setAcceptHoverEvents(false);

  /* формируем контур фигуры */
  path()->moveTo(-3, 2); 
  path()->lineTo(0, -2);
  path()->lineTo(3, 2);
  path()->lineTo(-3, 2);

}


