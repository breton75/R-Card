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

void ais::SvAIS::new_coordinates(const geo::COORD &coord)
{
  
}
