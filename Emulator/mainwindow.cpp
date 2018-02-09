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
  if(QSqlError::NoError != SQLITE->execSQL(QString("select * from vessels where self=true"), q).type()) {
    qDebug() << q->lastError().databaseText();
    q->finish();
    return;
  }
  
  if(!q->next()) {
    q->finish();
    QMessageBox::critical(this, "Ошибка", "В БД нет сведений о собственном судне", QMessageBox::Ok);
    return;    
  }
  
  // создаем объект собственного судна
  
  // проверяем, что все значения проинициализированы. иначе назначаем свои значения
  vsl::InitParams iprms = fillVesselInitParams(q) ;
  vsl::VesselStaticData sdata = fillVesselStaticData(q);
  vsl::VesselVoyageData vdata = fillVesselVoyageData(q);
  geo::POSITION pos = fillVesselPosition(q);
  QString nav_status = fillVesselNavStatus(q);
  
  
//  iprms.gps_timeout = 
  
  SELF = new vsl::SvVessel(this);
  SELF->setInitParams(iprms);
  SELF->setStaticData(sdata);
  SELF->setVoyageData(vdata);
  SELF->setPosition(pos);
  SELF->setNavStatus(nav_status);
  
  
  /** ------ читаем список судов --------- **/
//  QSqlQuery* q = new QSqlQuery(SQLITE->db);
//  if(QSqlError::NoError != SQLITE->execSQL(QString("select * from vessels where self=false"), q).type()) {
    
//    qDebug() << q->lastError().databaseText();
//    q->finish();
//    return;
//  }
  
//  while(q->next())
//  {
//    int id = q->value("id").toInt();
//    qreal lon = q->value("lon").toDouble();
//    qreal lat = q->value("lat").toDouble();
//    QString uid = q->value("uid").toString();
  
//    SvMapObjectBeaconPlanned* beacon = new SvMapObjectBeaconPlanned(area);
//    beacon->setGeo(lon, lat);
//    beacon->setId(id);
//    beacon->setUid(uid);
//    beacon->setBrush(QColor(255, 50, 100, 255), QColor(255, 50, 100, 255));
//    beacon->setPen(QColor(255, 50, 100, 255).dark());
//    area->scene->addMapObject(beacon);   
    
//  }
//  q->finish();
  
  
}

MainWindow::~MainWindow()
{
  delete ui;
}

vsl::InitParams fillVesselInitParams(QSqlQuery* q)
{
  vsl::InitParams result;
  result.course = q->value("init_course").isNull() ? geo::get_rnd_course() : q->value("init_course").toUInt();
  result.course_change_ratio = q->value("init_course_change_ratio").isNull() ? 45 : q->value("init_course_change_ratio").toUInt();
  result.course_change_segment = q->value("init_course_change_segment").isNull() ? 1 : q->value("init_course_change_segment").toUInt();
  result.speed = q->value("init_speed").isNull() ? geo::get_rnd_course() : q->value("init_speed").toUInt();
  result.speed_change_ratio = q->value("init_speed_change_ratio").isNull() ? 10 : q->value("init_speed_change_ratio").toUInt();
  result.speed_change_segment = q->value("init_speed_change_segment").isNull() ? 1 : q->value("init_speed_change_segment").toUInt();
  
  return result;
}

vsl::VesselStaticData fillVesselStaticData(QSqlQuery* q)
{
  vsl::VesselStaticData result;
  result.callsign = q->value("callsign").toString();
  result.length = q->value("length").isNull() ? 20 : q->value("length").toUInt();
  result.width = q->value("width").isNull() ? 20 : q->value("width").toUInt();
  result.imo = q->value("imo").toString();
  result.mmsi = q->value("mmsi").toString();
  result.type = q->value("vessel_type_name").toString();
  
  return result;
}

vsl::VesselVoyageData fillVesselVoyageData(QSqlQuery* q)
{
  vsl::VesselVoyageData result;
  result.cargo = q->value("cargo_type_name").toString();
  result.destination = q->value("destination").toString();
  result.draft = q->value("draft").toUInt();
  result.team = q->value("draft").toUInt();\
  
  return result;
}

geo::POSITION fillVesselPosition(QSqlQuery* q)
{
  geo::POSITION result;
  geo::COORD coord;
  coord.latitude = q->value("dynamic_latitude").isNull() ? geo::get_rnd_position(_area->bounds()).latitude : q->value("dynamic_latitude").toUInt();
  coord.latitude = q->value("dynamic_longtitude").isNull() ? geo::get_rnd_position(_area->bounds()).longtitude : q->value("dynamic_longtitude").toUInt();
  
  quint32 course = q->value("dynamic_course").isNull() ? geo::get_rnd_course() : q->value("dynamic_course").toUInt();
  
  result.setCoordinates(coord);
  result.setCourse(course);
  
  return result;
}

QString fillVesselNavStatus(QSqlQuery* q)
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
