#include "nmea.h"

inline QString str_to_6bit(const QString& str)
{
  QString result = "";
  
  for(int i = 1; i < str.count(); i++) {
    
    quint8 n = quint8(str.at(i).unicode());
    
    n += 0x28;  // 00101000
    n = n > 0x80 ? n + 0x20 : n + 0x28;
    n &= 0x3f;
    
    if(nmea::SIXBIT_SYMBOLS.find(n) == nmea::SIXBIT_SYMBOLS.end()) {
      
      result.clear();
      break;
      
    }
    
    result.append(nmea::SIXBIT_SYMBOLS.value(n));

  }
  
  return result;
  
}
