#include "mainwindow.h"
#include "ui_mainwindow.h"

extern SvSQLITE *SQLITE;
extern vsl::SvVessel* SELF;
extern QMap<int, vsl::SvVessel*> VESSELS;

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
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  /*! необходима проверка, что в таблице vessels существует запись с флагом sef == true !*/
  /** ------ читаем информацию о собственном судне --------- **/
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELCT_VESSELS).arg("true"), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    return;
  }
  
  if(!q->next()) {
    QMessageBox::critical(this, "Ошибка", "В БД нет сведений о собственном судне", QMessageBox::Ok);
    q->finish();
    return;    
  }
  
// читаем информацию из БД  
//  gps::Params iprms = fillVesselInitParams(q) ;
  vsl::VesselStaticData sdata = fillVesselStaticData(q);
  vsl::VesselVoyageData vdata = fillVesselVoyageData(q);
  geo::POSITION pos = fillVesselPosition(q);
//  QString navstat = fillVesselNavStatus(q);

  q->finish();
  
  // создаем объект собственного судна
  SELF = new vsl::SvVessel(this);
//  SELF->setInitParams(iprms);
  SELF->setStaticData(sdata);
  SELF->setVoyageData(vdata);
  SELF->setPosition(pos);
//  SELF->setNavStatus(navstat);
  
  
  /** ------ читаем список судов --------- **/
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELCT_VESSELS).arg("false"), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    return;
  }
  
  while(q->next()) {
    
    // читаем информацию из БД  
//      vsl::InitParams iprms = fillVesselInitParams(q) ;
      vsl::VesselStaticData sdata = fillVesselStaticData(q);
      vsl::VesselVoyageData vdata = fillVesselVoyageData(q);
      geo::POSITION pos = fillVesselPosition(q);
//      QString navstat = fillVesselNavStatus(q);
    
      q->finish();
      
      // создаем объект собственного судна
      vsl::SvVessel* v = new vsl::SvVessel(this);
//      v->setInitParams(iprms);
      v->setStaticData(sdata);
      v->setVoyageData(vdata);
      v->setPosition(pos);
//      v->setNavStatus(navstat);
      
      VESSELS.insert(sdata.id, v);
    
  }
  q->finish();
  
  
}

MainWindow::~MainWindow()
{
  delete ui;
}

//vsl::InitParams MainWindow::fillVesselInitParams(QSqlQuery* q) const
//{
//  vsl::InitParams result;
//  result.course = q->value("init_course").isNull() ? geo::get_rnd_course() : q->value("init_course").toUInt();
//  result.course_change_ratio = q->value("init_course_change_ratio").isNull() ? 45 : q->value("init_course_change_ratio").toUInt();
//  result.course_change_segment = q->value("init_course_change_segment").isNull() ? 1 : q->value("init_course_change_segment").toUInt();
//  result.speed = q->value("init_speed").isNull() ? geo::get_rnd_course() : q->value("init_speed").toUInt();
//  result.speed_change_ratio = q->value("init_speed_change_ratio").isNull() ? 10 : q->value("init_speed_change_ratio").toUInt();
//  result.speed_change_segment = q->value("init_speed_change_segment").isNull() ? 1 : q->value("init_speed_change_segment").toUInt();
  
//  return result;
//}

vsl::VesselStaticData MainWindow::fillVesselStaticData(QSqlQuery* q) const
{
  vsl::VesselStaticData result;
  result.id = q->value("id").toUInt();
  result.callsign = q->value("callsign").toString();
  result.length = q->value("length").isNull() ? 20 : q->value("length").toUInt();
  result.width = q->value("width").isNull() ? 20 : q->value("width").toUInt();
  result.imo = q->value("imo").toString();
  result.mmsi = q->value("mmsi").toString();
  result.type = q->value("vessel_type_name").toString();
  
  return result;
}

vsl::VesselVoyageData MainWindow::fillVesselVoyageData(QSqlQuery* q) const
{
  vsl::VesselVoyageData result;
  result.cargo = q->value("cargo_type_name").toString();
  result.destination = q->value("destination").toString();
  result.draft = q->value("draft").toUInt();
  result.team = q->value("draft").toUInt();\
  
  return result;
}

geo::POSITION MainWindow::fillVesselPosition(QSqlQuery* q) const
{
  geo::POSITION result;
  qreal lat = q->value("dynamic_latitude").isNull() ? geo::get_rnd_position(_area->bounds()).latitude : q->value("dynamic_latitude").toUInt();
  qreal lon = q->value("dynamic_longtitude").isNull() ? geo::get_rnd_position(_area->bounds()).longtitude : q->value("dynamic_longtitude").toUInt();
//  coord.latitude = 
//  coord.latitude = 
  geo::COORD coord(lat, lon); 
  
  quint32 course = q->value("dynamic_course").isNull() ? geo::get_rnd_course() : q->value("dynamic_course").toUInt();
  
  result.coord = coord;
  result.course = course;
  
  result.navstat = q->value("status_name").toString();
  
  return result;
}

QString MainWindow::fillVesselNavStatus(QSqlQuery* q) const
{
  QString result = q->value("status_name").toString();
  
  return result;  
}

//void MainWindow::on_bnStart_clicked()
//{
//  /* начальный угол поворота */
//  QTime t(0,0,0);
//  qsrand(t.secsTo(QTime::currentTime()));
//  qreal start_angle = qrand() % 360;
  
  
  
//}
