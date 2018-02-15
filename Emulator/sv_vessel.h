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
#include "sv_mapobjects.h"

namespace vsl {

  class SvVessel;
  
}

class vsl::SvVessel : public QObject
{
  Q_OBJECT
  
public:
  SvVessel(QObject* parent, quint32 id, bool self = false);
    
  ~SvVessel(); 

  void start();

  bool isSelf() { return _self; }
  
  void mountAIS(ais::SvAIS* ais) { _ais = ais; }
  void mountGPS(gps::SvGPS* gps) { _gps = gps; }
//  void mountLAG(lag::SvLAG* lag_dev) { _lag_dev = lag_dev; }
  
  void assignMapObject(SvMapObjectVesselAbstract* map_object) { _map_object = map_object; }
  SvMapObjectVesselAbstract* mapObject() { return _map_object; }
  
  QString get();
  
  int id = -1;

  gps::SvGPS* gps() { return _gps; }
  ais::SvAIS* ais() { return _ais; }
  
  geo::GEOPOSITION currentGeoPosition() const { return _current_geo_position; }
  void setGeoPosition(const geo::GEOPOSITION& current_geo_position) { _current_geo_position = current_geo_position; }
  
  qreal distanceTo(geo::GEOPOSITION& geopos) { 
    return geo::geo1_geo2_distance(_current_geo_position.longtitude, geopos.longtitude, _current_geo_position.latitude, geopos.latitude); }
  
private:
  bool _self;
  geo::GEOPOSITION _current_geo_position;
  
  gps::SvGPS* _gps = nullptr;
  ais::SvAIS* _ais = nullptr;
//  lag::SvLAG* _lag_dev = nullptr;
  
  SvMapObjectVesselAbstract* _map_object = nullptr;
  
signals:
  void updateMapObjectPos(SvMapObject* mapObject, const geo::GEOPOSITION& geopos);
  
public slots:
  void newGeoPosition(const geo::GEOPOSITION& geopos);
  
  
};


#endif // SV_VESSEL_H
