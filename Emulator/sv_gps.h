#ifndef SV_GPS_H
#define SV_GPS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <random>
#include <QTime>
#include <qmath.h>

#include "sv_idevice.h"

namespace gps {

  struct GPSParams {
  
    quint32 gps_timeout;
    
    quint32 course;
    quint32 course_change_segment;
    quint32 course_change_ratio;
    
    quint32 speed;
    quint32 speed_change_segment;
    quint32 speed_change_ratio;
  };

  class SvGPS;
  class SvGPSThread;
  
}

class gps::SvGPS: public idev::SvIDevice
{
  Q_OBJECT
  
public:
  
  SvGPS();
  ~SvGPS(); 
  
  void setGPSParams(const gps::GPSParams& params) { _gps_params = params; }
  
  void start();
  
  QString get();
  
  int id = -1;
  
private:
  gps::SvGPSThread* _gps_thr = nullptr;
  
  gps::GPSParams _gps_params;
  
public slots:
  void new_coordinates(geo::COORD coord);
  void new_course(qreal course);
  
  
};


class gps::SvGPSThread : public QThread
{
  Q_OBJECT
  
public:
  SvGPSThread(const gps::GPSParams& params, geo::BOUNDS* bounds = nullptr);
  ~SvGPSThread();
  
  void stop();
    
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started = false;
  bool _finished = false;
  
  gps::GPSParams _gps_params;
  geo::BOUNDS* _bounds = nullptr;
  
  geo::COORD _current_coordinates;
  qreal _current_course;
  qreal _current_speed;
  
//  geo::COORD _last_coordinates;
//  qreal _last_course;
  
  qreal _one_tick_length; /* длина пути в метрах, за один отсчет таймера */
  
  qreal inline lonOffset();
  qreal inline latOffset();
  
  int quarter();
  void normal_angle();
  
signals:
  void new_coordinates(geo::COORD coord);
  void new_course(qreal course);
  
};

#endif // SV_GPS_H
