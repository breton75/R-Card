#ifndef SV_VESSEL_H
#define SV_VESSEL_H

#include <QObject>
#include <QThread>
#include <QIODevice>
#include <QMetaType>
#include <QApplication>
#include <QTime>

#include "types.h"
#include "geo.h"

#include "sv_gps.h"
#include "sv_ais.h"

namespace vsl {

  class SvVessel;
  
}

class vsl::SvVessel : public QObject
{
  Q_OBJECT
  
public:
  SvVessel(QObject* parent, quint32 id);
    
  ~SvVessel(); 

  void start();

  void mountAIS(ais::SvAIS* ais_dev) { _ais_dev = ais_dev; }
  void mountGPS(gps::SvGPS* gps_dev) { _gps_dev = gps_dev; }
//  void mountLAG(lag::SvLAG* lag_dev) { _lag_dev = lag_dev; }
  
  QString get();
  
  int id = -1;

  geo::COORD coordinates() const { return _coord; }
  void setCoordinates(const geo::COORD& coord) { _coord = coord; }
  
  qreal distanceTo(geo::COORD& coord) { 
    return geo::geo1_geo2_distance(_coord.longtitude, coord.longtitude, _coord.latitude, coord.latitude); }
  
private:
  geo::COORD _coord;
  
  gps::SvGPS* _gps_dev = nullptr;
  ais::SvAIS* _ais_dev = nullptr;
//  lag::SvLAG* _lag_dev = nullptr;
  
public slots:
  void new_position(qreal lon, qreal lat, qreal course);
  
  
};


#endif // SV_VESSEL_H
