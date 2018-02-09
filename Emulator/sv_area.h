#ifndef SVAREA_H
#define SVAREA_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QPainter>
#include <QScreen>
#include <QtXml>
#include <QMessageBox>
#include <QPair>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSplitter>

#include <QGraphicsEllipseItem>
#include <QTextItem>
#include <QGraphicsTextItem>


#include <QSqlQuery>


#include "../../svlib/sv_settings.h"
//#include "sv_vesselsymbol.h"
#include "geo_calculations.h"
#include "sv_mapobjects.h"
//#include "sv_udpstream.h"
#include "../../svlib/sv_sqlite.h"

#include "sv_vesseleditor.h"

#define VALUES_INTERVAL MAJOR_LINE_INTERVAL/2
#define RULER_SIZE 40
#define BORDER_WIDTH 10
#define SCALE_FACTOR 1.2

namespace area {

  class SvArea;
  class SvAreaScene;
  class SvAreaView;
  class SvHRuler;
  class SvVRuler;
  
  enum ButtonTypes {
    bntNone = -1,
    bntZoomIn = 0,
    bntZoomOut,
    bntZoomOriginal,
    bntMoveLeft,
    bntMoveRight,
    bntMoveUp,
    bntMoveDown,
    bntAlignToLeftTop,
    bntReadSocket,
    bntAddBeacon,
    bntCenterOnAirplane,
    bntTrackAirplane,
    bntDropBeacon
  };
  
  class AreaButton;
  
  struct AREA_DATA
  {
    geo::BOUNDS geo_bounds;
    qreal scale;
    KOEFF koeff;
    QSize area_base_size;
    QSize area_curr_size;
    QString area_name;
    
    QMap<quint64, QPair<qreal, qreal>> NODES;
    QMap<quint64, QList<QPair<qreal, qreal>>> WAYS;
  //  QMap<quint64, COORD> NODES;
  //  QMap<quint64, QList<COORD> WAYS;
    
    qreal gridCellStep = MINOR_LINE_INTERVAL;
    quint64 gridCellDistance;
    
  };
  
  
}

class area::AreaButton: public QPushButton
{
  Q_OBJECT
  
public:
  explicit AreaButton(QWidget *parent, int type, QString caption, QString objName, QSize size = QSize(40, 40), QIcon icon = QIcon())
  {
    setParent(parent);
    setType(type);
    setObjectName(objName);
    setFixedSize(size);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setText(caption);
    setIcon(icon);
  }

  int type() { return _type; }
  void setType(int type) { _type = type; }
  
private:
  int _type;
};


class area::SvArea : public QWidget
{
    Q_OBJECT

public:
    explicit SvArea(QWidget *parent);
    ~SvArea();

    void setUp(QString areaName);
    
    bool readBounds(QString &fileName);
    bool readMap(QString &fileName);
    
    qreal scale();
    void setScale(qreal scale);
    
    area::AREA_DATA* areaData() const { return &_area_data; }
    geo::BOUNDS bounds() const { return _area_data.geo_bounds; }
    
    area::SvAreaScene* scene;
    area::SvAreaView* view;
    
//    SvUdpReader* udp;
    
    
    
    area::SvHRuler* hruler;
    area::SvVRuler* vruler;

    
    /* виджеты */
    QHBoxLayout* hlayMain;
    QFrame* frameLeft;
    QVBoxLayout* vlayFrameLeft;
    QFrame* frameRight;
    QVBoxLayout* vlayFrameRight;
    QWidget* widgetMap;
    QList<area::AreaButton*> buttonsLeft;
    QList<area::AreaButton*> buttonsRight;
    
    QLabel* hint = nullptr;

    void updateGridStep();
    
    
private:
  area::AREA_DATA _area_data;  
  
  int _mouseButton = false;
  QPoint _mousePos;
  
  int _last_airplane_angle = -1;

//  QMap<GEO, QGraphicsEllipseItem*> _trackPoints;
  QMap<geo::POSITION, SvMapObjectDirection*> _trackPoints;
  
  QTime _track_time = QTime(0, 0, 30, 0);
  quint64 _track_secs;
  QPen _trackPen;
  QBrush _trackBrush;
  
  bool _draw_map;
  
  bool _editMode = false;
  
  bool _trackAirplane = false;
  
  bool _socketIsActive = false;
  
