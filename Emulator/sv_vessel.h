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
#include "sv_lag.h"
#include "sv_mapobjects.h"

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

//  bool isSelf() { return (_self_vessel == nullptr); }
  
//  void setSelfVessel(SvVessel* vsl) { _self_vessel = vsl; }
  
  void mountAIS(ais::SvAIS* ais) { _ais = ais; }
  void mountGPS(gps::SvGPS* gps) { _gps = gps; }
  void mountLAG(lag::SvLAG* lag) { _lag = lag; }
  
  void assignMapObject(SvMapObjectVesselAbstract* map_object) { _map_object = map_object; }
  SvMapObjectVesselAbstract* mapObject() { return _map_object; }
  
  int id = -1;

  gps::SvGPS* gps() { return _gps; }
  ais::SvAIS* ais() { return _ais; }
  lag::SvLAG* lag() { return _lag; }
  
//  geo::GEOPOSITION currentGeoPosition() const { return _current_geo_position; }
//  void setGeoPosition(const geo::GEOPOSITION& current_geo_position) { _current_geo_position = current_geo_position; }
  
//  qreal distanceTo(geo::GEOPOSITION& geopos) { 
//    return geo::geo2geo_distance(ais()->aisDynamicData()->geoposition, geopos); }
  
//  qreal distanceToSelfVessel() { if(!_self_vessel) return 0.0; 
//    else 
//      return geo::geo2geo_distance(ais()->aisDynamicData()->geoposition, _self_vessel->ais()->aisDynamicData()->geoposition); }
  
private:
//  bool _self;
//  geo::GEOPOSITION _current_geo_position;
  
//  SvVessel* _self_vessel = nullptr;
  
  gps::SvGPS* _gps = nullptr;
  ais::SvAIS* _ais = nullptr;
  lag::SvLAG* _lag = nullptr;
  
  SvMapObjectVesselAbstract* _map_object = nullptr;
  
signals:
  void updateMapObjectPos(SvMapObject* mapObject, const geo::GEOPOSITION& geopos);
  
public slots:
  void updateVessel();
  
  
};


#endif // SV_VESSEL_H
