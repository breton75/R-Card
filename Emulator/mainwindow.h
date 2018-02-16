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
  
public slots:
//  void on_bnStart_clicked();
 
//  gps::GPSParams fillVesselInitParams(QSqlQuery* q) const;
//  vsl:: setVesselData(QSqlQuery* q);
  gps::gpsInitParams getGPSInitParams(QSqlQuery* q);
  ais::aisStaticData getAISStaticData(QSqlQuery* q);
  ais::aisVoyageData getAISVoyageData(QSqlQuery* q);
  ais::aisDynamicData getAISDynamicData(QSqlQuery* q);
  
//  QString fillVesselNavStatus(QSqlQuery* q) const;
  
  
private:
  Ui::MainWindow *ui;
  
  area::SvArea* _area;
  
  SvMapObjectSelfVessel* _self_map_obj;
  QMap<int, SvMapObjectSelfVessel*> _vessels_map_obj;
  
private slots:
  void stateChanged(bool state);
//  void setVesselPosition();
  
  
  void on_bnCycle_clicked();
  
signals:
  newState(bool state);
  
};

#endif // MAINWINDOW_H
