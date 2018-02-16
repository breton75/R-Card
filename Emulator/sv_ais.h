#ifndef SV_AIS_H
#define SV_AIS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"

namespace ais {

  struct aisStaticData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
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
    QString navstat;                      // Навигационный статус (к примеру: Лишен возможности управляться или Ограничен в возможности маневрировать)
  
  };
  
  struct aisVoyageData {                     // Рейсовая информация
  
    QString destination;                  // Пункт назначения
                                          // Время прибытия (ЕТА)
    qreal draft;                          // Осадка судна
    QString cargo;                        // Информация о грузе (класс/категория груза)
    quint32 team;                         // Количество людей на борту
                                          // Сообщения для предупреждения и обеспечения безопасности грузоперевозки
  };
  
  class SvAIS;
  class SvAISEmitter;
  
}

class ais::SvAIS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvAIS(int vessel_id);
  ~SvAIS(); 
  
  int vesselId() { return _vessel_id; }
  
  void setStaticData(const ais::aisStaticData& sdata) { _static_data = sdata; }
  void setVoyageData(const ais::aisVoyageData& vdata) { _voyage_data = vdata; }
  void setDynamicData(const ais::aisDynamicData& ddata) { _dynamic_data = ddata; }
  void setGeoPosition(const geo::GEOPOSITION& geopos) { _dynamic_data.geoposition = geopos; }
  void setNavStatus(const QString& status) { _dynamic_data.navstat = status; }

  ais::aisStaticData *aisStaticData() { return &_static_data; }
  ais::aisVoyageData *aisVoyageData() { return &_voyage_data; }
  ais::aisDynamicData *aisDynamicData() { return &_dynamic_data; }
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtAIS; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
private:
//  geo::GEOPOSITION _current_geo_position;
  
  ais::SvAISEmitter* _ais_emitter = nullptr;
  
  ais::aisStaticData _static_data;
  ais::aisVoyageData _voyage_data;
  ais::aisDynamicData _dynamic_data;
  
  int _vessel_id = -1;
  
  QMutex _mutex;
  
signals:
  void updateVessel();
  
public slots:
  void newSelfGeoPosition(const geo::GEOPOSITION &geopos);
  
};

class ais::SvAISEmitter : public QThread
{
  Q_OBJECT
  
public:
  SvAISEmitter(ais::aisStaticData *sdata, ais::aisVoyageData *vdata, ais::aisDynamicData *ddata, QMutex *mutex);
  ~SvAISEmitter();
  
  void stop();
  
  int vessel_id = -1;
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started = false;
  bool _finished = false;
  
  ais::aisStaticData *_static_data = nullptr;
  ais::aisVoyageData *_voyage_data = nullptr;
  ais::aisDynamicData *_dynamic_data = nullptr;
  
  QMutex *_mutex = nullptr;
  
signals:
  void ais_static_data(ais::aisStaticData *data);
  void ais_voyage_data(ais::aisVoyageData *data);
  void ais_dynamic_data(ais::aisDynamicData *data);
  
};

#endif // SV_AIS_H
