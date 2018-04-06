#ifndef SV_NAVTEX_H
#define SV_NAVTEX_H

#include <QObject>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

#include "geo.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"
#include "nmea.h"
#include "sv_serialeditor.h"
#include "sv_mapobjects.h"

namespace nav {

  struct navtexData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  
  class SvNAVTEX;
  
}

class nav::SvNAVTEX : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvNAVTEX(svlog::SvLog &log);
  ~SvNAVTEX(); 
  
  void setData(const nav::navtexData& ndata) { _data = ndata; }
  
  void setSerialPortParams(const SerialPortParams& params);
  
  nav::navtexData  *getData() { return &_data; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtNavtex; }
  
private:
  nav::navtexData _data;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  
  QTimer _timer;
  
  QSerialPort _port;
  
  SvMapObjectVesselAbstract* _map_object = nullptr;
  
signals:
  void write_message(const QString& message);
  
private slots:
  void write(const QString& message);
  void prepare_message();
  
  
};

#endif // SV_NAVTEX_H
