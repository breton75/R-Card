#include "sv_vessel.h"

SvVessel::SvVessel(const vsl::GPSParams &params, QObject *parent) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
   _gps_params = params;
   
   _gps_thr = new SvGPSThread(_gps_params);
   connect(_gps_thr, &QThread::finished, &QThread::deleteLater);
   connect(_gps_thr, SIGNAL(new_position(qreal, qreal, qreal)), this, SLOT(new_position(qreal, qreal, qreal)));
   _gps_thr->start();
   
   
}

SvVessel::~SvVessel()
{
  if(_gps_thr) 
    delete _gps_thr;
  
  deleteLater();
  
}

void SvVessel::new_position(qreal lon, qreal lat, qreal course)
{
//  data.
}



/*************** SvGPSThread ***************/
SvGPSThread::SvGPSThread(const vsl::GPSParams &gpsParams, geo::BOUNDS *bounds)
{
  _gps_params = gpsParams;
  _bounds = bounds;
  
}

SvGPSThread::~SvGPSThread()
{
  stop();
  deleteLater();
}

void SvGPSThread::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void SvGPSThread::run()
{
  /* свои начальные координаты */
  _current_position.setLongtitude((_bounds->min_lon+ _bounds->max_lon) / 2);
  _current_position.setLatitude((_bounds->min_lat + _bounds->max_lat) / 2);
  
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  
  _started = true;
  _finished = false;
  
  while(_started) {
    
    if(_gps_params.course_change_ratio && 
       _gps_params.course_change_segment && 
       (course_segment_counter > _gps_params.course_change_segment)) {
      
      /* вычисляем новый угол поворота */
//      int a = qrand() % _gps_params.course_change_ratio;
//      int b = -1 + (2 * (qrand() % 2));
//      _data.angle += a * b;
//      normal_angle();
//      counter = -_segment_length;
      
    }
    
    if(_gps_params.speed_change_ratio && 
       _gps_params.speed_change_segment && 
       (speed_segment_counter > _gps_params.speed_change_segment)) {
      
      
      
    }
    
    emit new_position(_current_position.longtitude(), _current_position.latitude(), _current_position.course());
    
    msleep(_gps_params.gps_timeout);
    
  }
  
  _finished = true;
  
}
