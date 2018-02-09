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
  struct BOUNDS;
  struct COORD;
  
   qreal geo1_geo2_distance(qreal lon1, qreal lat1, qreal lon2, qreal lat2);
  
   quint32 get_rnd_course();
   geo::COORD get_rnd_position(geo::BOUNDS &bounds) const;
}

class geo::POSITION 
{
public:
  explicit POSITION(geo::COORD coord, qreal course, QDateTime utc)  {
    
    qRegisterMetaType<geo::POSITION>("geo::POSITION");
    
    _coord.latitude =   coord.latitude;
    _coord.longtitude = coord.longtitude;
    _course = course;
    _utc = utc;
  }
  
  explicit POSITION() {  }  
  
  void setCoordinates(geo::COORD coord) { _coord.latitude =   coord.latitude;
                                          _coord.longtitude = coord.longtitude; }
  void setCourse(qreal course) { _course = course; }
  void setUTC(QDateTime utc) { _utc = utc; }
  
  geo::COORD coordinates() { return _coord; }
  qreal course() { return _course; }
  qreal angular_speed() { return _angular_speed; } // Угловая скорость поворота
  QDateTime utc() { return _utc; }
  
  
  qreal distanceTo(geo::COORD& coord) { 
    return geo::geo1_geo2_distance(_coord.longtitude, coord.longtitude, _coord.latitude, coord.latitude);
  }
  
//  bool operator !=(const geo::POSITION &other) const { return (_latitude != other.latitude && _longtitude != other.longtitude); }
//  bool operator ==(const geo::POSITION &other) const { return (_latitude == other.latitude && _longtitude == other.longtitude); }
  bool operator > (const geo::POSITION &other) const { return (this->_latitude >  other._latitude && this->_longtitude > other._longtitude); }
  bool operator < (const geo::POSITION &other) const { return (this->_latitude <  other._latitude && this->_longtitude < other._longtitude); }
//  bool operator <=(const geo::POSITION &other) const { return (_latitude <= other.latitude && _longtitude <= other.longtitude); }
//  bool operator >=(const geo::POSITION &other) const { return (_latitude >= other.latitude && _longtitude >= other.longtitude); }

private:  
  geo::COORD _coord;
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

struct geo::COORD {
  qreal latitude;
  qreal longtitude;  
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
