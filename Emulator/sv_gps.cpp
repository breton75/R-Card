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
  if(_gps_emitter) 
    delete _gps_emitter;
  
  _gps_emitter = new gps::SvGPSEmitter(_vessel_id, _gps_params, _bounds, _multiplier);
  connect(_gps_emitter, &gps::SvGPSEmitter::finished, _gps_emitter, &gps::SvGPSEmitter::deleteLater);
  connect(_gps_emitter, SIGNAL(newGPSData(const geo::GEOPOSITION&)), this, SIGNAL(newGPSData(const geo::GEOPOSITION&)));
  _gps_emitter->start();
                 
  
}

void gps::SvGPS::stop()
{
  if(!_gps_emitter) return;
  
  delete _gps_emitter;
  _gps_emitter = nullptr;
  
}


/** ******  EMITTER  ****** **/
gps::SvGPSEmitter::SvGPSEmitter(int vessel_id, gps::gpsInitParams& params, geo::BOUNDS *bounds, quint32 multiplier)
{
  _vessel_id = vessel_id;
  _gps_params = params;
  _bounds = bounds;
  _multiplier = multiplier;
  
  _current_geo_position = _gps_params.geoposition;

  // длина пути в метрах, за один отсчет таймера // скорость в узлах. 1 узел = 1852 метра в час
  _one_tick_length = _current_geo_position.speed * 1000.0 / 3600.0 / (1000.0 / qreal(_gps_params.gps_timeout)) * _multiplier;

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

  quint64 time_counter = QDateTime::currentDateTime().currentMSecsSinceEpoch();
  
  while(_started) {
    
    if(QDateTime::currentDateTime().currentMSecsSinceEpoch() - time_counter < _gps_params.gps_timeout) {
      
      continue;
    }
    
    time_counter = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    
    emit newGPSData(_current_geo_position);
    
    /** вычисляем новый курс **/
    if(_gps_params.course_change_ratio * 1000 && 
       _gps_params.course_change_segment && 
       (course_segment_counter > _gps_params.course_change_segment)) {
      
      int a = qrand() % _gps_params.course_change_ratio;
      int b = -1 + (2 * (qrand() % 2));
      
      // нормируем курс, чтобы он вписывался в диапазон 0 - 360 градусов
      _current_geo_position.course = normalize_course(_current_geo_position.course + a * b);

      course_segment_counter = -_one_tick_length;
      
    }
    
    course_segment_counter += _one_tick_length;
    
    /** вычисляем новую скорость **/
    if(_gps_params.speed_change_ratio && 
       _gps_params.speed_change_segment && 
       (speed_segment_counter > _gps_params.speed_change_segment)) {
      
      
      
    }
    
    geo::GEOPOSITION new_geopos = geo::get_next_geoposition(_current_geo_position, _one_tick_length);

    // если новая координата выходит за границу карты, то меняем курс и вычисляем новые координаты
    if(!geo::geoposition_within_bounds(new_geopos, _bounds)) {
      _current_geo_position.course = normalize_course(_current_geo_position.course + geo::get_rnd_course() % 45);
      continue;
    }
    
    _current_geo_position = new_geopos;
    
//    qDebug() << "llat llon:" << _current_geo_position.latitude << _current_geo_position.longtitude;
    
  }
  
  _finished = true;
  
}

qreal gps::SvGPSEmitter::normalize_course(quint32 course)
{
  qreal norm_course = course > 0 ? course % 360 : 360 - qAbs(course % 360);
  return norm_course;
}


