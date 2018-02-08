#include "sv_vesselsymbol.h"

SvBeackonSymbol::SvBeackonSymbol(area::SvArea *parent,
                                 bool allowEdit,
                                 int id,
                                 QString name,
                                 QPointF mapCoord,
                                 QPointF viewPosition,
                                 int itemType)
{
  this->parent = parent;
  this->id = id;
  this->item_type = itemType;
  this->view_position = viewPosition;
  this->setPos(viewPosition);
  this->map_coord = mapCoord;
  this->isEditable = allowEdit;
  this->name = name;
  
  switch (item_type) {
    case gtBeackon:
    {
      this->setToolTip(QString("Буй\n%1\nX=%2\nY=%3").arg(name).arg(mapCoord.x()).arg(mapCoord.y()));
      brushColor = Qt::blue;
      penColor = Qt::red;
      break;
    }
      
//    case gtRls:
//    {
//      this->setToolTip(QString("РЛС\n%1\nX=%2\nY=%3").arg(device_name).arg(X).arg(Y));
//      brushColor = Qt::green;
//      penColor = Qt::black;
//      break;
//    }
   
  }
 
  if(isEditable) setFlags(ItemIsMovable | ItemIsSelectable);
  else setFlags(ItemIsSelectable);
  
  setAcceptHoverEvents(true);
  
//  QIcon ico = QIcon(":/editors/icons/Close.ico");
//  removeAction = new QAction(ico, QString("Удалить"), 0);
   
//  QIcon ico_1 = QIcon(":/tree32/icons/Gears.ico");
//  editDeviceAction = new QAction(ico_1, QString("Редактировать устройство"), 0);
  
//  QIcon ico_2 = QIcon(":/tree32/icons/Eye.bmp");
//  playVideoAction = new QAction(ico_2, QString("Просмотр"), 0); 
  
//  QIcon ico_3 = QIcon(":/tree32/icons/rls.ico");
//  playRLSAction = new QAction(ico_3, QString("Просмотр"), 0); 
  
//  QIcon ico_4 = QIcon(":/tree32/icons/rls.ico");
//  playRLSOnMapAction = new QAction(ico_4, QString("Отобразить с наложением"), 0);   
  
  
//  QObject::connect(removeAction, removeAction->triggered(), this, delete(this));
}

SvBeackonSymbol::~SvBeackonSymbol()
{
//  p_list->removeAt(id);
//  for(int i = 0; i < this->scene()->items().count(); i++)
//  {
//    if(this->scene()->items().at(i)->type() == UserType + 1)
//    {
//      ZoneNode* zn = qgraphicsitem_cast<ZoneNode*>(this->scene()->items().at(i));
//      if(zn->id > id) zn->id--;      
//    }
//  }
//  this->scene()->update();
//  this->scene()->removeItem(this); /** ураааа, заработало! **/
}

void SvBeackonSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(widget);

  QColor fillColor = option->state & (QStyle::State_MouseOver | QStyle::State_Selected) ? brushColor.light() : brushColor;// fillColor = fillColor.light(125);
  painter->setBrush(QBrush(fillColor));
  QColor pen = option->state & (QStyle::State_MouseOver | QStyle::State_Selected) ? Qt::red : penColor;
  int penWidth = option->state & (QStyle::State_MouseOver | QStyle::State_Selected) ? 2 : 1;
  painter->setPen(QPen(pen, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));  
  
  switch (item_type)
  {
//    case gtVideocamera:
//    {
//      static const QPointF points[8] =
//      {
//        QPointF(-10.0, -8.0),
//        QPointF(10.0, -8.0),
//        QPointF(10.0, -1.0),
//        QPointF(18.0, -8.0),
//        QPointF(18.0, 8.0),
//        QPointF(10.0, 1.0),
//        QPointF(10.0, 8.0),
//        QPointF(-10.0, 8.0)        
//      };
//      painter->drawPolygon(points, 8);
//      break;
//    }
      
    case gtBeackon:
    {
      painter->drawEllipse(view_position.toPoint(), 10, 10);
      break;
    }
  }
}

QRectF SvBeackonSymbol::boundingRect() const
{
  return QRectF(-10, -10, 20, 20); /**  !!! **/
}

void SvBeackonSymbol::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//  qDebug() << "x=" << event->pos().x() << "y=" << event->pos().y();
//  x = event->pos().x();
//  y = event->pos().y();
  QGraphicsItem::mousePressEvent(event);
  update();
}

void SvBeackonSymbol::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
//  QMessageBox::information(0, "dddd", "dsdsdsds", QMessageBox::Ok);
//  qDebug() << id;

  QGraphicsItem::mouseDoubleClickEvent(event);
  update();
}

void SvBeackonSymbol::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//  qDebug() << "x=" << event->scenePos().x() << "y=" << event->scenePos().y();
//  p_list->replace(id, event->scenePos());
  QGraphicsItem::mouseMoveEvent(event);
  this->scene()->update();
}

void SvBeackonSymbol::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  update();
}

void SvBeackonSymbol::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QMenu menu;
//  qDebug() << this->type();
  switch (this->item_type)
  {
    case gtBeackon:
    {
      menu.addAction(playDataAction);
      menu.addSeparator();
      menu.addAction(viewInfoAction);
      menu.addSeparator();
      break;
    }

  }
  
  menu.addAction(editAction);
  menu.addSeparator();
  menu.addAction(removeAction);
  
  QAction *a = menu.exec(event->screenPos());
  
  if(removeAction == a)
  {
    this->~SvBeackonSymbol();
  }
//  else if (editDeviceAction == a)
//  {
//    SvDevice* dd = new SvDevice(0, this->device_name, SvDevice::smEdit);
//  }
//  else if (playRLSAction == a)
//  {
//    qDebug() << "SINGLERLS_UI";
//    SINGLERLS_UI = new SvSingleRlsWindow(8000, this->device_name,
//                                         "Данные РЛС реального времени");
//  }
//  else if (playVideoAction == a)
//  {
//    QSqlQuery* q = new QSqlQuery(PGDB->db);
//    QSqlError err = PGDB->execSQL(QString(D_SELECT_FROM_DEVICES_WHERE_2)
//                                  .arg("device_name")
//                                  .arg("'" + this->device_name + "'"), q);
    
//    //  qDebug() << this->treeModel->itemFromIndex(this->currentIndex())->data().toInt();
//    if(!q->size()) return;                    
//    q->first();
    
//    SINGLEVIDEO_UI = new SvSingleVideoWindow(q->value("video_path").toString(),
//                                             q->value("device_name").toString(),
//                                             "Видео: " + q->value("video_path").toString(),
//                                             800,
//                                             600);
//    q->finish();
//    free(q);
//  }

//  else if (playRLSOnMapAction == a)
//  {
//    parent->runRLS(this->position.x(), this->position.y());
    
//  }
}

QPainterPath SvBeackonSymbol::shape() const
{
    QPainterPath path;
    path.addRect(-10, -10, 20, 20); /**  !!! **/
    return path;
}

