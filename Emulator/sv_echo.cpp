#include "sv_echo.h"

/** --------- ECHO --------- **/
ech::SvECHO::SvECHO(int vessel_id, const geo::GEOPOSITION &geopos, geo::BOUNDS *bounds, QString &imgpath, svlog::SvLog &log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _bounds = *bounds;
  _log = log;
  
  /// вычисляем ширину карты и высоту в метрах
//  _map_width_meters =  1000 * geo::lon2lon_distance(_bounds.min_lon, _bounds.max_lon, (_bounds.max_lat + _bounds.min_lat) / 2.0);
//  _map_height_meters = 1000 * geo::lat2lat_distance(_bounds.min_lat, _bounds.max_lat, (_bounds.max_lon + _bounds.min_lon) / 2.0);
//  qDebug() << _map_width_meters << _map_height_meters;
  
  if(!_depth_map_image.load(imgpath)) {
    
    _log << svlog::Critical << svlog::Time << QString("Не удалось загрузить файл %1").arg(imgpath) << svlog::endl;
    
  }
  
  _koeff_lat = qreal(_depth_map_image.height()) / (_bounds.max_lat - _bounds.min_lat);
  _koeff_lon = qreal(_depth_map_image.width()) / (_bounds.max_lon - _bounds.min_lon);
  
}

ech::SvECHO::~SvECHO()
{
  while(_beams.count())
    delete _beams.takeFirst();
  
  deleteLater();
}

void ech::SvECHO::setBeamCount(int count)
{
  _beam_count = count;
  
  _beams.clear();
  
  qreal stepA = qreal(HEADRANGE) / qreal(_beam_count - 1);
  int A = HEADRANGE / 2;
  int I = (_beam_count % 2 == 0) ? _beam_count / 2 : (_beam_count - 1) / 2;
  for(int i = 0; i < _beam_count; i++) {
        
    _beams.append(new ech::Beam);
    _beams.last()->index = I - i;
    if(_beams.last()->index == 0 && _beam_count % 2 == 0)
      _beams.last()->index -= 1;
    
//    qDebug() << _beams.last()->index;
    _beams.last()->angle = A - stepA * i;
        
  }
  
}
  
bool ech::SvECHO::open()
{
  
//  connect(&_timer, &QTimer::timeout, this, &ech::SvECHO::prepare_message);
//  connect(this, &ech::SvECHO::write_message, this, &ech::SvECHO::write);
  
  _isOpened = true;
  
  return _isOpened;
  
}

void ech::SvECHO::close()
{

//  disconnect(&_timer, &QTimer::timeout, this, &ech::SvECHO::prepare_message);
//  disconnect(this, &ech::SvECHO::write_message, this, &ech::SvECHO::write);
  
  _isOpened = false;
}

bool ech::SvECHO::start(quint32 msecs)
{
  if(!_isOpened)
    return false;
  
  if(_udp) delete _udp;
  
  _udp = new QUdpSocket();
//  _udp->s MulticastInterface(QNetworkInterface::interfaceFromIndex(_params.ifc));
  
//  _timer.start(msecs);
  _clearance = msecs;
      
  return true;
}

void ech::SvECHO::stop()
{
  _timer.stop();
}

void ech::SvECHO::newGPSData(const geo::GEOPOSITION& geopos)
{
//  _current_geoposition = geopos;
}

void ech::SvECHO::passed1m(const geo::GEOPOSITION& geopos)
{
  if(_clearance_counter < _clearance) {
    _clearance_counter++; 
    return;
  }
  
  _clearance_counter = 0;
  
  _current_geoposition = geopos;
  
//  for(ech::Beam* beam: _beams) {
    qreal x0 = (_current_geoposition.longtitude - _bounds.max_lon) * _koeff_lon;
    qreal y0 = (_current_geoposition.latitude - _bounds.max_lat) * _koeff_lat;
               
//    int x0 = quint32(geo::lon2lon_distance(_bounds.min_lon, geopos.longtitude, geopos.latitude) * 1000) % 129; //_depth_map_image.width();
//    int y0 = quint32(geo::lat2lat_distance(_bounds.min_lat, geopos.latitude, geopos.longtitude) * 1000) % 129; //_depth_map_image.height();
    
    int x = x0; // * 2 - beam->index * cos(qDegreesToRadians(_current_geoposition.course));
    int y = y0; // * 2 + beam->index * sin(qDegreesToRadians(_current_geoposition.course));
    
    if(x < 0) x += _depth_map_image.width();
    if(x >= _depth_map_image.width()) x -= _depth_map_image.width();
    
    if(y < 0) y += _depth_map_image.height();
    if(y >= _depth_map_image.height()) y -= _depth_map_image.height();
    
    _beams[0]->Z = qGray(_depth_map_image.pixel(x, y));
    _beams[0]->X = cos(qDegreesToRadians(_beams[0]->angle));
    _beams[0]->Y = 0.0;
    
    _beams[0]->backscatter = qreal(qGray(_depth_map_image.pixel(y, x)) % 50);
    
    qsrand(QTime::currentTime().msecsSinceStartOfDay());
    if(qrand() % 10 == 0)
      _beams[0]->fish = 1 << (qrand() % 32);
    
//    qDebug() << "x" << x << "y" << y << _depth_map_image.pixelIndex(x, y);
//  }
  
  
  emit beamsUpdated(_beams.at(0));
  prepare_message();
  
  
}

void ech::SvECHO::prepare_message()
{
  QByteArray msg = QByteArray();
  
  _udp_header.size = sizeof(ech::Header) + sizeof(ech::Beam) * _beam_count + sizeof(quint32);
  _udp_header.num_points = _beam_count;
  _udp_header.ping_number += 1;
  _udp_header.latitude = _current_geoposition.latitude;
  _udp_header.longtitude = _current_geoposition.longtitude;
  _udp_header.bearing = _current_geoposition.course;
  _udp_header.roll = 0;
  _udp_header.pitch = 0;
  _udp_header.heave = 0;
  
  msg.append((const char*)(&_udp_header), sizeof(ech::Header));
  
  for(ech::Beam *beam: _beams) 
    msg.append((const char*)(beam), sizeof(ech::Beam));
  
  msg.append(4, 32);
      
  
  emit write_message(msg);
  
}

void ech::SvECHO::write(const QByteArray &message)
{
  if(!message.isEmpty()) {
    _udp->writeDatagram(message, QHostAddress(_params.ip), _params.port);
   
    _log << svlog::Attention << svlog::Time << QString("written: % bytes").arg(message.length()) << svlog::endl;
    
  }
}

void ech::SvECHO::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR("VD", id, state, text);
  emit write_message(msg.toLatin1());
}

