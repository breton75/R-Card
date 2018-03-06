#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "sv_area.h"
#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"
#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"
#include "sv_ais.h"
#include "sv_mapobjects.h"
#include "sv_vesseleditor.h"

#include "sql_defs.h"

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
  
public slots:
//  void on_bnStart_clicked();
 
//  gps::GPSParams fillVesselInitParams(QSqlQuery* q) const;
//  vsl:: setVesselData(QSqlQuery* q);
  gps::gpsInitParams getGPSInitParams(QSqlQuery* q, ais::aisDynamicData &dynamic_data);
  ais::aisStaticData getAISStaticData(QSqlQuery* q);
  ais::aisVoyageData getAISVoyageData(QSqlQuery* q);
  ais::aisDynamicData getAISDynamicData(QSqlQuery* q);
  
  void updateMapObjectInfo(SvMapObject* mapObject, const geo::GEOPOSITION& geopos);
  
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
  
  SvMapObjectSelfVessel* _self_map_obj;
  QMap<int, SvMapObjectSelfVessel*> _vessels_map_obj;
  
  States _current_state = sStopped;
  
public slots:
  void on_update_vessel_by_id(int id);
  
private slots:
  void stateChanged(States state);
  void on_bnCycle_clicked();
  
  void selectionChanged();
  
//  void initGeposition(gps::gpsInitParams& gpsParams, const ais::aisDynamicData& dynamicData);
  
  vsl::SvVessel* createSelfVessel(QSqlQuery* q);
  vsl::SvVessel* createOtherVessel(QSqlQuery* q);
  
  void on_actionNewVessel_triggered();
  
signals:
  void newState(States state);
  
  void startEmulation(quint32 multiplier);
  void stopEmulation();
  
};

#endif // MAINWINDOW_H
