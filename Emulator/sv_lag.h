#ifndef SV_LAG_H
#define SV_LAG_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"

namespace lag {

  struct lagData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  
  class SvLAG;
  
}

class lag::SvLAG : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvLAG(int vessel_id, const geo::GEOPOSITION& geopos, svlog::SvLog &log);
  ~SvLAG(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setData(const lag::lagData& ldata) { _data = ldata; }
  
  void setSerialPortInfo(const QSerialPortInfo& info);
  
  lag::lagData  *getData() { return &_data; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtLAG; }
  
private:
  lag::lagData _data;
  
  int _vessel_id = -1;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  
  QTimer _timer;
  
  QSerialPort _port;
  QSerialPortInfo _port_info;
  
private slots:
  void write_data();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  
};

#endif // SV_LAG_H
