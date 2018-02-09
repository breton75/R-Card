#ifndef SV_GPS_H
#define SV_GPS_H

#include <QObject>
#include <QThread>
#include <QIODevice>
#include <QMetaType>
#include <QApplication>
#include <QTime>

#include "types.h"
#include "geo_calculations.h"

namespace vsl {

  struct VesselStaticData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  struct VesselDynamicData {              // Динамическая информация
      
    geo::POSITION position;               // Местоположение (широта и долгота)
                                          // Время (UTC)
                                          // Возраст информации (как давно обновлялась)
                                          // Курс истинный (относительно грунта), курсовой угол
                                          // Скорость истинная
                                          // Угол крена, дифферента
                                          // Угол килевой качки
                                          // Угловая скорость поворота
    QString nav_status;                   // Навигационный статус (к примеру: Лишен возможности управляться или Ограничен в возможности маневрировать)

  };
  
  struct VesselVoyageData {               // Рейсовая информация

    QString destination;                  // Пункт назначения
                                          // Время прибытия (ЕТА)
    qreal draft;                          // Осадка судна
    QString cargo;                        // Информация о грузе (класс/категория груза)
    quint32 team;                         // Количество людей на борту
                                          // Сообщения для предупреждения и обеспечения безопасности грузоперевозки
  };

  struct InitParams {
    
    quint32 gps_timeout;
    
    quint32 course;
    quint32 course_change_segment;
    quint32 course_change_ratio;
    
    quint32 speed;
    quint32 speed_change_segment;
    quint32 speed_change_ratio;
  };
  
  class SvGPSThread;
  class SvVessel;
  
}

//class SvGPSThread;

class vsl::SvVessel : public QObject
{
  Q_OBJECT
  
public:
  SvVessel(QObject* parent);
    
  ~SvVessel(); 
  
  void setInitParams(const vsl::InitParams& params) { _init_params = params; }
  void setStaticData(const vsl::VesselStaticData& sdata) { _static_data = sdata; }
  void setVoyageData(const vsl::VesselVoyageData& vdata) { _voyage_data = vdata; }
  void setPosition(const geo::POSITION& position) { _dynamic_data.position = position; }
  void setNavStatus(const QString& status) { _dynamic_data.nav_status = status; }
  
  void start();
  
  QString get();
  
  int id = -1;
  
private:
  vsl::SvGPSThread* _gps_thr = nullptr;
  
  vsl::VesselStaticData _static_data;
  vsl::VesselVoyageData _voyage_data;
  vsl::VesselDynamicData _dynamic_data;
  
  vsl::InitParams _init_params;
  
public slots:
  void new_position(qreal lon, qreal lat, qreal course);
  
  
};

class vsl::SvGPSThread : public QThread
{
  Q_OBJECT
  
public:
  SvGPSThread(const vsl::InitParams& initParams, geo::BOUNDS* bounds = nullptr);
  ~SvGPSThread();
  
  void stop();
    
private:
  void run() Q_DECL_OVERRIDE;
  
  bool _started = false;
  bool _finished = false;
  
  vsl::InitParams _init_params;
  geo::BOUNDS* _bounds = nullptr;
  
  geo::POSITION _current_position; 
  
  
signals:
  void new_position(qreal lon, qreal lat, qreal course);
  
};

#endif // SV_GPS_H
