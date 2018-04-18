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
  
}

ech::SvECHO::~SvECHO()
{
  deleteLater();
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
  _udp->setMulticastInterface(QNetworkInterface::interfaceFromIndex(_params.ifc));
  
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
  
  int x = quint32(geo::lon2lon_distance(_bounds.min_lon, geopos.longtitude, geopos.latitude) * 1000) % _depth_map_image.width();
  int y = quint32(geo::lat2lat_distance(_bounds.min_lat, geopos.latitude, geopos.longtitude) * 1000) % _depth_map_image.height();
  
  qDebug() << _depth_map_image.pixelIndex(x, y);
}

void ech::SvECHO::prepare_message()
{
  QString msg = "";
  
  emit write_message(msg);
  
}

void ech::SvECHO::write(const QString &message)
{
  if(!message.isEmpty()) {
    _udp->writeDatagram(message.toLatin1(), QHostAddress(_params.ip), _params.port);
   
    _log << svlog::Attention << svlog::Time << message << svlog::endl;
    
  }
}

void ech::SvECHO::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR("VD", id, state, text);
  emit write_message(msg);
}

