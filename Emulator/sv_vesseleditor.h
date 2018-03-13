#ifndef SV_VESSELEDITOR_H
#define SV_VESSELEDITOR_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QException>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"

//#define SQL_SELECT_BEACON "select plan.id, plan.uid, plan.lon, plan.lat, plan.date_time, plan.description from plan where id=%1"

namespace Ui {
class SvVesselEditorDialog;
}

class SvVesselEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum ShowMode { smNew = 0, smEdit = 1 };
  enum ResultCode { rcSqlError = -1, rcNoError };
                  
  explicit SvVesselEditor(QWidget *parent, int vesselId = -1, bool self = false);

  ~SvVesselEditor();
  
  int showMode;
  
  QString last_error() { return _last_error; }
  
  int     t_id = -1;
  bool    t_self = false;
  QString t_static_callsign = "";
  QString t_static_imo = "";
  QString t_static_mmsi = "";
  quint32 t_static_type_id;
  QString t_static_vessel_type_name = "";
  quint32 t_static_length = 1;
  quint32 t_static_width = 1;
  
  QString t_voyage_destination = "";
  qreal   t_voyage_draft = 1.0;
  quint32 t_voyage_cargo_type_id = 0;
  QString t_voyage_cargo_type_name = "";
  quint32 t_voyage_team = 1;
  
  quint32 t_gps_timeout = 1000;
  bool    t_init_random_coordinates = false;
  bool    t_init_random_course = false;
  bool    t_init_random_speed = false;
  quint32 t_init_course_change_ratio = 45;
  quint32 t_init_course_change_segment = 10;
  quint32 t_init_speed_change_ratio = 10;
  quint32 t_init_speed_change_segment = 10;
  
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
private:
  Ui::SvVesselEditorDialog *ui;
  
  QString _last_error = "";
  SvException _exception;
  
  void loadVesselTypes();
  void loadCargoTypes();
  void loadInitRandoms();
  
};

#endif // SV_VESSELEDITOR_H
