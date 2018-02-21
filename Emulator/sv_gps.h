#ifndef SV_GPS_H
#define SV_GPS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <random>
#include <qmath.h>

#include "sv_idevice.h"
#include "geo.h"

namespace gps {

  struct gpsInitParams {
  
    quint32 gps_timeout;
    
    geo::GEOPOSITION geoposition;
    
//    quint32 course;
    quint32 course_change_segment;
    quint32 course_change_ratio;
    
//    quint32 speed;
    quint32 speed_change_segment;
    quint32 speed_change_ratio;
    
    bool init_random_coordinates;
    bool init_random_course;
    bool init_random_speed;
    
  };
  
//  struct LonLatOffset {
//    qreal dlon = 0.0;
//    qreal dlat = 0.0;
//  };

  class SvGPS;
  class SvGPSEmitter;
  
}

class gps::SvGPS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvGPS(int vessel_id, gps::gpsInitParams &params, geo::BOUNDS* bounds);
  ~SvGPS(); 
  
  int vesselId() { return _vessel_id; }
  
//  gps::SvGPSEmitter* emitter() { return _gps_emitter; }
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtGPS; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs = 0);
  void stop();
  
private:
//  geo::GEOPOSITION _current_geo_position;
  
  gps::SvGPSEmitter* _gps_emitter = nullptr;
  
  int _vessel_id = -1;
  gps::gpsInitParams _gps_params;
  geo::BOUNDS* _bounds = nullptr;
  
  QMutex _mutex;
  
  
signals:
  void newGeoPosition(const geo::GEOPOSITION& geopos);
  
};

class gps::SvGPSEmitter: public QThread
{
  Q_OBJECT
  
public:
  
  SvGPSEmitter(int vessel_id, gps::gpsInitParams &params, geo::BOUNDS* bounds);
  ~SvGPSEmitter(); 
  
  int vesselId() { return _vessel_id; }
  
  geo::GEOPOSITION currentGeoPosition() const { return _current_geo_position; }
  
  void stop();

  
private:
  void run() Q_DECL_OVERRIDE;
  
  int _vessel_id = -1;
  
  bool _started = false;
  bool _finished = false;
  
  gps::gpsInitParams _gps_params;
  geo::BOUNDS* _bounds = nullptr;
  
  geo::GEOPOSITION _current_geo_position;
//  quint32 _current_course;
//  quint32 _current_speed;
  
  // параметры, необходимые для расчетов
  qreal _one_tick_length;         // длина пути в метрах, за один отсчет
  qreal _lon_1m_angular_length;   // градусов в 1ом метре вдоль широты
  qreal _lat_1m_angular_length;   // градусов в 1ом метре вдоль долготы
  
  geo::COORDINATES lonlatOffset();

//public slots:
//  void new_coordinates(geo::COORD coord);
//  void new_course(qreal course);
  
signals:
  void newGeoPosition(const geo::GEOPOSITION& geopos);
  
};


//class gps::SvGPSThread : public QThread
//{
//  Q_OBJECT
  
//public:
//  SvGPSThread(const gps::GPSParams& params, geo::BOUNDS* bounds = nullptr);
//  ~SvGPSThread();
  
//  void stop();
    
//private:
//  void run() Q_DECL_OVERRIDE;
  
//  bool _started = false;
//  bool _finished = false;
  
//  gps::GPSParams _gps_params;
//  geo::BOUNDS* _bounds = nullptr;
  
//  geo::COORD _current_coordinates;
//  qreal _current_course;
//  qreal _current_speed;
  
////  geo::COORD _last_coordinates;
////  qreal _last_course;
  
//  qreal _one_tick_length; /* длина пути в метрах, за один отсчет таймера */
  
////  qreal inline lonOffset();
//  gps::LonLatOffset lonlatOffset();
  
////  int quarter();
//  void normal_angle();
  
//signals:
//  void new_coordinates(geo::COORD coord);
//  void new_course(qreal course);
  
//};

#endif // SV_GPS_H
