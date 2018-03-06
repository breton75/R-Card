#include "sv_ais.h"

ais::SvAIS* SELF_AIS;

/** --------- Self AIS --------- **/
ais::SvSelfAIS::SvSelfAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata)
//  ais::SvAIS(vessel_id, sdata, vdata, ddata)

{
  setVesselId(vessel_id);
  
  setStaticData(sdata);
  setVoyageData(vdata);
  setDynamicData(ddata);
  
//  _vessel_id = vessel_id;
//  _static_data = sdata;  
//  _voyage_data = vdata;  
//  _dynamic_data = ddata; 
  
}

ais::SvSelfAIS::~SvSelfAIS()
{
  deleteLater();
}
  
bool ais::SvSelfAIS::open()
{
  _isOpened = true;
}
void ais::SvSelfAIS::close()
{
  _isOpened = false;
}

bool ais::SvSelfAIS::start(quint32 msecs)
{
  
}

void ais::SvSelfAIS::stop()
{
  
}

void ais::SvSelfAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
  _dynamic_data.geoposition = geopos;
  emit updateSelfVessel(); 
}

void ais::SvSelfAIS::on_receive_ais_data(ais::SvAIS* ais, ais::AISDataTypes type)
{
  /** проверяем расстояние. если в радиусе действия антенны, то обрабатываем данные */
  if(distanceTo(ais) < _receive_range) {
    
    /** обрабатываем данные **/
//    switch (type) {
      
//      case ais::ais
        
//        break;
//      default:
//        break;
//    }
    
  }
  
  emit updateVesselById(_vessel_id);
  
}

qreal ais::SvSelfAIS::distanceTo(ais::SvAIS* remoteAIS)
{ 
  if(!remoteAIS) return 0.0; 
  else {
    
    geo::GEOPOSITION g;
    int i = remoteAIS->_dynamic_data.geoposition.course;
    return geo::geo2geo_distance(_dynamic_data.geoposition, g); 
  }
}

//void ais::SvSelfAIS::on_receive_voyage(const ais::aisVoyageData& vdata)
//{
  
//}

//void ais::SvSelfAIS::on_receive_dynamic(ais::SvAIS* ais)
//{
  
//}


/** ----- Vessel AIS ------- **/
ais::SvOtherAIS::SvOtherAIS(int vessel_id, const ais::aisStaticData& sdata, const ais::aisVoyageData& vdata, const ais::aisDynamicData& ddata)
{
  _vessel_id = vessel_id;
  
  _static_data = sdata;  
  _voyage_data = vdata;  
  _dynamic_data = ddata; 
  
}

ais::SvOtherAIS::~SvOtherAIS()
{
  deleteLater();
}
  
bool ais::SvOtherAIS::open()
{
  _isOpened = true; 
}
void ais::SvOtherAIS::close()
{
  _isOpened = false;
}

bool ais::SvOtherAIS::start(quint32 msecs)
{
  connect(&_timer_static, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static);
  connect(&_timer_static, SIGNAL(), this, SLOT()); //  &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_voyage);
  connect(&_timer_static, SIGNAL(), this, SLOT()); //  &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _timer_static.start(10000);
  _timer_voyage.start(5000);
  _timer_dynamic.start(3000);
  
  return true;
}

void ais::SvOtherAIS::stop()
{
  disconnect(&_timer_static, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static);
  disconnect(&_timer_static, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_voyage);
  disconnect(&_timer_static, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _timer_static.stop();
  _timer_voyage.stop();
  _timer_dynamic.stop();
}

void ais::SvOtherAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
//  _mutex.lock();
  _dynamic_data.geoposition = geopos;
//  _mutex.unlock();
}


//void ais::SvAIS::receivedGeoPosition(SvAIS* ais)
//{
//  if(distanceTo(ais) < _receive_range) {
    
//    _dynamic_data.geoposition = ais;
//    emit ais->
//    emit updateVessel(); 
//  }
    
//}



///** ******  EMITTER  ****** **/
//ais::SvAISEmitter::SvAISEmitter(aisStaticData *sdata, aisVoyageData *vdata, aisDynamicData *ddata, QMutex *mutex)
//{
//  _static_data = sdata;
//  _voyage_data = vdata;
//  _dynamic_data = ddata;
  
//  _mutex = mutex;
//}
  
//ais::SvAISEmitter::~SvAISEmitter()
//{ 
//  stop();
//  deleteLater();
//}

//void ais::SvAISEmitter::stop()
//{
  
//}

//void ais::SvAISEmitter::run()
//{
//  while(_started) {
    
//    QTime t = QTime::currentTime();
    
////    if()
    
//  }
//}
