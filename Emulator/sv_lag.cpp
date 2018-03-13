#include "sv_lag.h"

/** --------- Self LAG --------- **/
lag::SvLAG::SvLAG(int vessel_id, const geo::GEOPOSITION &geopos, svlog::SvLog &log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _log = log;
  
}

lag::SvLAG::~SvLAG()
{
  deleteLater();
}
  
bool lag::SvLAG::open()
{
  connect(&_timer, &QTimer::timeout, this, &lag::SvLAG::write_data);
  
  _isOpened = true;
}

void lag::SvLAG::close()
{
  disconnect(&_timer, &QTimer::timeout, this, &lag::SvLAG::write_data);
  
  _isOpened = false;
}

bool lag::SvLAG::start(quint32 msecs)
{
  _timer.start(msecs);
  
  return true;
}

void lag::SvLAG::stop()
{
  _timer.stop();
}

void lag::SvLAG::newGPSData(const geo::GEOPOSITION& geopos)
{
  _current_geoposition = geopos;
}

void lag::SvLAG::write_data()
{
  _log << svlog::Time << svlog::Data << QString("lag data: %1: spd:%2")
       .arg(_vessel_id) 
       .arg(_current_geoposition.speed, 0, 'g', 2)
       << svlog::endl;
  
}

