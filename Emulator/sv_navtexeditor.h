#ifndef SV_NAVTEXEDITOR_H
#define SV_NAVTEXEDITOR_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QException>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"

namespace Ui {
class SvNavtexEditorDialog;
}

class SvNavtexEditor : public QDialog
{
  Q_OBJECT
  
  enum ShowMode { smNew = 0, smEdit = 1 };
  
public:
  explicit SvNavtexEditor(QWidget *parent, int navtexId);
  ~SvNavtexEditor();
  
  int showMode;
  
  QString last_error() { return _last_error; }
  
  int     t_id = -1;
  bool    t_isactive = true;
  int     t_message_id = 1;
  int     t_region_id = 1;
  QString t_last_message = "";
  
private:
  Ui::SvNavtexEditorDialog *ui;
  
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
  QString _last_error = "";
  SvException _exception;
  
  void loadMessages();
  void loadRegions(); 
  
};

#endif // SV_NAVTEXEDITOR_H
