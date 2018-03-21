#include "nmea.h"


QMap<int, QChar> SIXBIT_SYMBOLS = {
  
  {0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
  {10, ':'}, {11, ';'}, {12, '<'}, {13, '='}, {14, '>'}, {15, '?'}, {16, '@'}, {17, 'A'}, {18, 'B'}, {19, 'C'}, 
  {20, 'D'}, {21, 'E'}, {22, 'F'}, {23, 'G'}, {24, 'H'}, {25, 'I'}, {26, 'J'}, {27, 'K'}, {28, 'L'}, {29, 'M'}, 
  {30, 'N'}, {31, 'O'}, {32, 'P'}, {33, 'Q'}, {34, 'R'}, {35, 'S'}, {36, 'T'}, {37, 'U'}, {38, 'V'}, {39, 'W'}, 
  {40, '\''}, {41, 'a'}, {42, 'b'}, {43, 'c'}, {44, 'd'}, {45, 'e'}, {46, 'f'}, {47, 'g'}, {48, 'h'}, {49, 'i'}, 
  {50, 'j'}, {51, 'k'}, {52, 'l'}, {53, 'm'}, {54, 'n'}, {55, 'o'}, {56, 'p'}, {57, 'q'}, {58, 'r'}, {59, 's'}, 
  {60, 't'}, {61, 'u'}, {62, 'v'}, {63, 'w'}, 
  
};

QMap<nmea::NMEASentence, QString> SENTENCES = {{nmea::ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement, "ABK"},
                                         {nmea::AIR_InterrogationRequest, "AIR"},
                                         {nmea::VDO_UAIS_VHF_Datalink_Own_vessel_report, "VDO"},
                                         {nmea::VDM_UAIS_VHF_Datalink_Message, "VDM"},
                                         {nmea::SSD_UAIS_Ship_Static_Data, "SSD"},
                                         {nmea::VSD_UAIS_Voyage_Static_Data, "VSD"},
                                         {nmea::Q_Query, "Q"}};

inline QString str_to_6bit(const QString& str)
{
  QString result = "";
  
  for(int i = 1; i < str.count(); i++) {
    
    quint8 n = quint8(str.at(i).unicode());
    
    n += 0x28;  // 00101000
    n = n > 0x80 ? n + 0x20 : n + 0x28;
    n &= 0x3f;
    
    if(SIXBIT_SYMBOLS.find(n) == SIXBIT_SYMBOLS.end()) {
      
      result.clear();
      break;
      
    }
    
    result.append(SIXBIT_SYMBOLS.value(n));

  }
  
  return result;
  
}

QString nmea::ais_message_1(quint8 repeat_indicator, quint32 mmsi, quint8 nav_status, qint8 rot, geo::GEOPOSITION &geopos) //, int true_heading, QDateTime utc,
//                 int manouevre, int raim, int communication_state)
{
  QString result = "";
  
  quint8 b6[28];
  memset(&b6, 0, 28);
  
  /// Message ID
  b6[0] = 1;
  
  /// Repeat indicator
  b6[1] = (repeat_indicator & 0x03) << 4; // 2 значащих бита
  
  /// User ID
  quint64 mmsi64 = mmsi & 0x3FFFFFFF; // 30 значащих бит
  mmsi64 <<= 32; // << 32;
  
  for(int i = 0; i < 6; i++) {
    mmsi64 >>= 2;
    b6[1 + i] += (mmsi64 >> (56 - i * 8));
    mmsi64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
  }
  
  /// Navigational status 
  b6[6] += (nav_status & 0x0F); // 4 значащих бит
  
  /// Rate of turn
  b6[7] = rot >> 2; // 8 значащих бит
  b6[8] = (rot << 4) & 0x3F;
  
  ///  Speed over ground
  quint16 sog16 = quint16(trunc(geopos.speed * 10)) & 0x03FF; // 10 значащих бит
  b6[8] += (sog16 >> 6);
  b6[9] += (sog16 & 0x003F);
  
  /// Position accuracy 
  b6[10] = geopos.accuracy << 5;  // 1 значащий бит
  
  /// Longitude in 1/10000 minutes
  quint64 lon64 = quint64(trunc(geopos.longtitude * 10000 * 60))  & 0x0FFFFFFF; // 28 значащих бит
  lon64 <<= 33;  //! 33

  for(int i = 0; i < 5; i++) {
    b6[10 + i] += (lon64 >> (56 - i * 8));
    lon64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
    lon64 >>= 2;
  }
  
  /// Latitude in 1/10000 minutes
  quint64 lat64 = quint64(trunc(geopos.latitude * 10000 * 60))  & 0x07FFFFFF; // 27 значащих бит
  lat64 <<= 32;  //! 32
  
  for(int i = 0; i < 6; i++) {
    lat64 >>= 2;
    b6[14 + i] += (lat64 >> (56 - i * 8));
    lat64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
  }  
  
  /// Course over ground in 1/10 degrees
  quint32 cog32 = quint32(trunc(geopos.course * 10))  & 0x0FFF;  // 12 значащих бит
  cog32 = cog32 << 16;  //! 16
  
  for(int i = 0; i < 3; i++) {
    b6[19 + i] += (cog32 >> (24 - i * 8));
    cog32 &= (0x00FFFFFF >> (i * 8));
    cog32 >>=  2;
  }
  
  /// дальше пока подстановки
  /// True Heading 
  b6[21] += 0x0F;
  b6[22] += 0x3E;
  
  /// UTC second when report generated 
  quint8 s = geopos.utc.time().second();
  b6[22] += (s & 0x63) >> 5;
  b6[23] += (s & 0x31) >> 2;
  
  /// Regional Application + Spare + RAIM Flag 
  b6[24] += 0;
  
  /// Communications State
  b6[24] += 0;          // utc direct
  b6[25] += 0x01 << 2;  // 1 frame remaining until a new slot is selected
  
  quint8 h = geopos.utc.time().hour();
  b6[25] += (h & 0x1F) >> 3;
  b6[26] += (h & 0x07) << 3;
  
  quint8 m = geopos.utc.time().minute();
  b6[26] += (m & 0x7F) >> 4;
  b6[27] += (m & 0x0F) << 2;
  
  /// форимруем сообщение
  QString msg = "";
  for(int i = 0; i < 28; i++)
    msg.append(SIXBIT_SYMBOLS.value(b6[i]));  // message id
  
  result = QString("$AIVDM,1,1,,A,%1,0*").arg(msg);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 1; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  
  result.append(QString("%1%2%3").arg(src, 2, 16).arg(QChar(13)).arg(QChar(10)).replace(' ', '0'));
  
  return result;
  
}




QString nmea::lag_VBW(const geo::GEOPOSITION& geopos)
{
  QString result = QString("$VDVBW,%1,%2,A,%3,%4,A,%5,A,%6,A*")
                .arg(geopos.speed, 0, 'f', 1)
                .arg(0.0, 0, 'g', 1)
                .arg(geopos.speed, 0, 'f', 1)
                .arg(0.0, 0, 'g', 1)
                .arg(0.0, 0, 'g', 1)
                .arg(0.0, 0, 'g', 1);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 1; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  
  result.append(QString("%1%2%3").arg(src, 2, 16).arg(QChar(13)).arg(QChar(10)));
  
  
  return result;
}
