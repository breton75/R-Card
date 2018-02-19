#include "sv_vesseleditor.h"
#include "ui_sv_vesseleditor.h"

SvVesselEditor *VESSELEDITOR_UI;
extern SvSQLITE *SQLITE;

SvVesselEditor::SvVesselEditor(QWidget *parent, int vesselId) :
  QDialog(parent),
  ui(new Ui::SvVesselEditorDialog)
{
  ui->setupUi(this);
  
  showMode = vesselId == -1 ? smNew : smEdit;
  
  loadCargoTypes();
  loadVesselTypes();
  loadInitRandoms();
  
  if(showMode == smEdit) {
    
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_ID).arg(vesselId), q).type()) {
      
      q->finish();
      delete q;
      return;
    }
   
    if(q->next()) {
      
      t_id = vesselId;
      
      t_self = q->value("self").toBool();            
      t_static_callsign = q->value("static_callsign").toString();
      t_static_imo = q->value("static_imo").toString();
      t_static_mmsi = q->value("static_mmsi").toString();
      t_static_type_id = q->value("static_type_id").toUInt();
      t_static_vessel_type_name = q->value("static_vessel_type_name").toString();
      t_static_length = q->value("static_length").toUInt();
      t_static_width = q->value("static_width").toUInt();
                                 
      t_voyage_destination = q->value("voyage_destination").toString();
      t_voyage_draft = q->value("voyage_draft").toUInt();
      t_voyage_cargo_type_id = q->value("voyage_cargo_type_id").toUInt();
      t_voyage_cargo_type_name = q->value("voyage_cargo_type_name").toString();
      t_voyage_team = q->value("voyage_team").toUInt();
                                 
      t_gps_timeout = q->value("gps_timeout").toUInt();
      t_init_random_coordinates = q->value("init_random_coordinates").toBool();
      t_init_random_course = q->value("init_random_course").toBool();
      t_init_random_speed = q->value("init_random_speed").toBool();
      t_init_course_change_ratio = q->value("init_course_change_ratio").toUInt();
      t_init_course_change_segment = q->value("init_course_change_segment").toUInt();
      t_init_speed_change_ratio = q->value("init_speed_change_ratio").toUInt();
      t_init_speed_change_segment = q->value("init_speed_change_segment").toUInt();
      
    }
    
    q->finish();
    delete q;
    
  }
  
  ui->ediId->setText(showMode == smNew ? "<Новый>" : QString::number(t_id));
//  ui-> set(t_self = false;
  ui->editCallsign->setText(t_static_callsign);
  ui->editIMO->setText(t_static_imo);
  ui->editMMSI->setText(t_static_mmsi);
  
  ui->cbVesselType->setCurrentIndex(ui->cbVesselType->findData(t_static_type_id));
  
  ui->spinLength->setValue(t_static_length);
  ui->spinWidth->setValue(t_static_width);
  
  ui->editDestination->setText(t_voyage_destination);
  ui->spinDraft->setValue(t_voyage_draft);
      
  ui->cbCargoType->setCurrentIndex(ui->cbCargoType->findData(t_voyage_cargo_type_id));
  
  ui->spinTeam->setValue(t_voyage_team);
  
  ui->spinGPSTimeout->setValue(t_gps_timeout);
  
  ui->cbInitCoordinates->setCurrentIndex(ui->cbInitCoordinates->findData(t_init_random_coordinates));
  ui->cbInitCourse->setCurrentIndex(ui->cbInitCourse->findData(t_init_random_course));
  ui->cbInitSpeed->setCurrentIndex(ui->cbInitSpeed->findData(t_init_random_speed));
  
  ui->spinCourseChangeSegment->setValue(t_init_course_change_ratio);
  ui->spinCourseChangeRatio->setValue(t_init_course_change_segment);
  ui->spinSpeedChangeRatio->setValue(t_init_speed_change_ratio);
  ui->spinSpeedChangeSegment->setValue(t_init_speed_change_segment);
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();

}

SvVesselEditor::~SvVesselEditor()
{
  delete ui;
}

void SvVesselEditor::loadVesselTypes()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_TYPES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbVesselType->addItem(q->value("type_name").toString(),
                              q->value("id").toInt());

  q->finish();
  delete q;
  
  if(ui->cbVesselType->count()) ui->cbVesselType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbVesselType->currentData().isNull());
  
}

