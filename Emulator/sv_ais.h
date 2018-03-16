#ifndef SV_AIS_H
#define SV_AIS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"

namespace ais {

  enum AISDataTypes {
    adtStatic,
    adtVoyage,
    adtStaticVoyage,
    adtDynamic
  };
  
  struct aisStaticData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    quint32 mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  struct aisDynamicData {                    // Динамическая информация
      
    geo::GEOPOSITION geoposition;         // Местоположение (широта и долгота)
                                          // Время (UTC)
                                          // Возраст информации (как давно обновлялась)
                                          // Курс истинный (относительно грунта), курсовой угол
                                          // Скорость истинная
                                          // Угол крена, дифферента
                                          // Угол килевой качки
                                          // Угловая скорость поворота
//    quint32 navstat;                      // Навигационный статус (к примеру: Лишен возможности управляться или Ограничен в возможности маневрировать)
  
  };
  
  struct aisVoyageData {                     // Рейсовая информация
  
    QString destination;                  // Пункт назначения
                                          // Время прибытия (ЕТА)
    qreal draft;                          // Осадка судна
    QString cargo;                        // Информация о грузе (класс/категория груза)
    quint32 team;                         // Количество людей на борту
                                          // Сообщения для предупреждения и обеспечения безопасности грузоперевозки
  };
  
  struct aisNavStat {
    QString name;
    quint32 static_voyage_interval;
//    quint32 voyage_interval;
    quint32 dynamic_interval;
  };
  
  class SvAIS;
  class SvSelfAIS;
  class SvOtherAIS;
//  class SvAISEmitter;
  
}

class ais::SvAIS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
//  SvAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata);
//  ~SvAIS(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setStaticData(const ais::aisStaticData& sdata) { _static_data = sdata; }
  void setVoyageData(const ais::aisVoyageData& vdata) { _voyage_data = vdata; }
  void setDynamicData(const ais::aisDynamicData& ddata) { _dynamic_data = ddata; }
  
  void setGeoPosition(const geo::GEOPOSITION& geopos) { _dynamic_data.geoposition = geopos; }
  
  int navStatus() { return _nav_status; }
  void setNavStatus(const int status) { _nav_status = status; }
  
  ais::aisStaticData  *getStaticData() { return &_static_data; }
  ais::aisVoyageData  *getVoyageData() { return &_voyage_data; }
  ais::aisDynamicData *getDynamicData() { return &_dynamic_data; }
  
//  idev::SvSimulatedDeviceTypes type();
    
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual void stop() = 0;
  
  friend class ais::SvSelfAIS;
  friend class ais::SvOtherAIS;
  
private:
  ais::aisStaticData _static_data;
  ais::aisVoyageData _voyage_data;
  ais::aisDynamicData _dynamic_data;
  
  int _vessel_id = -1;
  
  quint32 _nav_status = 1;
  
//public slots:
//  void newGPSData(const geo::GEOPOSITION& geopos);
  
};


class ais::SvSelfAIS : public ais::SvAIS
{
  Q_OBJECT
  
public:
  SvSelfAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata, svlog::SvLog& log);
  ~SvSelfAIS(); 
  
//  int vesselId() { return _vessel_id; }
  
//  void setStaticData(const ais::aisStaticData& sdata) { _static_data = sdata; }
//  void setVoyageData(const ais::aisVoyageData& vdata) { _voyage_data = vdata; }
//  void setDynamicData(const ais::aisDynamicData& ddata) { _dynamic_data = ddata; }
  
//  void setGeoPosition(const geo::GEOPOSITION& geopos) { _dynamic_data.geoposition = geopos; }
//  void setNavStatus(const QString& status) { _dynamic_data.navstat = status; }

  qreal receiveRange() { return _receive_range; }
  void setReceiveRange(qreal range) { _receive_range = range; }
  
  qreal distanceTo(ais::SvAIS* remoteAIS); /*{ if(!remoteAIS) return 0.0; 
    else {
      geo::GEOPOSITION g;
      int i = remoteAIS->aisDynamicData()->geoposition.course;
      return geo::geo2geo_distance(_dynamic_data.geoposition, g); }*/
                                          
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtSelfAIS; }
  
//  ais::aisStaticData *aisStaticData() { return &_static_data; }
//  ais::aisVoyageData *aisVoyageData() { return &_voyage_data; }
//  ais::aisDynamicData *aisDynamicData() { return &_dynamic_data; }
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
private:
  qreal _receive_range;
  
  svlog::SvLog _log;
//  ais::aisStaticData _static_data;
//  ais::aisVoyageData _voyage_data;
//  ais::aisDynamicData _dynamic_data;
  
//  int _vessel_id = -1;
  
signals:
  void updateSelfVessel();
  void updateVesselById(int id);
//  void broadcast();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void on_receive_ais_data(ais::SvAIS* otherAIS, ais::AISDataTypes type);
//  void on_receive_voyage (const ais::aisVoyageData& vdata);
//  void on_receive_dynamic(const ais::aisDynamicData& ddata);
  

  
};


class ais::SvOtherAIS : public ais::SvAIS
{
  Q_OBJECT
  
public:
  SvOtherAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata);
  ~SvOtherAIS(); 
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtOtherAIS; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
private:  
  QTimer _timer_static_voyage;
//  QTimer _timer_voyage;
  QTimer _timer_dynamic;
  
  quint32 _static_voyage_interval;
//  quint32 _voyage_interval;
  quint32 _dynamic_interval;
  
signals:
  void broadcast_ais_data(ais::SvAIS* ais, ais::AISDataTypes type);
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  
private slots:
  void on_timer_static_voyage()  { _timer_static_voyage.setInterval(_static_voyage_interval);
                                   emit broadcast_ais_data(this, ais::adtStaticVoyage); }
//  void on_timer_voyage()  { emit broadcast_ais_data(this, ais::aisVoyage); }
  void on_timer_dynamic() { emit broadcast_ais_data(this, ais::adtDynamic); }

  
};


//class ais::SvAISEmitter : public QThread
//{
//  Q_OBJECT
  
//public:
//  SvAISEmitter(ais::aisStaticData *sdata, ais::aisVoyageData *vdata, ais::aisDynamicData *ddata, QMutex *mutex);
//  ~SvAISEmitter();
  
//  void stop();
  
//  int vessel_id = -1;
  
//private:
//  void run() Q_DECL_OVERRIDE;
  
//  bool _started = false;
//  bool _finished = false;
  
//  ais::aisStaticData *_static_data = nullptr;
//  ais::aisVoyageData *_voyage_data = nullptr;
//  ais::aisDynamicData *_dynamic_data = nullptr;
  
//  QMutex *_mutex = nullptr;
  
//signals:
//  void ais_static_data(ais::aisStaticData *data);
//  void ais_voyage_data(ais::aisVoyageData *data);
//  void ais_dynamic_data(ais::aisDynamicData *data);
  
//};

#endif // SV_AIS_H
