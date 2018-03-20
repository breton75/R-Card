#ifndef SV_SERIALEDITOR_H
#define SV_SERIALEDITOR_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMap>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"

namespace Ui {
class SvSerialEditorDialog;
}

enum SerialDevices {
  sdLAG = 0,
  sdAIS,
  sdNAVTEK,
  sdECHO
};

struct SerialPortParams {
//public:
  SerialPortParams() {  }
  SerialPortParams(SerialDevices type) { dev_type = type; }
  SerialDevices dev_type;
  QString name = "COM1";
  quint32 baudrate = 9600;
  QSerialPort::DataBits databits = QSerialPort::Data8;
  QSerialPort::Parity parity = QSerialPort::NoParity;
  QSerialPort::StopBits stopbits = QSerialPort::OneStop;
  QSerialPort::FlowControl flowcontrol = QSerialPort::NoFlowControl;
};

class SvSerialEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum ResultCode { rcSqlError = -1, rcNoError };
  
  explicit SvSerialEditor(SerialPortParams params, QWidget *parent = 0);
  ~SvSerialEditor();
  
  SerialPortParams params;
 
  QString last_error() { return _last_error; }
  
private:
  Ui::SvSerialEditorDialog *ui;
  
  QList<int> baudartes_list = {75, 115, 134, 150, 300, 600, 1200, 1800, 2400, 4800, 7200, 9600, 14400, 19200, 38400, 57600, 115200, 128000};
  QMap<QSerialPort::DataBits, QString> databits_list = {{QSerialPort::Data5, "5"}, 
                                                    {QSerialPort::Data6, "6"},
                                                    {QSerialPort::Data7, "7"},
                                                    {QSerialPort::Data8, "8"}};
  
  QMap<QSerialPort::Parity, QString> parities_list = {{QSerialPort::NoParity, "Нет"},
                                                 {QSerialPort::EvenParity, "Чет"},
                                                 {QSerialPort::OddParity, "Нечет"},
                                                 {QSerialPort::SpaceParity, "Пробел"},
                                                 {QSerialPort::MarkParity, "Маркер"}};
  
  QMap<QSerialPort::StopBits, QString> stopbits_list = {{QSerialPort::OneStop, "1"},
                                                   {QSerialPort::OneAndHalfStop, "1,5"},
                                                   {QSerialPort::TwoStop, "2"}};
  
  QMap<QSerialPort::FlowControl, QString> flowcontrols_list = {{QSerialPort::NoFlowControl, "Нет"},
                                                   {QSerialPort::HardwareControl, "Аппаратное"},
                                                   {QSerialPort::SoftwareControl, "Программное"}};
  
  QMap<SerialDevices, QString> devices_list = {{sdLAG, "ЛАГ"}, {sdAIS, "АИС"}, {sdNAVTEK, "НАВТЭК"}};
  
  SvException _exception;
  QString _last_error = "";
  
  void accept();
 
};

#endif // SV_SERIALEDITOR_H
