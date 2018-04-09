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

  struct navtexData {
    
    quint32 id;
    quint32 station_region_id;
    quint32 station_message_id;
    QString region_station_name;
    QString message_designation;
    QString last_message;
    bool is_active = true;
    QString message_letter_id;
    QString region_letter_id;
    QString region_country;
    qint32 message_last_number;
    
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
  
  nav::navtexData  *data() { return &_data; }
    
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
