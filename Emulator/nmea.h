#ifndef NMEA_H
#define NMEA_H
 
#include <QMap>

//#include "sv_ais.h"

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

  
  QString message1(quint8 repeat_indicator, quint32 mmsi, quint8 nav_status, qint8 rot, qreal sog,
                   quint8 pos_accuracy, qreal longtitude, qreal latitude, qreal cog); //, int true_heading, QDateTime utc,
//                   int manouevre, int raim, int communication_state);
  
}



#endif // NMEA_H
