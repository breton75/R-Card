#ifndef NMEA_H
#define NMEA_H
 
#include <QMap>

namespace nmea {

  QMap<int, QChar> SIXBIT_SYMBOLS = {
    
    {0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
    {10, ':'}, {11, ';'}, {12, '<'}, {13, '='}, {14, '>'}, {15, '?'}, {16, '@'}, {17, 'A'}, {18, 'B'}, {19, 'C'}, 
    {20, 'D'}, {21, 'E'}, {22, 'F'}, {23, 'G'}, {24, 'H'}, {25, 'I'}, {26, 'J'}, {27, 'K'}, {28, 'L'}, {29, 'M'}, 
    {30, 'N'}, {31, 'O'}, {32, 'P'}, {33, 'Q'}, {34, 'R'}, {35, 'S'}, {36, 'T'}, {37, 'U'}, {38, 'V'}, {39, 'W'}, 
    {40, '\''}, {41, 'a'}, {42, 'b'}, {43, 'c'}, {44, 'd'}, {45, 'e'}, {46, 'f'}, {47, 'g'}, {48, 'h'}, {49, 'i'}, 
    {50, 'j'}, {51, 'k'}, {52, 'l'}, {53, 'm'}, {54, 'n'}, {55, 'o'}, {56, 'p'}, {57, 'q'}, {58, 'r'}, {59, 's'}, 
    {60, 't'}, {61, 'u'}, {62, 'v'}, {63, 'w'}, 
    
  };

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
  
  
  QMap<NMEASentence, QString> SENTENCES = {{ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement, "ABK"},
                                           {AIR_InterrogationRequest, "AIR"},
                                           {VDO_UAIS_VHF_Datalink_Own_vessel_report, "VDO"},
                                           {VDM_UAIS_VHF_Datalink_Message, "VDM"},
                                           {SSD_UAIS_Ship_Static_Data, "SSD"},
                                           {VSD_UAIS_Voyage_Static_Data, "VSD"},
                                           {Q_Query, "Q"}};


  /** NMEA 0183 Version 3.01 --- TABLE 7 - SIX-BIT BINARY FIELD CONVERSION TABLE **/
  inline QString str_to_6bit(const QString& str);

}



#endif // NMEA_H
