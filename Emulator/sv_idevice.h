#ifndef SV_IDEVICE_H
#define SV_IDEVICE_H

#include <QObject>
#include <QMutex>
#include <QDialog>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>
#include <QMetaType>

#include "geo.h"

namespace idev {

  enum SvSimulatedDeviceTypes {
    sdtUndefined = -1,
    sdtVessel,
    sdtNavtek
  };

  class SvIDevice;

}


/** ----------- SvIDevice ------------ **/
class idev::SvIDevice : public QObject
{
    Q_OBJECT
    
public:
  SvIDevice() { }
    
//    qRegisterMetaType<svidev::MeasuredData>("svidev::mdata_t"); 
    
//  }
  
  virtual ~SvIDevice() { }
  
  virtual idev::SvSimulatedDeviceTypes type() const { return idev::sdtUndefined; }
  
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual void stop() = 0;
  
//  QMutex mutex;
  
  void setLastError(const QString& lastError) { _last_error = lastError; }
  QString lastError() { return _last_error; }
  
  void setDeviceType(idev::SvSimulatedDeviceTypes type) { _type = type; }
  idev::SvSimulatedDeviceTypes deviceType() { return _type; }
  
  void setOpened(bool isOpened) { _isOpened = isOpened; }
  bool isOpened() { return _isOpened; }
  
  void setCoordinates(const geo::COORD& coord) { _coord = coord; }
//  void setCourse(const qreal course) { _course = course; }
//  void setUTC(const QDateTime& utc) { _utc = utc; }
//  void setNavStatus(const QString& navstat) { _navstat = navstat; }
  
  geo::COORD coordinates() const { return _coord; }
//  qreal course() const { return _course; }
//  qreal angular_speed() const { return _angular_speed; } // Угловая скорость поворота
//  QDateTime utc() const { return _utc; }
//  QString navstat() const { return _navstat; }
  
  qreal distanceTo(geo::COORD& coord) { 
    return geo::geo1_geo2_distance(_coord.longtitude, coord.longtitude, _coord.latitude, coord.latitude); }
  
protected:
  quint32 _id;
  idev::SvSimulatedDeviceTypes _type;
  
//  geo::POSITION _position;
  geo::COORD _coord;
  
  bool _isOpened;
  QString _last_error;
  
    
//signals:
//  void new_data(const svidev::mdata_t& data);
      
};

#endif // SV_IDEVICE_H
