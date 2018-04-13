#include "sv_echo.h"

/** --------- Self LAG --------- **/
ech::SvECHO::SvECHO(int vessel_id, const geo::GEOPOSITION &geopos, svlog::SvLog &log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _log = log;
  
}

ech::SvECHO::~SvECHO()
{
  deleteLater();
}
  
bool ech::SvECHO::open()
{
  
  connect(&_timer, &QTimer::timeout, this, &ech::SvECHO::prepare_message);
  connect(this, &ech::SvECHO::write_message, this, &ech::SvECHO::write);
  
  _isOpened = true;
  
  return _isOpened;
  
}

void ech::SvECHO::close()
{

  disconnect(&_timer, &QTimer::timeout, this, &ech::SvECHO::prepare_message);
  disconnect(this, &ech::SvECHO::write_message, this, &ech::SvECHO::write);
  
  _isOpened = false;
}

bool ech::SvECHO::start(quint32 msecs)
{
  if(!_isActive)
    return true;
  
  if(_udp) delete _udp;
  
  _udp = new QUdpSocket();
  
  _timer.start(msecs);
  
  return true;
}

void ech::SvECHO::stop()
{
  _timer.stop();
}

void ech::SvECHO::newGPSData(const geo::GEOPOSITION& geopos)
{
  _current_geoposition = geopos;
}

void ech::SvECHO::prepare_message()
{
  QString msg = "";
  
  emit write_message(msg);
  
}

void ech::SvECHO::write(const QString &message)
{
  if(!message.isEmpty()) {
    _udp->writeDatagram(message.toLatin1(), QHostAddress(_ip), _port);
   
    _log << svlog::Attention << svlog::Time << message << svlog::endl;
    
  }
  
}

void ech::SvECHO::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR("VD", id, state, text);
  emit write_message(msg);
}

