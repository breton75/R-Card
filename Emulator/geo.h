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

  struct BOUNDS {
    qreal min_lat;
    qreal min_lon;
    qreal max_lat; 
    qreal max_lon;
  };
  
  struct COORD {
    COORD() { }
    COORD(qreal latitude, qreal longtitude) { this->latitude = latitude; this->longtitude = longtitude; }
    qreal latitude;
    qreal longtitude; 
    geo::COORD& operator =(const geo::COORD& other) { latitude = other.latitude; longtitude = other.longtitude; }
  };

//  class POSITION;
  
  struct POSITION {
    geo::COORD coord;
    qreal course;
    qreal angular_speed;
    QDateTime utc;
    
    QString navstat;
    
  };
  
  // расстояние между двумя координатами 
  qreal geo1_geo2_distance(qreal lon1, qreal lat1, qreal lon2, qreal lat2);
   
  // расстояние в километрах между двумя широтами/долготами 
  qreal lon1_lon2_distance(qreal min_lon, qreal max_lon, qreal lat);
  qreal lat1_lat2_distance(qreal min_lat, qreal max_lat, qreal lon);
  
  quint32 get_rnd_course();
  geo::COORD get_rnd_position(BOUNDS *bounds);
}


//class geo::POSITION 
//{
//public:
//  POSITION(geo::COORD coord, qreal course, QDateTime utc)  {
    
//    qRegisterMetaType<geo::POSITION>("geo::POSITION");
    
//    _coord = coord;
//    _course = course;
//    _utc = utc;
//  }
  
//  explicit POSITION() {  }  
  
//  void setCoordinates(const geo::COORD& coord) { _coord = coord; }
//  void setCourse(const qreal course) { _course = course; }
//  void setUTC(const QDateTime& utc) { _utc = utc; }
//  void setNavStatus(const QString& navstat) { _navstat = navstat; }
  
//  geo::COORD coordinates() const { return _coord; }
//  qreal course() const { return _course; }
//  qreal angular_speed() const { return _angular_speed; } // Угловая скорость поворота
//  QDateTime utc() const { return _utc; }
  
//  QString navstat() const { return _navstat; }
  
//  qreal distanceTo(geo::COORD& coord) { 
//    return geo::geo1_geo2_distance(_coord.longtitude, coord.longtitude, _coord.latitude, coord.latitude);
//  }
  
////  bool operator !=(const geo::POSITION &other) const { return (_latitude != other.latitude && _longtitude != other.longtitude); }
////  bool operator ==(const geo::POSITION &other) const { return (_latitude == other.latitude && _longtitude == other.longtitude); }
//  bool operator > (const geo::POSITION &other) const { return (this->_latitude >  other._latitude && this->_longtitude > other._longtitude); }
//  bool operator < (const geo::POSITION &other) const { return (this->_latitude <  other._latitude && this->_longtitude < other._longtitude); }
////  bool operator <=(const geo::POSITION &other) const { return (_latitude <= other.latitude && _longtitude <= other.longtitude); }
////  bool operator >=(const geo::POSITION &other) const { return (_latitude >= other.latitude && _longtitude >= other.longtitude); }

//private:  
//  geo::COORD _coord;
//  qreal _course;
//  qreal _angular_speed;
//  QDateTime _utc;
  
//  QString _navstat;
  
//};


struct KOEFF {
  qreal lat;
  qreal lon;
};







//void getGridStep(AREA_DATA* area_data);

#endif // GEO_CALCULATIONS_H
