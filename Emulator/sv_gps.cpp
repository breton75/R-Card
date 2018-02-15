#include "sv_gps.h"

gps::SvGPS* SELF_GPS;

gps::SvGPS::SvGPS(int vessel_id, gps::GPSParams& params, geo::BOUNDS *bounds)
{
  _vessel_id = vessel_id;
  _gps_params = params;
  
  _bounds = bounds;
  
  // начальные координаты. если переданные координаты равны 0, то получаем случайные координаты
  _current_geo_position.longtitude = (_gps_params.init_random_coordinates || _gps_params.coordinates.longtitude == 0) ? 
                                      geo::get_rnd_position(_bounds).longtitude : 
                                      _gps_params.coordinates.longtitude; 
  
  _current_geo_position.latitude = (_gps_params.init_random_coordinates || _gps_params.coordinates.latitude == 0) ? 
                                    geo::get_rnd_position(_bounds).latitude : 
                                    _gps_params.coordinates.latitude;
  // начальные курс и скорость
  _current_geo_position.course = _gps_params.init_random_course ? geo::get_rnd_course() : _gps_params.course;
  _current_speed = _gps_params.speed;
  
  // длина пути в метрах, за один отсчет таймера 
  _one_tick_length = qreal(_current_speed * 1000) / 3600 / (1000 / _gps_params.gps_timeout);
  
  // определяем, сколько градусов в 1ом метре вдоль долготы
  _lon_1m_angular_length = (_bounds->max_lat - _bounds->min_lat) / geo::lat1_lat2_distance(_bounds->min_lat, _bounds->max_lat, _current_geo_position.longtitude) / 1000;
  
  // определяем, сколько градусов в 1ом метре вдоль широты */
  _lat_1m_angular_length = (_bounds->max_lon - _bounds->min_lon) / geo::lon1_lon2_distance(_bounds->min_lon, _bounds->max_lon, _current_geo_position.latitude) / 1000;
  
   
}

gps::SvGPS::~SvGPS()
{
  stop();
  deleteLater();  
}

void gps::SvGPS::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void gps::SvGPS::run()
{
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  
  _started = true;
  _finished = false;
  
  while(_started) {
    
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
      
      _current_geo_position.course = _current_geo_position.course > 0 ? _current_geo_position.course % 360 : 360 - qAbs(_current_geo_position.course % 360);

      course_segment_counter = -_gps_params.course_change_segment;
      
    }
    
    course_segment_counter = +_gps_params.course_change_segment;
    
    /** вычисляем новую скорость **/
    if(_gps_params.speed_change_ratio && 
       _gps_params.speed_change_segment && 
       (speed_segment_counter > _gps_params.speed_change_segment)) {
      
      
      
    }
    
    gps::LonLatOffset llo = lonlatOffset();
    _current_geo_position.latitude += llo.dlat;
    _current_geo_position.longtitude += llo.dlon;
    
    
  }
  
  _finished = true;
  
}

gps::LonLatOffset gps::SvGPS::lonlatOffset()
{
  qreal a, b;
  gps::LonLatOffset result;
  
  // определяем четверть, в которой находится текущий курс
  int q = _current_geo_position.course % 90 == 0 ? int(_current_geo_position.course) : int(_current_geo_position.course / 90) + 1;
  
  switch (q) {
    
    case 1:
      a = cos(_current_geo_position.course * M_PI / 180);
      b = sin(_current_geo_position.course * M_PI / 180);
      break;
      
    case 2:
      a = -sin((_current_geo_position.course - 90) * M_PI / 180);
      b = cos((_current_geo_position.course - 90) * M_PI / 180);
      break;
      
    case 3:
      a = -cos((_current_geo_position.course - 180) * M_PI / 180);
      b = -sin((_current_geo_position.course - 180) * M_PI / 180);
      break;
      
    case 4:
      a = sin((_current_geo_position.course - 270) * M_PI / 180);
      b = -cos((_current_geo_position.course - 270) * M_PI / 180);
      break;
      
    case 0:
      a = 1;
      b = 0;
      break;
      
    case 90:
      a = 0;
      b = 1;
      break;
      
    case 270:
      a = 0;
      b = -1;
      break;
      
    case 180:
      a = -1;
      b = 0;
      break;
      
      
    default:
      qDebug() << "!!!! wrong quarter a. angle=" << _current_geo_position.course;
      break;
  }

  result.dlat = a * _one_tick_length * _lat_1m_angular_length;
  result.dlon = b * _one_tick_length * _lon_1m_angular_length;
  
  return result;
}

void gps::SvGPS::norm_course()
{
  /**
  _data.angle = _data.angle > 0 ? _data.angle % 360 : 360 - qAbs(_data.angle % 360); //   * (qAbs(int(_data.angle / 360)) + 1) + _data.angle;
  **/
//  _data.angle = _data.angle % 360; // != 0 ? _data.angle % 360 : 0; //_data.angle;
}   

//int gps::SvGPSThread::quarter()
//{
//  switch (_current_course % 90) {
//    case 0:
//      return int(_current_course);
//      break;
      
//    default:
//      return int(_current_course / 90) + 1;
//      break;
//  }
//}
