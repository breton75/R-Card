#include "sv_vesseleditor.h"
#include "ui_sv_vesseleditor.h"

SvVesselEditor *VESSELEDITOR_UI;
extern SvSQLITE *SQLITE;

SvVesselEditor::SvVesselEditor(QWidget *parent, int vesselId) :
  QDialog(parent),
  ui(new Ui::SvVesselDialog)
{
  ui->setupUi(this);
  
  showMode = beaconId == -1 ? smNew : smEdit;

  t_lon = lon;
  t_lat = lat;
  t_date_time = QDateTime::currentDateTime();
  
  if(showMode == smEdit)
  {
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_BEACON).arg(beaconId), q).type())  
    {
      q->finish();
      delete q;
      return;
    }
   
    if(q->next())
    {
      t_id = beaconId;
      t_uid = q->value("uid").toString();
      t_date_time = QDateTime::fromString(q->value("date_time").toString(), "dd/MM/yyyy hh:mm:ss");
      t_lon = q->value("lon").toDouble();
      t_lat = q->value("lat").toDouble();
      t_description = q->value("description").toString();
    }
    
    q->finish();
    delete q;
    
  }
  
  ui->editID->setText(showMode == smNew ? "" : QString::number(t_id));
  ui->dateTimeEdit->setDateTime(t_date_time);
  ui->editUID->setText(t_uid);
  ui->editLon->setText(QString::number(t_lon));
  ui->editLat->setText(QString::number(t_lat));
  ui->textDescription->setText(t_description);
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
  
}

SvVesselEditor::~SvBeaconEditor()
{
  delete ui;
}

void SvVesselEditor::accept()
{
  t_uid = ui->editUID->text();
  t_description = ui->textDescription->toPlainText();
  
  
  bool result = false;
  switch (this->showMode)
  {
    case smNew:
    {
      QSqlError err = SQLITE->execSQL(QString("insert into plan (uid, lon, lat, date_time, description) values('%1', %2, %3, '%4', '%5')")
                                      .arg(t_uid)
                                      .arg(t_lon)
                                      .arg(t_lat)
                                      .arg(t_date_time.toString("dd/MM/yyyy hh:mm:ss"))
                                      .arg(t_description));
      if(QSqlError::NoError != err.type())
      {
        QMessageBox::critical(this, "Новый буй", err.databaseText(), QMessageBox::Ok);
        break;
      }
      
      QSqlQuery* q = new QSqlQuery(SQLITE->db);
      err = SQLITE->execSQL(QString("select id from plan order by id desc limit 1"), q);
      if(QSqlError::NoError != err.type())
      {
        q->finish();
        QMessageBox::critical(this, "Новый буй", err.databaseText(), QMessageBox::Ok);
        break;
      }

      q->first();
      t_id = q->value("id").toInt();
      q->finish();
      delete q;

      result = true;
      break;
      
    }
      
    case smEdit:
    {
      if(QSqlError::NoError != SQLITE->execSQL(QString("update plan set uid='%1', lon=%2, lat=%3, date_time='%4', description='%5' where id=%6")
                                             .arg(t_uid)
                                             .arg(t_lon)
                                             .arg(t_lat)
                                             .arg(t_date_time.toString("dd/MM/yyyy hh:mm:ss"))
                                             .arg(t_description)
                                             .arg(t_id)).type())
      {
        QMessageBox::critical(this, "Редактирование", "Не удалось обновить запись в журнале местных буев", QMessageBox::Ok);
        break;
      }      
      
      result = true;
      break;
    }
  }

  if(result) QDialog::accept();
  else QDialog::reject();
  
}
