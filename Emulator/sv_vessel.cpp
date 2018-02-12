#include "sv_vessel.h"

vsl::SvVessel* SELF;
QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
//   _init_params = params;
//   _static_data = sdata;
//   _voyage_data = vdata;
   
//   _dynamic_data.position.setCoordinates(_init_params.);
   

   
   
}

vsl::SvVessel::~SvVessel()
{
  deleteLater();
  
}

void vsl::SvVessel::new_position(qreal lon, qreal lat, qreal course)
{
//  data.
}

void vsl::SvVessel::start()
{

}