void SvVesselEditor::loadCargoTypes()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_CARGO_TYPES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbCargoType->addItem(q->value("type_name").toString(),
                              q->value("id").toInt());

  q->finish();
  delete q;
  
  if(ui->cbCargoType->count()) ui->cbCargoType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbCargoType->currentData().isNull());
  
}

void SvVesselEditor::loadInitRandoms()
{
  ui->cbInitCoordinates->clear();
  ui->cbInitCoordinates->addItem("Случайно", QVariant(true));
  ui->cbInitCoordinates->addItem("Последние", QVariant(false));
  
  ui->cbInitCourse->clear();
  ui->cbInitCourse->addItem("Случайно", QVariant(true));
  ui->cbInitCourse->addItem("Последние", QVariant(false));
  
  ui->cbInitSpeed->clear();
  ui->cbInitSpeed->addItem("Случайно", QVariant(true));
  ui->cbInitSpeed->addItem("Последние", QVariant(false));
}

void SvVesselEditor::accept()
{
//  t_id = ui->ediId->text();
//  ui-> set(t_self = false;
  t_static_callsign = ui->editCallsign->text();
  t_static_imo = ui->editIMO->text();
  t_static_mmsi = ui->editMMSI->text();
  
  t_static_type_id = ui->cbVesselType->currentData().toUInt();
  
  t_static_length = ui->spinLength->value();
  t_static_width = ui->spinWidth->value();
  
  t_voyage_destination = ui->editDestination->text();
  t_voyage_draft = ui->spinDraft->value();
      
  t_voyage_cargo_type_id = ui->cbCargoType->currentData().toUInt();
  
  t_voyage_team = ui->spinTeam-> value();
  
  t_gps_timeout = ui->spinGPSTimeout->value();
  
  t_init_random_coordinates = ui->cbInitCoordinates->currentData().toBool();
  t_init_random_course = ui->cbInitCourse->currentData().toBool();
  t_init_random_speed = ui->cbInitSpeed->currentData().toBool();
  
  t_init_course_change_ratio = ui->spinCourseChangeSegment->value();
  t_init_course_change_segment = ui->spinCourseChangeRatio->value();
  t_init_speed_change_ratio = ui->spinSpeedChangeRatio->value();
  t_init_speed_change_segment = ui->spinSpeedChangeSegment->value();
  
  setResult(rcNoError);
  
//  bool result = false;
  switch (this->showMode) {
    
    case smNew: {
      
      QSqlError sql = SQLITE->execSQL(SQL_INSERT_NEW_VESSEL);
      
      if(QSqlError::NoError != sql.type()) {
        
        setResult(rcSqlError);
        _last_error = sql.databaseText();
        QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении записи:\n%1").arg(_last_error), QMessageBox::Ok);
        break;
      }
      
      QSqlQuery* q = new QSqlQuery(SQLITE->db);
      sql = SQLITE->execSQL(QString("select id from plan order by id desc limit 1"), q);
      
      if(QSqlError::NoError != sql.type()) {
        
        _last_error = sql.databaseText();
        q->finish();
        setResult(rcSqlError);
        
        QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении записи:\n%1").arg(_last_error), QMessageBox::Ok);
        break;
      }

      q->first();
//      t_id = q->value("id").toInt();
      q->finish();
      delete q;

//      result = true;
      break;
      
    }

    case smEdit: {
      /**
      QSqlError sql = SQLITE->execSQL(QString("update plan set uid='%1', lon=%2, lat=%3, date_time='%4', description='%5' where id=%6")
                                      .arg(t_uid)
                                      .arg(t_lon)
                                      .arg(t_lat)
                                      .arg(t_date_time.toString("dd/MM/yyyy hh:mm:ss"))
                                      .arg(t_description)
                                      .arg(t_id));
      
      if(QSqlError::NoError != sql.type()) {
        
        setResult(rcSqlError);
        _last_error = sql.databaseText();
        QMessageBox::critical(this, "Ошибка", QString("Не удалось обновить запись:\n%1").arg(_last_error), QMessageBox::Ok);
        break;
      } **/     
      
      
//      result = true;
      break;
    }
  }

    
  if(result() == rcNoError) QDialog::accept();
  else QDialog::reject();
  
}
