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
  if(!_port.open(QSerialPort::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  connect(&_timer, &QTimer::timeout, this, &lag::SvLAG::prepare_message);
  connect(this, &lag::SvLAG::write_message, this, &lag::SvLAG::write);
  
  
  _isOpened = _port.isOpen();
  
  return _isOpened;
  
}

void lag::SvLAG::close()
{
  _port.close();
  
  disconnect(&_timer, &QTimer::timeout, this, &lag::SvLAG::prepare_message);
  disconnect(this, &lag::SvLAG::write_message, this, &lag::SvLAG::write);
  
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

void lag::SvLAG::prepare_message()
{
  QString msg = nmea::lag_VBW(_current_geoposition);
  emit write_message(msg);
}

void lag::SvLAG::write(const QString &message)
{
  if(_port.isOpen()) {
   
    _port.write(message.toStdString().c_str(), message.size());
    
    _log << svlog::Attention << svlog::Time << message << svlog::endl;
    
  }
  
}

void lag::SvLAG::setSerialPortParams(const SerialPortParams& params)
{ 
  _port.setPortName(params.name);
  _port.setBaudRate(params.baudrate);
  _port.setDataBits(params.databits);
  _port.setFlowControl(params.flowcontrol);
  _port.setParity(params.parity);
  _port.setStopBits(params.stopbits);
}

void lag::SvLAG::read()
{
  
}
