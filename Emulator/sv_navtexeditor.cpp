#include "sv_navtexeditor.h"
#include "ui_sv_navtexeditor.h"

SvNavtexEditor* NAVTEXEDITOR_UI;
 
SvNavtexEditor::SvNavtexEditor(QWidget *parent, int navtexId) :
  QDialog(parent),
  ui(new Ui::SvNavtexEditorDialog)
{
  ui->setupUi(this);
  
  showMode = navtexId == -1 ? smNew : smEdit;
  
  loadMessages();
  loadRegions();
  
  if(showMode == smEdit) {
    
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_ID).arg(vesselId), q).type()) {
      
      q->finish();
      delete q;
      return;
    }
   
    if(q->next()) {
      
      t_id = q->value("id").toUInt();
      t_region_id = q->value("station_region_id").toUInt();
      t_message_id = q->value("station_message_id").toUInt();
      t_last_message = q->value("last_message").toString();
      t_isactive = q->value("is_active").toBool();
      
    }
    
    q->finish();
    delete q;
    
  }
 
  ui->cbNAVTEXMessageType->setCurrentIndex(ui->cbNAVTEXMessageType->findData(t_message_id));
  ui->cbNAVTEXStation->setCurrentIndex(ui->cbNAVTEXStation->findData(t_region_id));
  ui->textNAVTEXMessageText->setText(t_last_message);
  
}

SvNavtexEditor::~SvNavtexEditor()
{
  delete ui;
}

void SvNavtexEditor::loadMessages()
{
  
}

void SvNavtexEditor::loadRegions()
{
  
}

void SvNavtexEditor::accept()
{
  
  
  switch (this->showMode) {
    
    case smNew: {
      
      try {
        
      }
      
      catch(SvException &e) {
          
        _last_error = e.err;
//        qDebug() << _last_error;
        QDialog::reject();
        
        return;
      }
    }
      
    case smEdit: {
      try {
        
      }
      
      
      catch(SvException &e) {
          
        _last_error = e.err;
//        qDebug() << _last_error;
        QDialog::reject();
        
        return;
      }
    }
  }
  
  QDialog::accept();
  
}
