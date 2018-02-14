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
  struct StaticData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  struct DynamicData {                    // Динамическая информация
      
    geo::POSITION position;               // Местоположение (широта и долгота)
                                          // Время (UTC)
                                          // Возраст информации (как давно обновлялась)
                                          // Курс истинный (относительно грунта), курсовой угол
                                          // Скорость истинная
                                          // Угол крена, дифферента
                                          // Угол килевой качки
                                          // Угловая скорость поворота
    QString navstat;                      // Навигационный статус (к примеру: Лишен возможности управляться или Ограничен в возможности маневрировать)
  
  };
  
  struct VoyageData {                     // Рейсовая информация
  
    QString destination;                  // Пункт назначения
                                          // Время прибытия (ЕТА)
    qreal draft;                          // Осадка судна
    QString cargo;                        // Информация о грузе (класс/категория груза)
    quint32 team;                         // Количество людей на борту
                                          // Сообщения для предупреждения и обеспечения безопасности грузоперевозки
  };
  
  class SvAIS;
  class SvAISTransmitter;
  
}

class ais::SvAIS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvAIS(int vessel_id);
  ~SvAIS(); 
  
  int vesselId() { return _vessel_id; }
  
  void setStaticData(const ais::StaticData& sdata) { _static_data = sdata; }
  void setVoyageData(const ais::VoyageData& vdata) { _voyage_data = vdata; }
  void setDynamicData(const ais::DynamicData& ddata) { _dynamic_data = ddata; }
  void setPosition(const geo::POSITION& position) { _dynamic_data.position = position; }
  void setNavStatus(const QString& status) { _dynamic_data.navstat = status; }

  ais::StaticData *StaticData() { return &_static_data; }
  ais::VoyageData *VoyageData() { return &_voyage_data; }
  ais::DynamicData *DynamicData() { return &_dynamic_data; }
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtAIS; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
private:
  ais::SvAISTransmitter* _ais_transmitter = nullptr;
  
  ais::StaticData _static_data;
  ais::VoyageData _voyage_data;
  ais::DynamicData _dynamic_data;
  
  int _vessel_id = -1;
  
  QMutex _mutex;
  
public slots:
  void new_location(const geo::LOCATION& location);
  
};

class ais::SvAISTransmitter : public QThread
{
  Q_OBJECT
  
public:
  SvAISTransmitter(ais::StaticData *sdata, ais::VoyageData *vdata, ais::DynamicData *ddata, QMutex *mutex);
  ~SvAISTransmitter();
  
  void stop();
  
  int vessel_id = -1;
  
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started = false;
  bool _finished = false;
  
  ais::StaticData *_static_data = nullptr;
  ais::VoyageData *_voyage_data = nullptr;
  ais::DynamicData *_dynamic_data = nullptr;
  
  QMutex *_mutex = nullptr;
  
signals:
  void ais_static_data(ais::StaticData *data);
  void ais_voyage_data(ais::VoyageData *data);
  void ais_dynamic_data(ais::DynamicData *data);
  
};

#endif // SV_AIS_H
