#include "sv_vessel.h"

vsl::SvVessel* SELF_VESSEL;
QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent, quint32 id, bool self) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
  this->id = id;
  _self = self;
  
//   _init_params = params;
//   _static_data = sdata;
//   _voyage_data = vdata;
   
//   _dynamic_data.position.setCoordinates(_init_params.);
   

   
   
}

vsl::SvVessel::~SvVessel()
{
  deleteLater();
  
}

void vsl::SvVessel::newGeoPosition(const geo::GEOPOSITION& geopos)
{
  _current_geo_position = geopos;
  emit updateMapObjectPos(_map_object, geopos);
}

void vsl::SvVessel::start()
{

}
