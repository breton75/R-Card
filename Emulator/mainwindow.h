#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>

#include "sv_area.h"
#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"
#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"

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
  vsl::VesselStaticData fillVesselStaticData(QSqlQuery* q) const;
  vsl::VesselVoyageData fillVesselVoyageData(QSqlQuery* q) const;
  geo::POSITION fillVesselPosition(QSqlQuery* q) const;
  QString fillVesselNavStatus(QSqlQuery* q) const;
  
  
private:
  Ui::MainWindow *ui;
  
  area::SvArea* _area;
  
};

#endif // MAINWINDOW_H
