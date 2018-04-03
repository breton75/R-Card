#include "sv_ais.h"
#include "nmea.h"

//ais::SvAIS* SELF_AIS;
QMap<quint32, ais::aisNavStat> NAVSTATs;

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
  connect(&_port, &QSerialPort::readyRead, this, &SvSelfAIS::readPort);
  connect(this, &ais::SvSelfAIS::newIncomeMessage, this, &ais::SvSelfAIS::on_income_message);
  
//  qDebug() << _port.baudRate() << _port.
  
  return _isOpened;
}

void ais::SvSelfAIS::close()
{
  _port.close();
  disconnect(this, &ais::SvSelfAIS::write_message, this, &ais::SvSelfAIS::write);
  disconnect(&_port, &QSerialPort::readyRead, this, &SvSelfAIS::readPort);
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

void ais::SvSelfAIS::on_receive_message(ais::SvAIS* otherAIS, quint32 message_id)
{
  /** проверяем расстояние. если в радиусе действия антенны, то обрабатываем данные */
  if(distanceTo(otherAIS) < _receive_range * 1000) {
    
    /** обрабатываем данные **/
    switch (message_id) {
      
      case 5:
      {
       
//        QStringList l = nmea::ais_message_5(0, otherAIS->getStaticData(), otherAIS->getVoyageData(), otherAIS->navStatus());
//        emit write_message(l.first());
//        for(int i = 0; i < 100000; i++) ;
//        emit write_message(l.last());
        
        break;
      }
      
      case 73: {  // Binary acknowledgement of message 3
        
        QString abk = nmea::ais_sentence_ABK(otherAIS->getStaticData()->mmsi, 3);
        emit write_message(abk);
        
        break;
      }

      case 75: {  // Binary acknowledgement of message 5
        
        QString abk = nmea::ais_sentence_ABK(otherAIS->getStaticData()->mmsi, 5);
        emit write_message(abk);
        
        break;
      }
        
      case 1:
      {
        
        QString msg = nmea::ais_message_1_2_3(1, otherAIS->getStaticData()->mmsi, otherAIS->navStatus()->ITU_id, 10, otherAIS->getDynamicData()->geoposition);
        emit write_message(msg);
        
        break;
       } 
        
        
      case 3:
      {
        
        QString msg = nmea::ais_message_1_2_3(3, otherAIS->getStaticData()->mmsi, otherAIS->navStatus()->ITU_id, 10, otherAIS->getDynamicData()->geoposition);
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
//  return;
  _log << svlog::Time << svlog::Data << message << svlog::endl;
  _port.write(message.toStdString().c_str(), message.size());

//  udp = new QUdpSocket();
//  QByteArray b(message.toStdString().c_str(), message.size());
//  udp->writeDatagram(b, QHostAddress("192.168.44.228"), 29421);
//  udp->close();
//  delete udp;
  
//  _log << svlog::Data << svlog::Time << message << svlog::endl;
  
}

void ais::SvSelfAIS::readPort()
{
 QByteArray b = _port.readAll();

 _income_message.append(QString::fromLatin1(b));
 
 if(_income_message.contains("\r\n")) {
   
   QString msg = _income_message.split("\r\n").first();
   
   qDebug() << _income_message;
   _income_message.clear();
   
   emit newIncomeMessage(msg);
   
 }
 else if(_income_message.length() > 1024)
   _income_message.clear();
 
 
 
}

void ais::SvSelfAIS::on_income_message(QString &msg)
{
  /// парсим входящее сообщение
  QMap<int, QString> sentences = {{0, "AIR"}, {1, "Q"}, {2, "SSD"}, {3, "VSD"}};
  
  if(msg.length() < 11)
    return;
  
  if((msg.left(1) != "$") && (msg.left(1) != "!"))
    return;
  
  QString snt = msg.mid(5, 1) == "Q" ? "Q" : msg.mid(3, 3);
  if(!sentences.values().contains(snt))
    return;
  
  switch (sentences.key(snt)) {
    case 0: {
      QStringList l = msg.split(',');
      bool ok = true;
      QString s;
      quint32 mmsi1 = 0;
      quint32 msg1num = 0;
      qint32 msg2num = 0;
      quint32 mmsi2 = 0;
      qint32 msg3num = 0;
      
      // mmsi запрашиваемого судна. если пусто, то запрашиваем все суда
      s = QString(l.at(1));
      mmsi1 = s.toUInt(&ok);
      if(!ok && !s.isEmpty()) return;
      
      // номер 1го запрашиваемого сообщения
      s = QString(l.at(2));
      if(s.isEmpty()) return;
      
      msg1num = QString(s.split('.').first()).toUInt(&ok);
      if(!ok) return;
      
      // номер 2го запрашиваемого сообщения
      s = QString(l.at(4));
      if(!s.isEmpty()) {
        msg2num = QString(s.split('.').first()).toUInt(&ok);
        if(!ok) return;      
      }
      
      // mmsi 2го запрашиваемого судна
      s = QString(l.at(6));
      mmsi2 = s.toUInt(&ok);
      if(!ok && !s.isEmpty()) return;
      
      // номер 3го запрашиваемого сообщения
      s = QString(l.at(7));
      msg3num = QString(s.split('.').first()).toUInt(&ok);
      if(!ok && !s.isEmpty()) return;
      
      emit interrogateRequest(mmsi1, msg1num, msg2num, mmsi2, msg3num);
      
      break;
      
    }
      
    case 1:
      
      break;
      
    case 2:
      
      break;
      
    case 3:
      
      break;
      
  }
  
  
  
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
  if(!_isActive)
    return true;
  
  connect(&_timer_static_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static_voyage);
//  connect(&_timer_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_voyage);
  connect(&_timer_dynamic, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _static_voyage_interval = _nav_status.static_voyage_interval;
//  _voyage_interval = NAVSTATS.value(_nav_status).voyage_interval;
  
  switch (_nav_status.ITU_id) {
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
  
  _dynamic_data.geoposition = geopos;
//  _mutex.unlock();
}

void ais::SvOtherAIS::on_interrogate(quint32 mmsi1, quint32 msg1num, quint32 msg2num, quint32 mmsi2, quint32 msg3num)
{
  qDebug() << mmsi1 << msg1num;
  QList<int> messageIDs;
  messageIDs << 3 << 5;
  
  if((_static_data.mmsi == mmsi1) || (mmsi1 == 0)) {
    
    if(messageIDs.contains(msg1num)) {
      
      // для формирования подтверждения ABK
      emit broadcast_message(this, 70 + msg1num);
      
      emit broadcast_message(this, msg1num);
      
      if(messageIDs.contains(msg2num))
        emit broadcast_message(this, msg2num);
      
    }
    
    if((_static_data.mmsi == mmsi2) || (mmsi1 == 0)) {
      
      if(messageIDs.contains(msg3num))
        emit broadcast_message(this, msg1num);
      
    }
  }
  
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
