#include "sv_gps.h"

gps::SvGPS* SELF_GPS;

gps::SvGPS::SvGPS(int vessel_id, gpsInitParams &params, geo::BOUNDS* bounds)
{
  _vessel_id = vessel_id;
  _gps_params = params;
  _bounds = bounds;
}

gps::SvGPS::~SvGPS()
{
  if(_gps_emitter)
    delete _gps_emitter;
  
  deleteLater();
}
  
bool gps::SvGPS::open()
{
  _isOpened = true;
}
void gps::SvGPS::close()
{
  _isOpened = false;
}

bool gps::SvGPS::start(quint32 msecs)
{
  if(_gps_emitter) {
    delete _gps_emitter;
    _gps_emitter = nullptr;
  }
  
  _gps_emitter = new gps::SvGPSEmitter(_vessel_id, _gps_params, _bounds);
  connect(_gps_emitter, &gps::SvGPSEmitter::finished, _gps_emitter, &gps::SvGPSEmitter::deleteLater);
  connect(_gps_emitter, SIGNAL(newGeoPosition(const geo::GEOPOSITION&)), this, SIGNAL(newGeoPosition(const geo::GEOPOSITION&)));
  _gps_emitter->start();
                 
  
}

void gps::SvGPS::stop()
{
  if(!_gps_emitter) return;
  
  delete _gps_emitter;
  _gps_emitter = nullptr;
  
}


/** ******  EMITTER  ****** **/
gps::SvGPSEmitter::SvGPSEmitter(int vessel_id, gps::gpsInitParams& params, geo::BOUNDS *bounds)
{
  _vessel_id = vessel_id;
  _gps_params = params;
  _bounds = bounds;
  
  _current_geo_position = _gps_params.geoposition;

  // длина пути в метрах, за один отсчет таймера // скорость в узлах. 1 узел = 1852 метра в час
  _one_tick_length = qreal(_current_geo_position.speed * 1000) / 3600.0 / (1000.0 / qreal(_gps_params.gps_timeout));
  
  // определяем, сколько градусов в 1ом метре вдоль долготы
  qDebug() << "ll1:" << _bounds->max_lat - _bounds->min_lat <<  geo::lat1_lat2_distance(_bounds->min_lat, _bounds->max_lat, _current_geo_position.longtitude);
  _lon_1m_angular_length = (_bounds->max_lat - _bounds->min_lat) / geo::lat1_lat2_distance(_bounds->min_lat, _bounds->max_lat, _current_geo_position.longtitude) / 1000.0;
  
  // определяем, сколько градусов в 1ом метре вдоль широты */
  qDebug() << "ll2:" << _bounds->max_lon - _bounds->min_lon << geo::lon1_lon2_distance(_bounds->min_lon, _bounds->max_lon, _current_geo_position.latitude) << _current_geo_position.latitude;
  _lat_1m_angular_length = (_bounds->max_lon - _bounds->min_lon) / geo::lon1_lon2_distance(_bounds->min_lon, _bounds->max_lon, _current_geo_position.latitude) / 1000.0;
  
   
}

gps::SvGPSEmitter::~SvGPSEmitter()
{
  stop();
  deleteLater();  
}

void gps::SvGPSEmitter::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void gps::SvGPSEmitter::run()
{
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  
  _started = true;
  _finished = false;
  
  while(_started) {
    
//    qDebug() << " emitter:" << _current_geo_position.latitude;
    emit newGeoPosition(_current_geo_position);
//    emit new_course(_current_course);
    
    msleep(_gps_params.gps_timeout);
    
    /** вычисляем новый курс **/
    if(_gps_params.course_change_ratio && 
       _gps_params.course_change_segment && 
       (course_segment_counter > _gps_params.course_change_segment)) {
      
      int a = qrand() % _gps_params.course_change_ratio;
      int b = -1 + (2 * (qrand() % 2));
      
      _current_geo_position.course += a * b;
      
      // нормируем курс, чтобы он вписывался в диапазон 0 - 360 градусов
      _current_geo_position.course = _current_geo_position.course > 0 ? _current_geo_position.course % 360 : 360 - qAbs(_current_geo_position.course % 360);

      course_segment_counter = -_one_tick_length;
      
    }
    
    course_segment_counter += _one_tick_length;
    
    /** вычисляем новую скорость **/
    if(_gps_params.speed_change_ratio && 
       _gps_params.speed_change_segment && 
       (speed_segment_counter > _gps_params.speed_change_segment)) {
      
      
      
    }
    
    geo::COORDINATES llo = lonlatOffset();
    _current_geo_position.latitude += llo.latitude;
    _current_geo_position.longtitude += llo.longtitude;
    
    
  }
  
  _finished = true;
  
}

geo::COORDINATES gps::SvGPSEmitter::lonlatOffset()
{
  qreal a, b;
  geo::COORDINATES result;
  
  // определяем четверть, в которой находится текущий курс
  int q = _current_geo_position.course % 90 == 0 ? int(_current_geo_position.course) : int(_current_geo_position.course / 90) + 1;
  
  switch (q) {
    
    case 1:
      a = cos(_current_geo_position.course * M_PI / 180.0);
      b = sin(_current_geo_position.course * M_PI / 180.0);
      break;
      
    case 2:
      a = -sin((_current_geo_position.course - 90) * M_PI / 180.0);
      b =  cos((_current_geo_position.course - 90) * M_PI / 180.0);
      break;
      
    case 3:
      a = -cos((_current_geo_position.course - 180) * M_PI / 180.0);
      b = -sin((_current_geo_position.course - 180) * M_PI / 180.0);
      break;
      
    case 4:
      a =  sin((_current_geo_position.course - 270) * M_PI / 180.0);
      b = -cos((_current_geo_position.course - 270) * M_PI / 180.0);
      break;
      
    case 0:
      a = 1.0;
      b = 0.0;
      break;
      
    case 90:
      a = 0.0;
      b = 1.0;
      break;
      
    case 270:
      a =  0.0;
      b = -1.0;
      break;
      
    case 180:
      a = -1.0;
      b =  0.0;
      break;
      
      
    default:
      qDebug() << "!!!! wrong quarter a. angle=" << _current_geo_position.course;
      break;
  }
//  qDebug() << a << b << _one_tick_length << _lat_1m_angular_length << _lon_1m_angular_length;
  result.latitude = a * _one_tick_length * _lat_1m_angular_length;
  result.longtitude = b * _one_tick_length * _lon_1m_angular_length;
  
  return result;
}


