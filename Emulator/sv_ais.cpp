#include "sv_ais.h"

ais::SvAIS* SELF_AIS;

ais::SvAIS::SvAIS(int vessel_id)
{
  _vessel_id = vessel_id;
}

ais::SvAIS::~SvAIS()
{
  deleteLater();
}
  
bool ais::SvAIS::open()
{
  _isOpened = true;
}
void ais::SvAIS::close()
{
  _isOpened = false;
}

bool ais::SvAIS::start(quint32 msecs)
{
  
}

void ais::SvAIS::stop()
{
  
}

void ais::SvAIS::newSelfGeoPosition(const geo::GEOPOSITION& geopos)
{
  _dynamic_data.geoposition = geopos;
//  qDebug() << "ais::newgepos" << _dynamic_data.geoposition.latitude << _dynamic_data.geoposition.longtitude << _dynamic_data.geoposition.course << _dynamic_data.geoposition.speed;
  emit updateVessel(); 
}




/** ******  EMITTER  ****** **/
ais::SvAISEmitter::SvAISEmitter(aisStaticData *sdata, aisVoyageData *vdata, aisDynamicData *ddata, QMutex *mutex)
{
  
}
  
ais::SvAISEmitter::~SvAISEmitter()
{ 
  stop();
  deleteLater();
}

void ais::SvAISEmitter::stop()
{
  
}

void ais::SvAISEmitter::run()
{
  
}
