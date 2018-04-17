#ifndef SV_ECHO_H
#define SV_ECHO_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"
#include "nmea.h"

namespace ech {

  struct echoData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  
  class SvECHO;
  
}

class ech::SvECHO : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvECHO(int vessel_id, const geo::GEOPOSITION& geopos, svlog::SvLog &log);
  ~SvECHO(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setData(const ech::echoData& edata) { _data = edata; }
  
  ech::echoData  *getData() { return &_data; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtEcho; }
  
  void alarm(int id, QString state, QString text);

  
private:
  ech::echoData _data;
  
  QUdpSocket *_udp = nullptr;
  quint32 _ip;
  quint16 _port;
  
  int _vessel_id = -1;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  
  QTimer _timer;
  
signals:
  void write_message(const QString& message);
  
private slots:
  void write(const QString& message);
  void prepare_message();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  
};

#endif // SV_ECHO_H
