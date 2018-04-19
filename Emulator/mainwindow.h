#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QCommandLineParser>

#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_log.h"

#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"
#include "sv_ais.h"
#include "sv_lag.h"
#include "sv_navtex.h"
#include "sv_echo.h"

#include "sv_mapobjects.h"
#include "sv_vesseleditor.h"
#include "sv_navtexeditor.h"

#include "sql_defs.h"
#include "sv_area.h"
#include "sv_exception.h"
#include "nmea.h"
#include "sv_serialeditor.h"
#include "sv_networkeditor.h"

#include "../qcustomplot/qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  QString ARG_LAG_MSGTYPE = "msgtype";
  QString ARG_AIS_RECEIVERANGE = "receive_range";
  QString ARG_NAV_RECV_FREQ = "recv_freq";
  QString ARG_ECHO_BEAM_COUNT = "beam_count";
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  bool init();
  
  svlog::SvLog log;
  
public slots:
  gps::gpsInitParams readGPSInitParams(QSqlQuery* q, ais::aisDynamicData &dynamic_data, QDateTime lastUpdate);
  ais::aisStaticVoyageData readAISStaticVoyageData(QSqlQuery* q);
  ais::aisDynamicData readAISDynamicData(QSqlQuery* q);
  ais::aisNavStat readNavStat(QSqlQuery* q);
  
  void updateMapObjectInfo(SvMapObject* mapObject);
  
//  QString fillVesselNavStatus(QSqlQuery* q) const;
  
  
private:
  enum States {
    sRunning,
    sStopping,
    sRunned,
    sStopped
  };

  Ui::MainWindow *ui;
  
  area::SvArea* _area;
  
  gps::SvGPS* _self_gps = nullptr;
  ais::SvSelfAIS* _self_ais = nullptr;
  vsl::SvVessel* _self_vessel = nullptr;
  lag::SvLAG* _self_lag = nullptr;
  ech::SvECHO* _self_multi_echo;
  nav::SvNAVTEX* _navtex = nullptr;
  
//  SvMapObjectSelfVessel* _self_map_obj;
  QMap<int, SvMapObjectSelfVessel*> _vessels_map_obj;
  
  States _current_state = sStopped;
  
  QSqlQuery* _query;
  SvException _exception;
  
  QTimer _tm;
  
  SerialPortParams _lag_serial_params = SerialPortParams(idev::sdtLAG);
  SerialPortParams _ais_serial_params = SerialPortParams(idev::sdtSelfAIS);
  SerialPortParams _navtex_serial_params = SerialPortParams(idev::sdtNavtex);
  NetworkParams _echo_network_params = NetworkParams(idev::sdtEcho);
  
  int _selected_vessel_id = -1;
  
  QFont _font_default;
  QFont _font_inactive;
  QFont _font_nolink;
  
  QVariant parse_args(QString args, QString arg);
  
  QTimer _timer_x10;
  
  QCustomPlot *_customplot;
  int _echoXcounter = 0;
  
  void updateGPSInitParams(gps::SvGPS* g);
  
public slots:
  void update_vessel_by_id(int id);
  
private slots:
  void stateChanged(States state);
  void on_bnCycle_clicked();
  
  void area_selection_changed();

  vsl::SvVessel* createSelfVessel(QSqlQuery* q);
  vsl::SvVessel* createOtherVessel(QSqlQuery* q);
  
  nav::SvNAVTEX* createNavtex(QSqlQuery* q);

  
  void on_actionNewVessel_triggered();
  void on_actionEditVessel_triggered();
  
//  void on_listVessels_currentRowChanged(int currentRow);
  
  void on_bnAISEditSerialParams_clicked();
  
  void on_bnLAGEditSerialParams_clicked();
  
  void on_bnNAVTEKEditSerialParams_clicked();
    
  void currentVesselListItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
  
  void editVessel(int id);
  
  void on_listVessels_doubleClicked(const QModelIndex &index);
  
  void on_bnSetActive_clicked();
  
  void update_NAVTEX_data();
  
  void read_devices_params();
  void save_devices_params();
  
  void on_bnEditNAVTEX_clicked();
  
  void on_bnNAVTEXAlarmSend_clicked();
  
  void on_bnLAGAlarmSend_clicked();
  
  void on_bnAISAlarmSend_clicked();
  
  void on_cbLAGMessageType_currentIndexChanged(int index);
  
  void on_bnCycle_pressed();
  
  void setX10Emulation();
  
  void on_bnCycle_released();
  
  void on_bnDropDynamicData_clicked();
  
  void on_bnECHOEditNetworkParams_clicked();
  
  void on_echoBeamsUpdated(const ech::Beam *bl);
  
signals:
  void newState(States state);
  
  void setMultiplier(quint32 multiplier);
  
  void startGPSEmulation(quint32 msecs);
  void startAISEmulation(quint32 msecs);
  void startLAGEmulation(quint32 msecs);
  void startNAVTEXEmulation(quint32 msecs);
  void startECHOEmulation(quint32 msecs);
  
  void stopGPSEmulation();
  void stopAISEmulation();
  void stopLAGEmulation();
  void stopNAVTEXEmulation();
  void stopECHOEmulation();
  
  void new_lag_message_type(lag::MessageType msgtype);
  
};

#endif // MAINWINDOW_H
