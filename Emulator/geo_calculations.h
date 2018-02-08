#ifndef GEO_CALCULATIONS_H
#define GEO_CALCULATIONS_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <math.h>
#include <QDateTime>
#include <qmath.h>
#include <QMetaType>

#define MINOR_LINE_INTERVAL 10
#define MAJOR_LINE_INTERVAL 50

namespace geo {

  class POSITION;
  class BOUNDS;
  
   qreal geo1_geo2_distance(qreal lon1, qreal lat1, qreal lon2, qreal lat2);
  
}

class geo::POSITION 
{
public:
  explicit POSITION(qreal longtitude, qreal latitude, qreal course, QDateTime utc)  {
    
    qRegisterMetaType<geo::POSITION>("geo::POSITION");
    
    _longtitude = longtitude;
    _latitude = latitude;
    _course = course;
    _utc = utc;
  }
  
  explicit POSITION() {  }  
  
  void setLatitude(qreal latitude) { _latitude = latitude; }
  void setLongtitude(qreal longtitude) { _longtitude = longtitude; }
  void setCourse(qreal course) { _course = course; }
  void setUTC(QDateTime utc) { _utc = utc; }
  
  qreal latitude() { return _latitude; }
  qreal longtitude() { return _longtitude; }
  qreal course() { return _course; }
  qreal angular_speed() { return _angular_speed; } // Угловая скорость поворота
  QDateTime utc() { return _utc; }
  
  
  qreal distanceTo(geo::POSITION& o) { 
    return geo::geo1_geo2_distance(_longtitude, o.longtitude(), _latitude, o.latitude());
  }
  
//  bool operator !=(const geo::POSITION &other) const { return (_latitude != other.latitude && _longtitude != other.longtitude); }
//  bool operator ==(const geo::POSITION &other) const { return (_latitude == other.latitude && _longtitude == other.longtitude); }
  bool operator > (const geo::POSITION &other) const { return (this->_latitude >  other._latitude && this->_longtitude > other._longtitude); }
  bool operator < (const geo::POSITION &other) const { return (this->_latitude <  other._latitude && this->_longtitude < other._longtitude); }
//  bool operator <=(const geo::POSITION &other) const { return (_latitude <= other.latitude && _longtitude <= other.longtitude); }
//  bool operator >=(const geo::POSITION &other) const { return (_latitude >= other.latitude && _longtitude >= other.longtitude); }

private:  
  qreal _latitude;
  qreal _longtitude;
  qreal _course;
  qreal _angular_speed;
  QDateTime _utc;
  
};

struct geo::BOUNDS {
  qreal min_lat;
  qreal min_lon;
  qreal max_lat; 
  qreal max_lon;
};

struct KOEFF {
  qreal lat;
  qreal lon;
};






/* расстояние в километрах между двумя широтами/долготами */
qreal lon1_lon2_distance(qreal min_lon, qreal max_lon, qreal lat);
qreal lat1_lat2_distance(qreal min_lat, qreal max_lat, qreal lon);

//void getGridStep(AREA_DATA* area_data);

#endif // GEO_CALCULATIONS_H
