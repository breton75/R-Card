#include "mainwindow.h"
#include "ui_mainwindow.h"

extern SvSQLITE *SQLITE;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  
  QString map_file_name = AppParams::readParam(this, "General", "xml", "").toString();
  QString db_file_name = AppParams::readParam(this, "General", "db", "").toString();
  
  /** -------- создаем область отображения -------- **/
  _area = new area::SvArea(ui->dockWidget);
  ui->vlayDock->addWidget(_area);
  
  if(_area->readBounds(map_file_name)) 
    _area->readMap(map_file_name);
  
  else 
    QMessageBox::warning(this, tr("Ошибка в файле XML"), tr("Неверные границы области (bounds)"));

  _area->setUp("area_1");
  
  
  /** ---------- открываем БД ----------- **/
  SQLITE = new SvSQLITE(this, db_file_name);
  QSqlError err = SQLITE->connectToDB();
  
  if(err.type() != QSqlError::NoError) {
    qDebug() << err.databaseText();
    return;
  }
  
  /** ------ читаем список судов --------- **/
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString("select * from vessels"), q).type()) {
    
    qDebug() << q->lastError().databaseText();
    q->finish();
    return;
  }
  
  while(q->next())
  {
    qreal lon = q->value("lon").toDouble();
    qreal lat = q->value("lat").toDouble();
    int id = q->value("id").toInt();
    QString uid = q->value("uid").toString();
  
    SvMapObjectBeaconPlanned* beacon = new SvMapObjectBeaconPlanned(area);
    beacon->setGeo(lon, lat);
    beacon->setId(id);
    beacon->setUid(uid);
    beacon->setBrush(QColor(255, 50, 100, 255), QColor(255, 50, 100, 255));
    beacon->setPen(QColor(255, 50, 100, 255).dark());
    area->scene->addMapObject(beacon);   
    
  }
  q->finish();
  
  
}

MainWindow::~MainWindow()
{
  delete ui;
}

//void MainWindow::on_bnStart_clicked()
//{
//  /* начальный угол поворота */
//  QTime t(0,0,0);
//  qsrand(t.secsTo(QTime::currentTime()));
//  qreal start_angle = qrand() % 360;
  
  
  
//}
