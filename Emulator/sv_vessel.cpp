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
  delete _gps;
  delete _ais;
  delete _lag;
  
  deleteLater();
  
}

void vsl::SvVessel::updateVessel()
{
    _map_object->setGeoPosition(_ais->dynamicData()->geoposition);
    emit updateMapObjectPos(_map_object, _ais->dynamicData()->geoposition);
}

