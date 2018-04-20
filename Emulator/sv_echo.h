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
#include <QtGui/QImage>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"
#include "nmea.h"
#include "sv_networkeditor.h"

#define HEADRANGE 40

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
  
  #pragma pack(1)
  struct Beam {
    int index;
    qreal X;
    qreal Y;
    qreal Z;
    qreal angle;
    qreal backscatter;
    quint8 quality;
    quint32 fish;
  };
  
  struct Header {
    quint32 sync_pattern = 0x77F9345A;
    quint32 size;
    char header[8] = {'C','O','R','B','A','T','H','Y'};
    quint32 version = 3;
    double time = 100;
    quint32 num_points;
    qint32 ping_number = 0;
    double latitude;
    double longtitude;
    float bearing;
    float roll;
    float pitch;
    float heave;
    quint32 sample_type = 1;
    quint32 spare = 0;
  };
  #pragma pack(pop)
  
  class SvECHO;
  
}

class ech::SvECHO : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvECHO(int vessel_id, const geo::GEOPOSITION& geopos, geo::BOUNDS* bounds, QString& imgpath, svlog::SvLog &log);
  ~SvECHO(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setData(const ech::echoData& edata) { _data = edata; }
  
  void setNetworParams(NetworkParams params) { _params = params; }
  void setBeamCount(int count);
  
  ech::echoData  *getData() { return &_data; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtEcho; }
  
  void alarm(int id, QString state, QString text);

  
private:
  ech::echoData _data;
  
  QList<ech::Beam*> _beams;
  
  QUdpSocket *_udp = nullptr;
  ech::Header _udp_header;
  
  NetworkParams _params;
  
  int _vessel_id = -1;
  
  int _beam_count = 1;
  
  qreal _koeff_lat = 1.0;
  qreal _koeff_lon = 1.0;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  geo::BOUNDS _bounds;
  
  qreal _map_width_meters;
  qreal _map_height_meters;
  
  QTimer _timer;
  
  QImage _depth_map_image;
  
  quint32 _clearance = 1;
  quint32 _clearance_counter = 0;
  
signals:
  void write_message(const QByteArray& message);
  void beamsUpdated(ech::Beam* bl);
  
private slots:
  void write(const QByteArray& message);
  void prepare_message();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void passed1m(const geo::GEOPOSITION& geopos);
  
};

#endif // SV_ECHO_H
