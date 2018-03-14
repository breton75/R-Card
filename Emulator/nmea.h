#ifndef SERIAL_DEFS_H
#define SERIAL_DEFS_H
 
#include <QMap>

namespace nmea {

  enum IncomingSentence {
    
    PSRF_SetSerialPortParams,
    
    AIR_InterrogationRequest,
    BBM_BroadcastBinaryMessage,
    ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement,
    
    OSD_OwnShipData,
    VDO_UAIS_VHF_Datalink_Own_vessel_report,
    VDM_UAIS_VHF_Datalink_Message,
    SSD,
    VSD,
    
    
    Q_Query
  };
  
  enum OutcomingSentence {
    
    PSRF_SetSerialPortParams,
    AIS_InterrogationRequest,
    VDO_OwnVesselReport  
  };
  
  
  
  QMap<IncomingSentence, QString> IN_SENTECES = {{PSRF_SetSerialPortParams, "PSRF100"},
                                                 {AIS_InterrogationRequest, "AIS"},
                                                 {VDO_OwnVesselReport, "VDO"}};



}



#endif // SERIAL_DEFS_H
