#include "sv_lag.h"

/** --------- Self LAG --------- **/
lag::SvLAG::SvLAG(int vessel_id, const geo::GEOPOSITION &geopos, svlog::SvLog &log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _log = log;
  
}

lag::SvLAG::~SvLAG()
{
  deleteLater();
}
  
bool lag::SvLAG::open()
{
  if(!_port.open(QSerialPort::WriteOnly)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  connect(&_timer, &QTimer::timeout, this, &lag::SvLAG::write_data);
//  connect(_port, &QSerialPort::readyRead, this, &lag::SvLAG::read_data);
  
  
  _isOpened = true;
  
  return true;
}

void lag::SvLAG::close()
{
  _port.close();
  
  disconnect(&_timer, &QTimer::timeout, this, &lag::SvLAG::write_data);
//  disconnect(_port, &QSerialPort::readyRead, this, &lag::SvLAG::read_data);
  
  _isOpened = false;
}

bool lag::SvLAG::start(quint32 msecs)
{
  _timer.start(msecs);
  
  return true;
}

void lag::SvLAG::stop()
{
  _timer.stop();
}

void lag::SvLAG::newGPSData(const geo::GEOPOSITION& geopos)
{
  _current_geoposition = geopos;
}

void lag::SvLAG::write_data()
{
  _log << svlog::Time << svlog::Data << QString("lag data: %1: spd:%2")
       .arg(_vessel_id) 
       .arg(_current_geoposition.speed, 0, 'g', 2)
       << svlog::endl;
  
}

void lag::SvLAG::setSerialPortInfo(const QSerialPortInfo& info)
{ 
  _port_info = QSerialPortInfo(info); 
  
  _port.setPort(info);
  
}

void lag::SvLAG::read_data()
{
  
}
