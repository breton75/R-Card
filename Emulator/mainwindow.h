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

#include "sv_area.h"
#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"
#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"
#include "sv_ais.h"
#include "sv_lag.h"
#include "sv_mapobjects.h"
#include "sv_vesseleditor.h"

#include "sql_defs.h"
#include "../../svlib/sv_log.h"
#include "sv_exception.h"
#include "nmea.h"
#include "sv_serialeditor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
  bool init();
  
  svlog::SvLog log;
  
public slots:
//  void on_bnStart_clicked();
 
//  gps::GPSParams fillVesselInitParams(QSqlQuery* q) const;
//  vsl:: setVesselData(QSqlQuery* q);
  gps::gpsInitParams getGPSInitParams(QSqlQuery* q, ais::aisDynamicData &dynamic_data, int vessel_id);
  ais::aisStaticData getAISStaticData(QSqlQuery* q);
  ais::aisVoyageData getAISVoyageData(QSqlQuery* q);
  ais::aisDynamicData getAISDynamicData(QSqlQuery* q);
  
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
  
//  SvMapObjectSelfVessel* _self_map_obj;
  QMap<int, SvMapObjectSelfVessel*> _vessels_map_obj;
  
  States _current_state = sStopped;
  
  QSqlQuery* _query;
  SvException _exception;
  
  QTimer _tm;
  
  SerialPortParams _lag_serial_params = SerialPortParams(sdLAG);
  SerialPortParams _ais_serial_params = SerialPortParams(sdAIS);
//  QSerialPort _navteks_serial;
//  QSerialPort _echo_serial;
  
  int _selected_vessel_id = -1;
  
public slots:
  void update_vessel_by_id(int id);
  
private slots:
  void stateChanged(States state);
  void on_bnCycle_clicked();
  
  void on_area_selection_changed();
  
//  void initGeposition(gps::gpsInitParams& gpsParams, const ais::aisDynamicData& dynamicData);
  
  vsl::SvVessel* createSelfVessel(QSqlQuery* q);
  vsl::SvVessel* createOtherVessel(QSqlQuery* q);
  
  void on_actionNewVessel_triggered();
  void on_actionEditVessel_triggered();
  
//  void on_listVessels_currentRowChanged(int currentRow);
  
  void on_bnAISEditSerialParams_clicked();
  
  void on_bnLAGEditSerialParams_clicked();
  
  void on_bnNAVTEKEditSerialParams_clicked();
    
  void currentVesselListItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
  
  void editVessel(int id);
  
signals:
  void newState(States state);
  
  void setMultiplier(quint32 multiplier);
  
  void startGPSEmulation(quint32 msecs);
  void startAISEmulation(quint32 msecs);
  void startLAGEmulation(quint32 msecs);
//  void startNAVEmulation(quint32 msecs);
  
  void stopGPSEmulation();
  void stopAISEmulation();
  void stopLAGEmulation();
//  void stopNAVEmulation();
  
};

#endif // MAINWINDOW_H
