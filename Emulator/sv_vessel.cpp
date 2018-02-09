#include "sv_vessel.h"

vsl::SvVessel* SELF;
QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
//   _init_params = params;
//   _static_data = sdata;
//   _voyage_data = vdata;
   
//   _dynamic_data.position.setCoordinates(_init_params.);
   

   
   
}

vsl::SvVessel::~SvVessel()
{
  if(_gps_thr) 
    delete _gps_thr;
  
  deleteLater();
  
}

void vsl::SvVessel::new_position(qreal lon, qreal lat, qreal course)
{
//  data.
}

void vsl::SvVessel::start()
{
   _gps_thr = new vsl::SvGPSThread(_init_params);
   connect(_gps_thr, &QThread::finished, &QThread::deleteLater);
   connect(_gps_thr, SIGNAL(new_position(qreal, qreal, qreal)), this, SLOT(new_position(qreal, qreal, qreal)));
   _gps_thr->start();
}

/*************** SvGPSThread ***************/
vsl::SvGPSThread::SvGPSThread(const InitParams &initParams, geo::BOUNDS *bounds)
{
  _init_params = initParams;
  _bounds = bounds;
  
}

vsl::SvGPSThread::~SvGPSThread()
{
  stop();
  deleteLater();
}

void vsl::SvGPSThread::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void vsl::SvGPSThread::run()
{
  /* свои начальные координаты */
  _current_position.setLongtitude((_bounds->min_lon+ _bounds->max_lon) / 2);
  _current_position.setLatitude((_bounds->min_lat + _bounds->max_lat) / 2);
  
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  
  _started = true;
  _finished = false;
  
  while(_started) {
    
    if(_init_params.course_change_ratio && 
       _init_params.course_change_segment && 
       (course_segment_counter > _init_params.course_change_segment)) {
      
      /* вычисляем новый угол поворота */
//      int a = qrand() % _gps_params.course_change_ratio;
//      int b = -1 + (2 * (qrand() % 2));
//      _data.angle += a * b;
//      normal_angle();
//      counter = -_segment_length;
      
    }
    
    if(_init_params.speed_change_ratio && 
       _init_params.speed_change_segment && 
       (speed_segment_counter > _init_params.speed_change_segment)) {
      
      
      
    }
    
    emit new_position(_current_position.longtitude(), _current_position.latitude(), _current_position.course());
    
    msleep(_init_params.gps_timeout);
    
  }
  
  _finished = true;
  
}
