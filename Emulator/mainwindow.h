#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>
#include <QThread>

#include "sv_area.h"
#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"
#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"
#include "sv_ais.h"

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
  gps::GPSParams getGPSData(QSqlQuery* q);
  ais::StaticData getAISStaticData(QSqlQuery* q);
  ais::VoyageData getAISVoyageData(QSqlQuery* q);
  ais::DynamicData getAISDynamicData(QSqlQuery* q);
  
//  QString fillVesselNavStatus(QSqlQuery* q) const;
  
  
private:
  Ui::MainWindow *ui;
  
  area::SvArea* _area;
  
};

#endif // MAINWINDOW_H
