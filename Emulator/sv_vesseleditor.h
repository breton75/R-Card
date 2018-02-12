#ifndef SV_VESSELEDITOR_H
#define SV_VESSELEDITOR_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>

#include "../../svlib/sv_sqlite.h"

#define SQL_SELECT_BEACON "select plan.id, plan.uid, plan.lon, plan.lat, plan.date_time, plan.description from plan where id=%1"

namespace Ui {
class SvVesselEditorDialog;
}

class SvVesselEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum ShowMode { smNew = 0, smEdit = 1 };
                  
  explicit SvVesselEditor(QWidget *parent, int vesselId = -1);

  ~SvVesselEditor();
  
  int showMode;
  
  int     t_id;
  QString t_uid = "";
  int t_model_id = -1; // модель может быть не выбрана
//  QString t_modelName = "";
//  QString t_className = "";
//  QString t_brandName = "";
  float   t_lon = 0;
  float   t_lat = 0;
  QDateTime t_date_time;
  QString t_description = "";
  
   
private slots:
//  void on_DEVMODELSLIST_UI_closed();
//  void on_DEVMODEL_UI_closed();
//  void on_ZONELIST_UI_closed();
//  void on_ZONE_UI_closed();
  
//  void on_bnSelectZone_clicked();
//  void on_bnNewZone_clicked();
//  void on_NEW_ZONE_UI_closed();
//  void on_bnSelectPosition_clicked();
  
public slots:
  void accept() Q_DECL_OVERRIDE;
//  void slotNewModel();
//  void slotSelectModel();
  
private:
  Ui::SvVesselEditorDialog *ui;
  
};

#endif // SV_VESSELEDITOR_H
