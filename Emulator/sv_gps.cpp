#include "sv_gps.h"

gps::SvGPS::SvGPS()
{
  
}

gps::SvGPS::~SvGPS()
{
  if(_gps_thr) 
    delete _gps_thr;
  
  deleteLater();
  
}

void gps::SvGPS::new_coordinates(geo::COORD coord)
{
//  data.
}

void gps::SvGPS::new_course(qreal course)
{
//  data.
}

void gps::SvGPS::start()
{
   _gps_thr = new gps::SvGPSThread(_gps_params);
   connect(_gps_thr, &QThread::finished, &QThread::deleteLater);
   connect(_gps_thr, SIGNAL(new_coordinates(geo::COORD)), this, SLOT(new_coordinates(geo::COORD)));
   _gps_thr->start();
}




/** ************* SvGPSThread ************* **/
gps::SvGPSThread::SvGPSThread(const gps::GPSParams &params, geo::BOUNDS *bounds)
{
  _gps_params = params;
  _bounds = bounds;
  
}

gps::SvGPSThread::~SvGPSThread()
{
  stop();
  deleteLater();
}

void gps::SvGPSThread::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void gps::SvGPSThread::run()
{
  /* свои начальные координаты */
  _current_coordinates.longtitude = (_bounds->min_lon + _bounds->max_lon) / 2;
  _current_coordinates.latitude = (_bounds->min_lat + _bounds->max_lat) / 2;
  
  _current_course = _gps_params.course;
  _current_speed = _gps_params.speed;
  
  /* длина пути в метрах, за один отсчет таймера */
  _one_tick_length = qreal(_current_speed * 1000) / 3600 / (1000 / _gps_params.gps_timeout); 
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  
  _started = true;
  _finished = false;
  
  while(_started) {
    
    /** вычисляем новый курс **/
    if(_gps_params.course_change_ratio && 
       _gps_params.course_change_segment && 
       (course_segment_counter > _gps_params.course_change_segment)) {
      
//      int a = qrand() % _gps_params.course_change_ratio;
//      int b = -1 + (2 * (qrand() % 2));
//      _data.angle += a * b;
//      normal_angle();
//      counter = -_segment_length;
      
    }
    
    /** вычисляем новую скорость **/
    if(_gps_params.speed_change_ratio && 
       _gps_params.speed_change_segment && 
       (speed_segment_counter > _gps_params.speed_change_segment)) {
      
      
      
    }
    
    emit new_coordinates(_current_coordinates);
    emit new_course(_current_course);
    
    msleep(_gps_params.gps_timeout);
    
  }
  
  _finished = true;
  
}


qreal gps::SvGPSThread::lonOffset()
{
  qreal b;
  
  int q = quarter();
  switch (q) {
    case 1:
      b = sin(_current_course * M_PI / 180);
      break;
      
    case 2:
      b = cos((_current_course - 90) * M_PI / 180);
      break;
      
    case 3:
      b = -sin((_current_course - 180) * M_PI / 180);
      break;
      
    case 4:
      b = -cos((_current_course - 270) * M_PI / 180);
      break;
      
    case 0:
    case 180:  
      b = 0;
      break;
      
    case 90:
      b = 1;
      break;
      
    case 270:
      b = -1;
      break;
      
    default:
      qDebug() << "!!!! wrong quarter b. angle=" << _current_course;
      break;

  }
//  qDebug() << b * _segment_length * _lon1m;
  return b * _one_tick_length * _lon1m;
}

qreal gps::SvGPSThread::latOffset()
{
  qreal a;
  
  int q = quarter();
  switch (q) {
    case 1:
      a = cos(_data.angle * M_PI / 180);
      break;
      
    case 2:
      a = -sin((_data.angle - 90) * M_PI / 180);
      break;
      
    case 3:
      a = -cos((_data.angle - 180) * M_PI / 180);
      break;
      
    case 4:
      a = sin((_data.angle - 270) * M_PI / 180);
      break;
      
    case 0:
      a = 1;
      break;
      
    case 90:
    case 270:
      a = 0;
      break;
      
    case 180:
      a = -1;
      break;
      
      
    default:
      qDebug() << "!!!! wrong quarter a. angle=" << _data.angle;
      break;
  }
//  qDebug() << b * _segment_length * _lat1m;
  return a * _one_tick_length * _lat1m;
}

void gps::SvGPSThread::normal_angle()
{
  /**
  _data.angle = _data.angle > 0 ? _data.angle % 360 : 360 - qAbs(_data.angle % 360); //   * (qAbs(int(_data.angle / 360)) + 1) + _data.angle;
  **/
//  _data.angle = _data.angle % 360; // != 0 ? _data.angle % 360 : 0; //_data.angle;
}   

int gps::SvGPSThread::quarter()
{
  switch (_current_course % 90) {
    case 0:
      return int(_current_course);
      break;
      
    default:
      return int(_current_course / 90) + 1;
      break;
  }
}
