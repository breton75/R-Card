#include "sv_navtexeditor.h"
#include "ui_sv_navtexeditor.h"

SvNavtexEditor* NAVTEXEDITOR_UI;
extern SvSQLITE* SQLITE;

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
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_WHERE_ID).arg(navtexId), q).type()) {
      
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
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
  
}

SvNavtexEditor::~SvNavtexEditor()
{
  delete ui;
}

void SvNavtexEditor::loadMessages()
{
  ui->cbNAVTEXMessageType->clear();
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_MESSAGES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbNAVTEXMessageType->addItem(QString("%1 (%2)")
                                 .arg(q->value("letter_id").toString())
                                 .arg(q->value("designation").toString()),
                              q->value("id").toUInt());

  q->finish();
  delete q;
  
  if(ui->cbNAVTEXMessageType->count()) ui->cbNAVTEXMessageType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbNAVTEXMessageType->currentData().isNull());
}

void SvNavtexEditor::loadRegions()
{
  ui->cbNAVTEXStation->clear();
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_REGIONS), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbNAVTEXStation->addItem(QString("%1 (%2)")
                                 .arg(q->value("letter_id").toString())
                                 .arg(q->value("station_name").toString()),
                              q->value("id").toUInt());

  q->finish();
  delete q;
  
  if(ui->cbNAVTEXStation->count()) ui->cbNAVTEXStation->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbNAVTEXStation->currentData().isNull());
  
}

void SvNavtexEditor::accept()
{
  t_region_id = ui->cbNAVTEXStation->currentData().toUInt();
  t_message_id = ui->cbNAVTEXMessageType->currentData().toUInt();
  t_last_message = ui->textNAVTEXMessageText->toPlainText();
  
  switch (this->showMode) {
    
    case smNew: {
      
      try {
        qDebug() << QString(SQL_INSERT_NAVTEX)
                    .arg(t_region_id)
                    .arg(t_message_id)
                    .arg(t_last_message);
        
        QSqlError sql = SQLITE->execSQL(QString(SQL_INSERT_NAVTEX)
                                        .arg(t_region_id)
                                        .arg(t_message_id)
                                        .arg(t_last_message));
        
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
        
      }
      
      catch(SvException &e) {
          
        _last_error = e.err;
        qDebug() << 111 << _last_error;
        QDialog::reject();
        
        return;
      }
    }
      
    case smEdit: {
      try {
        
        
        QSqlError sql = SQLITE->execSQL(QString(SQL_UPDATE_NAVTEX)
                                        .arg(t_region_id)
                                        .arg(t_message_id)
                                        .arg(t_last_message)
                                        .arg(t_id));
        
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
        
      }
      
      
      catch(SvException &e) {
          
        _last_error = e.err;
        qDebug() << 222 << _last_error;
        QDialog::reject();
        
        return;
      }
    }
  }
  
  QDialog::accept();
  
}

void SvNavtexEditor::on_bnSimpleMessage_clicked()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_SIMPLE_MESSAGE)
                                           .arg(ui->cbNAVTEXMessageType->currentData().toUInt()), q).type()) {
    
    q->finish();
    return;
  }
  
  if(q->next())
    ui->textNAVTEXMessageText->setText(q->value("simple_message").toString());
  
  q->finish();
  delete q;
  
}
