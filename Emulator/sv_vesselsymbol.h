#ifndef SVBEACKONSYMBOL_H
#define SVBEACKONSYMBOL_H

#include <QWidget>
#include <QGraphicsItem>
#include <QStyle>
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QMenu>
#include <QAction>

#include "sv_area.h"

class SvArea;

enum SvGraphicSymbolTypes
{
  gtNode = 65537,
  gtZone,
  gtBeackon,
};

class SvBeackonSymbol: public QGraphicsItem
{
public:
  explicit SvBeackonSymbol(SvArea* parent,
                           bool allowEdit,
                           int id,
                           QString name,
                           QPointF mapCoord,
                           QPointF viewPosition,
                           int itemType = gtBeackon);
  
  ~SvBeackonSymbol();
  
  int type() const { return item_type; }

  QRectF boundingRect() const Q_DECL_OVERRIDE;
  QPainterPath shape() const Q_DECL_OVERRIDE;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

  SvArea* parent;
  int id;
  QString name;
  int item_type;
  QPointF view_position;
  QPointF map_coord;
  
  QAction* removeAction;
  QAction* editAction;
  QAction* playDataAction;
  QAction* viewInfoAction;
  
private:
  QColor brushColor;
  QColor penColor;
  bool isEditable;
  
  void deleteItem();
  
//  private slots:
//    void on_removeAction_triggered();
//    void on_editDeviceAction_triggered();
//    void on_playVideoAction_triggered();
//    void on_playRLSAction_triggered();
  
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
};

#endif // SVBEACKONSYMBOL_H
