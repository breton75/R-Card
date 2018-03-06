#include "sv_vessel.h"

//vsl::SvVessel* SELF_VESSEL;
//QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent, quint32 id/*, bool self*/) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
  this->id = id;
//  _self = self;

   
   
}

vsl::SvVessel::~SvVessel()
{
  delete _ais;
  delete _gps;
//  delete _lag;
  
  deleteLater();
  
}

void vsl::SvVessel::updateVessel()
{
////  if(_self_vessel) qDebug() << _self_vessel->ais()->receiveRange();
  
//  if(!_self_vessel || (_self_vessel && (distanceToSelfVessel() < _self_vessel->ais()->receiveRange() * 1000))) {
    _map_object->setGeoPosition(_ais->getDynamicData()->geoposition);
    emit updateMapObjectPos(_map_object, _ais->getDynamicData()->geoposition);
//  }
}

void vsl::SvVessel::start()
{

}
