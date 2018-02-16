#include "sv_vessel.h"

vsl::SvVessel* SELF_VESSEL;
QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent, quint32 id, bool self) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
  this->id = id;
  _self = self;

   
   
}

vsl::SvVessel::~SvVessel()
{
  deleteLater();
  
}

void vsl::SvVessel::updateVessel()
{
  emit updateMapObjectPos(_map_object, _ais->aisDynamicData()->geoposition);
}

void vsl::SvVessel::start()
{

}