  quint32 _beaconToDropCount = 10;
  
  
signals:
  void scaleChanged();
//  void editing(bool editMode);
  
public slots:
  void scaleInc();
  void scaleDec();
  void centerAirplane();
//  void dropBeacon();
  
  void moveHorizontal(int val);
  void moveVertical(int val);
  void mousePressed(QMouseEvent * event);
  void mouseReleased(QMouseEvent * event);
  void mouseMoved(QMouseEvent * event);
  
private slots:
  void buttonPressed();
  void trackAirplane(qreal lon, qreal lat, int angle);
  void editing(bool editMode);
  SvMapObjectBeaconPlanned* findNearestPlanned(QPointF fromPoint);

};

class area::SvAreaScene: public QGraphicsScene
{
  Q_OBJECT

  public:
    explicit SvAreaScene(area::AREA_DATA *area_data);
    ~SvAreaScene() { deleteLater(); }
    
    QGraphicsRectItem* ltsq;
    QGraphicsRectItem* rtsq;
    QGraphicsRectItem* lbsq;
    QGraphicsRectItem* rbsq;
    
    QList<SvMapObject*> mapObjects() { return _mapObjects; }  

    void addMapObject(SvMapObject* object){ addItem(object); _mapObjects.append(object); }
    void removeMapObject(SvMapObject* object) { removeItem(object); _mapObjects.removeOne(object); }
  
  private:
    area::AREA_DATA *_area_data;
    QList<SvMapObject*> _mapObjects;
//    qreal   *_scale;
//    QPointF *_koeff;
//    QSize   *_area_size;
//    QSizeF  *_geo_size;
    
//    void drawBackground(QPainter *painter, const QRectF &r);

  public slots:
//    void resizeScene();
    
    
};

class area::SvAreaView : public QGraphicsView
{
    Q_OBJECT

  public:
    explicit SvAreaView(QWidget *parent, area::AREA_DATA *area_data);
    ~SvAreaView() { deleteLater(); }

    void setAreaScene(area::SvAreaScene* areaScene) { setScene(areaScene); _areaScene = areaScene; } 
    area::SvAreaScene* areaScene() { return _areaScene; }
    
//    QGraphicsScene* scene;
//    QGraphicsEllipseItem* ell;
//    GItem* mesh;

protected:
  #ifndef QT_NO_WHEELEVENT
//    void wheelEvent(QWheelEvent * e) Q_DECL_OVERRIDE;
  #endif

private:
  void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent * event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent * event) Q_DECL_OVERRIDE;

  void drawBackground(QPainter *painter, const QRectF &r);


  bool m_down = false;
  QPoint mousePos;
  int step = 10;
  
  QColor _gridBorderColor;
  QColor _gridMinorColor;
  QColor _gridMajorColor;
  QColor _mapCoastColor;
  QColor _fontColor;
  
  QPen _penBorder;
  QPen _penMajorLine;
  QPen _penMinorLine;
  QPen _pen_coastLine;

//  QWidget* _parentWidget;

  private:
    area::AREA_DATA *_area_data;
    
    area::SvAreaScene* _areaScene;
    
    
    
//    qreal   *_scale;
//    QPointF *_koeff;
//    QSize   *_area_size;
//    QSizeF  *_geo_size;

  signals:
    void mousePressed(QMouseEvent * event);
    void mouseReleased(QMouseEvent * event);
    void mouseMoved(QMouseEvent * event);

  public slots:
//    void resizeView();
    
//    void scaleInc();
//    void scaleDec();
//    void moveHorizontal(int val);
//    void moveVertical(int val);

};

class area::SvHRuler: public QWidget
{
  Q_OBJECT

  public:
    explicit SvHRuler(QWidget* parent, float* scale, QSize* areaSize);

    ~SvHRuler() { deleteLater(); }
  
  int begin_offset = 0;

  private:
//    void drawForeground(QPainter *painter, const QRectF &r);  
//    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
  
  float* _scale;
  QSize* _areaCurrSize;
  
  protected:
    void paintEvent(QPaintEvent * event);

};

class area::SvVRuler: public QWidget
{
  Q_OBJECT

  public:
    explicit SvVRuler(QWidget* parent, float* scale, QSize* areaSize);

    ~SvVRuler() { deleteLater(); }
  
  int begin_offset = 0;

  private:
//    void drawForeground(QPainter *painter, const QRectF &r);
//    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

  float* _scale;
  QSize* _areaCurrSize;
  
  protected:
    void paintEvent(QPaintEvent * event);

};


#endif //SVAREA_H
