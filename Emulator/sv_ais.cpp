#include "sv_ais.h"

//ais::SvAIS* SELF_AIS;
QMap<int, ais::aisNavStat> NAVSTATS;

/** --------- Self AIS --------- **/
ais::SvSelfAIS::SvSelfAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata, svlog::SvLog &log)
//  ais::SvAIS(vessel_id, sdata, vdata, ddata)

{
  setVesselId(vessel_id);
  
  setStaticData(sdata);
  setVoyageData(vdata);
  setDynamicData(ddata);
  
  _log = log;
  
//  _vessel_id = vessel_id;
//  _static_data = sdata;  
//  _voyage_data = vdata;  
//  _dynamic_data = ddata; 
  
}

ais::SvSelfAIS::~SvSelfAIS()
{
  deleteLater();
}
  
bool ais::SvSelfAIS::open()
{
  if(!_port.open(QSerialPort::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  _isOpened = _port.isOpen();
  
  connect(this, &ais::SvSelfAIS::write_message, this, &ais::SvSelfAIS::write);
  
  return _isOpened;
}

void ais::SvSelfAIS::close()
{
  _port.close();
  disconnect(this, &ais::SvSelfAIS::write_message, this, &ais::SvSelfAIS::write);
  qDebug() << "closed";
  _isOpened = false;
}

bool ais::SvSelfAIS::start(quint32 msecs)
{
  
  
}

void ais::SvSelfAIS::stop()
{
  
}

void ais::SvSelfAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
  _dynamic_data.geoposition = geopos;
  emit updateSelfVessel(); 
}

void ais::SvSelfAIS::on_receive_ais_data(ais::SvAIS* otherAIS, ais::AISDataTypes type)
{
  /** проверяем расстояние. если в радиусе действия антенны, то обрабатываем данные */
  if(distanceTo(otherAIS) < _receive_range * 1000) {
    
    /** обрабатываем данные **/
    switch (type) {
      
      case ais::adtStaticVoyage:
       
//        msg = nmea::message1(0, otherAIS->getStaticData()->mmsi, otherAIS->navStatus(), 10, otherAIS->getDynamicData()->geoposition);
//        write(msg);
        
        break;
      
//      case ais::aisVoyage:
//        _log << svlog::Time << svlog::Data << QString("voyage data from %1: %2")
//             .arg(otherAIS->getStaticData()->id) 
//             .arg(otherAIS->getVoyageData()->destination) << svlog::endl;
//        break;

      case ais::adtDynamic:
      {
        QString msg = nmea::ais_message_1(0, otherAIS->getStaticData()->mmsi, otherAIS->navStatus(), 10, otherAIS->getDynamicData()->geoposition);
        emit write_message(msg);
        
        break;
       } 
        
      default:
        break;
    }
    
  }
  
  emit updateVesselById(otherAIS->vesselId());
  
}

qreal ais::SvSelfAIS::distanceTo(ais::SvAIS* remoteAIS)
{ 
  if(!remoteAIS) return 0.0; 
  else {
    
    geo::GEOPOSITION g = remoteAIS->_dynamic_data.geoposition;
    return geo::geo2geo_distance(_dynamic_data.geoposition, g); 
  }
}


void ais::SvSelfAIS::write(const QString &message)
{
  _log << svlog::Time << svlog::Data << message << svlog::endl;
  _port.write(message.toStdString().c_str(), message.size());
  
  udp = new QUdpSocket();
  QByteArray b(message.toStdString().c_str(), message.size());
  udp->writeDatagram(b, QHostAddress("192.168.44.228"), 29421);
  udp->close();

  _log << svlog::Data << svlog::Time << message << svlog::endl;
  
}

void ais::SvSelfAIS::setSerialPortParams(const SerialPortParams& params)
{ 
  _port.setPortName(params.name);
  _port.setBaudRate(params.baudrate);
  _port.setDataBits(params.databits);
  _port.setFlowControl(params.flowcontrol);
  _port.setParity(params.parity);
  _port.setStopBits(params.stopbits);
}

void ais::SvSelfAIS::read()
{
  
}


/** ----- Vessel AIS ------- **/
ais::SvOtherAIS::SvOtherAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata)
{
  _vessel_id = vessel_id;
  
  _static_data = sdata;  
  _voyage_data = vdata;  
  _dynamic_data = ddata; 
  
}

ais::SvOtherAIS::~SvOtherAIS()
{
  deleteLater();
}
  
bool ais::SvOtherAIS::open()
{
  _isOpened = true; 
}
void ais::SvOtherAIS::close()
{
  _isOpened = false;
}

bool ais::SvOtherAIS::start(quint32 msecs)
{
  connect(&_timer_static_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static_voyage);
//  connect(&_timer_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_voyage);
  connect(&_timer_dynamic, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _static_voyage_interval = NAVSTATS.value(_nav_status).static_voyage_interval;
//  _voyage_interval = NAVSTATS.value(_nav_status).voyage_interval;
  
  switch (_nav_status) {
    case 2:
    case 6:
      _dynamic_interval = _dynamic_data.geoposition.speed > 3 ? 10 * 1000 : 3 * 60 * 1000;
      
      break;
      
    default:
      if(_dynamic_data.geoposition.speed < 14) _dynamic_interval = 5 * 1000;
      else if(_dynamic_data.geoposition.speed < 23) _dynamic_interval = 3 * 1000;
      else _dynamic_interval = 2 * 1000;
      break;
  }
  
  // при первом запуске ставим интервал случайный, чтобы все корабли не отбивались одновременно
  QTime t = QTime::currentTime();
  qsrand(t.msec() + _vessel_id);
  qreal r = qreal(qrand() % 100) / 100.0;
  quint32 first_interval = quint32(_static_voyage_interval * r);
  qDebug() << first_interval;
  _timer_static_voyage.start(first_interval);
//  _timer_voyage.start(_voyage_interval);
  _timer_dynamic.start(_dynamic_interval);
  
  return true;
}

void ais::SvOtherAIS::stop()
{
//  disconnect(&_timer_static, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static);
//  disconnect(&_timer_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_voyage);
//  disconnect(&_timer_dynamic, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _timer_static_voyage.stop();
//  _timer_voyage.stop();
  _timer_dynamic.stop();
}

void ais::SvOtherAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
//  _mutex.lock();
  _dynamic_data.geoposition = geopos;
//  _mutex.unlock();
}


//void ais::SvAIS::receivedGeoPosition(SvAIS* ais)
//{
//  if(distanceTo(ais) < _receive_range) {
    
//    _dynamic_data.geoposition = ais;
//    emit ais->
//    emit updateVessel(); 
//  }
    
//}



///** ******  EMITTER  ****** **/
//ais::SvAISEmitter::SvAISEmitter(aisStaticData *sdata, aisVoyageData *vdata, aisDynamicData *ddata, QMutex *mutex)
//{
//  _static_data = sdata;
//  _voyage_data = vdata;
//  _dynamic_data = ddata;
  
//  _mutex = mutex;
//}
  
//ais::SvAISEmitter::~SvAISEmitter()
//{ 
//  stop();
//  deleteLater();
//}

//void ais::SvAISEmitter::stop()
//{
  
//}

//void ais::SvAISEmitter::run()
//{
//  while(_started) {
    
//    QTime t = QTime::currentTime();
    
////    if()
    
//  }
//}
