#ifndef NMEA_H
#define NMEA_H
 
#include <QMap>
#include <QDebug>

#include "geo.h"
#include "sv_ais.h"

namespace nmea {


  enum NMEASentence {
    
    ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement,
    AIR_InterrogationRequest,
    
//    OSD_OwnShipData,
    VDO_UAIS_VHF_Datalink_Own_vessel_report,
    VDM_UAIS_VHF_Datalink_Message,
    
    SSD_UAIS_Ship_Static_Data,
    VSD_UAIS_Voyage_Static_Data,
    
    Q_Query
  };
  
  


  /** NMEA 0183 Version 3.01 --- TABLE 7 - SIX-BIT BINARY FIELD CONVERSION TABLE **/
  inline QString str_to_6bit(const QString& str);

  
  QString ais_message_1(quint8 repeat_indicator, quint32 mmsi, quint8 nav_status, qint8 rot, geo::GEOPOSITION& geopos); //, int true_heading, QDateTime utc,
//                   int manouevre, int raim, int communication_state);
  
  
  QString lag_VBW(const geo::GEOPOSITION &geopos);
  
}



#endif // NMEA_H
