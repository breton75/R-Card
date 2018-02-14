#include "mainwindow.h"
#include "ui_mainwindow.h"

extern SvSQLITE *SQLITE;
extern vsl::SvVessel* SELF_VESSEL;
extern ais::SvAIS* SELF_AIS;
extern gps::SvGPS* SELF_GPS;
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
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS).arg("true"), q).type()) {
    
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
  int vessel_id = q->value("id").toUInt();
  
  gps::GPSParams gps_params = getGPSData(q);
  
  ais::StaticData sdata = getAISStaticData(q);
  ais::VoyageData vdata = getAISVoyageData(q);
  ais::DynamicData ddata = getAISDynamicData(q);
  
  q->finish();
 
  // создаем устройство АИС
  SELF_AIS = new ais::SvAIS(vessel_id);
  SELF_AIS->setStaticData(sdata);
  SELF_AIS->setVoyageData(vdata);
  SELF_AIS->setDynamicData(ddata);

  // создаем устройство LAG
  
  
  // создаем устройство GPS
  SELF_GPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
//  connect(SELF_GPS, &QThread::finished, SELF_GPS, &gps::SvGPS::deleteLater);
  connect(SELF_GPS, SIGNAL(new_location(geo::LOCATION)), SELF_AIS, SLOT(new_location(geo::LOCATION)));
  
 
  
  // создаем объект собственного судна
  SELF_VESSEL = new vsl::SvVessel(this, vessel_id, true) ;
  
  SELF_VESSEL->mountAIS(SELF_AIS);
//  SELF_VESSEL->mountLAG(SELF_LAG);
  SELF_VESSEL->mountGPS(SELF_GPS);
  
  SELF_VESSEL->assignMapObject(new SvMapObjectSelfVessel(_area, SELF_VESSEL));
  _area->scene->addMapObject(SELF_VESSEL->mapObject());
  SELF_VESSEL->mapObject()->setVisible(true);
  SELF_VESSEL->mapObject()->setZValue(1);
  connect(SELF_GPS, SIGNAL(new_location(geo::LOCATION)), SELF_VESSEL, SLOT(new_location(geo::LOCATION)));
  
  
  /** ------ читаем список судов --------- **/
  /**
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
  **/
  

  
}

MainWindow::~MainWindow()
{
  delete ui;
}

gps::GPSParams MainWindow::getGPSData(QSqlQuery* q)
{
  gps::GPSParams result;
  result.course = q->value("init_course").isNull() ? geo::get_rnd_course() : q->value("init_course").toUInt();
  result.course_change_ratio = q->value("init_course_change_ratio").isNull() ? 45 : q->value("init_course_change_ratio").toUInt();
  result.course_change_segment = q->value("init_course_change_segment").isNull() ? 1 : q->value("init_course_change_segment").toUInt();
  result.speed = q->value("init_speed").isNull() ? geo::get_rnd_course() : q->value("init_speed").toUInt();
  result.speed_change_ratio = q->value("init_speed_change_ratio").isNull() ? 10 : q->value("init_speed_change_ratio").toUInt();
  result.speed_change_segment = q->value("init_speed_change_segment").isNull() ? 1 : q->value("init_speed_change_segment").toUInt();
  
  return result;
}

ais::StaticData  MainWindow::getAISStaticData(QSqlQuery* q)
{
  ais::StaticData result;
  result.id = q->value("id").toUInt();
  result.mmsi = q->value("static_mmsi").toString();
  result.imo = q->value("static_imo").toString();
  result.callsign = q->value("static_callsign").toString();
  result.length = q->value("static_length").isNull() ? 20 : q->value("length").toUInt();
  result.width = q->value("static_width").isNull() ? 20 : q->value("width").toUInt();
  result.type = q->value("static_vessel_type_name").toString();
  
  return result;
}

ais::VoyageData MainWindow::getAISVoyageData(QSqlQuery* q)
{
  ais::VoyageData result;
  result.cargo = q->value("voyage_cargo_type_name").toString();
  result.destination = q->value("voyage_destination").toString();
  result.draft = q->value("voyage_draft").toUInt();
  result.team = q->value("voyage_team").toUInt();
  
  return result;
}

ais::DynamicData MainWindow::getAISDynamicData(QSqlQuery* q)
{
  ais::DynamicData result;
  result.position.coord.latitude = q->value("dynamic_latitude").toUInt();
  result.position.coord.longtitude = q->value("dynamic_longtitude").toUInt();
  result.position.course = q->value("dynamic_course").toUInt();
  result.navstat = q->value("dynamic_status_name").toString();
  
  return result;
}

//QString MainWindow::fillVesselNavStatus(QSqlQuery* q) const
//{
//  QString result = q->value("status_name").toString();
  
//  return result;  
//}

//void MainWindow::on_bnStart_clicked()
//{
//  /* начальный угол поворота */
//  QTime t(0,0,0);
//  qsrand(t.secsTo(QTime::currentTime()));
//  qreal start_angle = qrand() % 360;
  
  
  
//}
