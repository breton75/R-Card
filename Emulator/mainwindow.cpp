#include "mainwindow.h"
#include "ui_mainwindow.h"

extern SvSQLITE *SQLITE;
extern vsl::SvVessel* SELF_VESSEL;
extern ais::SvAIS* SELF_AIS;
extern gps::SvGPS* SELF_GPS;
extern QMap<int, vsl::SvVessel*> VESSELS;
extern SvVesselEditor* VESSELEDITOR_UI;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  qRegisterMetaType<geo::GEOPOSITION>("geo::GEOPOSITION");
  
  ui->setupUi(this);
  
  AppParams::WindowParams p;
  p = AppParams::readWindowParams(this);
  resize(p.size);
  move(p.position);
  setWindowState(p.state);
  
  
  /* параметры окна графики */
  AppParams::WindowParams gw = AppParams::readWindowParams(this, "AREA WINDOW");
  ui->dockWidget->resize(gw.size);
  ui->dockWidget->move(gw.position);
//  ui->dockWidget->setWindowState(gw.state);
  
}

bool MainWindow::init()
{
  QString map_file_name = AppParams::readParam(this, "General", "xml", "").toString();
  QString db_file_name = AppParams::readParam(this, "General", "db", "").toString();
  
  /** ---------- открываем БД ----------- **/
  SQLITE = new SvSQLITE(this, db_file_name);
  QSqlError err = SQLITE->connectToDB();
  
  if(err.type() != QSqlError::NoError) {
    qDebug() << err.databaseText();
    return false;
  }
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  /*! необходима проверка, что в таблице vessels существует запись с флагом sef == true !*/
  /** ------ читаем информацию о собственном судне --------- **/
SV:
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_SELF).arg("true"), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    return false;
  }
  
  if(!q->next()) {
    QMessageBox::warning(this, "", "В БД нет сведений о собственном судне", QMessageBox::Ok);
    q->finish();
    
    VESSELEDITOR_UI = new SvVesselEditor(this);
    if(QDialog::Accepted == VESSELEDITOR_UI->exec()) {
      
      goto SV;    
            
    }
    else {
      
      qDebug() << VESSELEDITOR_UI->last_error();
      delete VESSELEDITOR_UI;
//      deleteLater();
//      delete ui;
//      deleteLater();
//      qApp->quit();
      return false;
    }
      
    delete VESSELEDITOR_UI;
    
  }
  
// читаем информацию из БД  
  int vessel_id = q->value("id").toUInt();
  
  gps::gpsInitParams gps_params = getGPSInitParams(q);
  
  ais::aisStaticData sdata = getAISStaticData(q); 
  ais::aisVoyageData vdata = getAISVoyageData(q);
  ais::aisDynamicData ddata = getAISDynamicData(q);
  
  q->finish();
 
  
  
  /** -------- создаем область отображения -------- **/
  _area = new area::SvArea(ui->dockWidget);
  ui->vlayDock->addWidget(_area);
  
  if(_area->readBounds(map_file_name)) 
    _area->readMap(map_file_name);
  
  else 
    QMessageBox::warning(this, tr("Ошибка в файле XML"), tr("Неверные границы области (bounds)"));

  _area->setUp("area_1");
  
  
  /** --------- создаем устройства ----------- **/
  
  // АИС
  SELF_AIS = new ais::SvAIS(vessel_id);
  SELF_AIS->setStaticData(sdata);
  SELF_AIS->setVoyageData(vdata);
  SELF_AIS->setDynamicData(ddata);
  SELF_AIS->open();
  
  
  // LAG
  
  
  
  // GPS
  // начальные координаты
  if(gps_params.init_random_coordinates || 
     (!gps_params.init_random_coordinates && !ddata.geoposition.isValidCoordinates()))
    
    gps_params.geoposition = geo::get_rnd_position(_area->bounds());
  
  else gps_params.geoposition = ddata.geoposition; 
  
  // начальный курс и скорость 
  if(gps_params.init_random_course) gps_params.geoposition.course = geo::get_rnd_course();
  if(gps_params.init_random_speed) gps_params.geoposition.speed = geo::get_rnd_speed();
  
  SELF_GPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  SELF_GPS->open();
 
  
  // создаем объект собственного судна
  SELF_VESSEL = new vsl::SvVessel(this, vessel_id, true) ;
  
  SELF_VESSEL->mountAIS(SELF_AIS);
//  SELF_VESSEL->mountLAG(SELF_LAG);
  SELF_VESSEL->mountGPS(SELF_GPS);
  
  SELF_VESSEL->assignMapObject(new SvMapObjectSelfVessel(_area));
  _area->scene->addMapObject(SELF_VESSEL->mapObject());
  SELF_VESSEL->mapObject()->setVisible(true);
  SELF_VESSEL->mapObject()->setZValue(1);
  
  // подключаем gps к АИС
  connect(SELF_GPS->emitter(), SIGNAL(newGeoPosition(geo::GEOPOSITION&)), SELF_AIS, SLOT(newSelfGeoPosition(geo::GEOPOSITION&)));
  
  connect(SELF_AIS, &ais::SvAIS::updateVessel, SELF_VESSEL, &vsl::SvVessel::updateVessel);
  
  connect(SELF_VESSEL, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  
//  SELF_GPS->start();
  
//  SELF_VESSEL->newGeoPosition(SELF_GPS->currentGeoPosition());
  
  
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
  
  connect(this, SIGNAL(newState(bool)), this, SLOT(stateChanged(bool)));
  
  return true;
}

MainWindow::~MainWindow()
{
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveWindowParams(ui->dockWidget, ui->dockWidget->size(), ui->dockWidget->pos(), ui->dockWidget->windowState(), "AREA WINDOW");
  
  delete ui;
}

gps::gpsInitParams MainWindow::getGPSInitParams(QSqlQuery* q)
{
  gps::gpsInitParams result;
  result.gps_timeout = q->value("gps_timeout").toUInt();
  result.init_random_coordinates = q->value("init_random_coordinates").toBool();
  result.init_random_course = q->value("init_random_course").toBool();
  result.init_random_speed = q->value("init_random_speed").toBool();
  result.course_change_ratio = q->value("init_course_change_ratio").toUInt();
  result.course_change_segment = q->value("init_course_change_segment").toUInt();
  result.speed_change_ratio = q->value("init_speed_change_ratio").toUInt();
  result.speed_change_segment = q->value("init_speed_change_segment").toUInt();
  
  return result;
}

ais::aisStaticData  MainWindow::getAISStaticData(QSqlQuery* q)
{
  ais::aisStaticData result;
  result.id = q->value("id").toUInt();
  result.mmsi = q->value("static_mmsi").toString();
  result.imo = q->value("static_imo").toString();
  result.callsign = q->value("static_callsign").toString();
  result.length = q->value("static_length").isNull() ? 20 : q->value("length").toUInt();
  result.width = q->value("static_width").isNull() ? 20 : q->value("width").toUInt();
  result.type = q->value("static_vessel_type_name").toString();
  
  return result;
}

ais::aisVoyageData MainWindow::getAISVoyageData(QSqlQuery* q)
{
  ais::aisVoyageData result;
  result.cargo = q->value("voyage_cargo_type_name").toString();
  result.destination = q->value("voyage_destination").toString();
  result.draft = q->value("voyage_draft").toUInt();
  result.team = q->value("voyage_team").toUInt();
  
  return result;
}

ais::aisDynamicData MainWindow::getAISDynamicData(QSqlQuery* q)
{
  ais::aisDynamicData result;
  result.geoposition.latitude = q->value("dynamic_latitude").toUInt();
  result.geoposition.longtitude = q->value("dynamic_longtitude").toUInt();
  result.geoposition.course = q->value("dynamic_course").toUInt();
  result.navstat = q->value("dynamic_status_name").toString();
  
  return result;
}

void MainWindow::stateChanged(bool state)
{
  if(state) {
    ui->bnCycle->setText("Стоп");
    ui->bnCycle->setStyleSheet("background-color: tomato");
   
  }
  else {
   
    ui->bnCycle->setText("Старт");
    ui->bnCycle->setStyleSheet("");
    
//    on_bnSaveToFile_clicked(false);
  }
  
  ui->tabWidget->setEnabled(!state);
  ui->bnCycle->setEnabled(true);
  QApplication::processEvents();
  
}

void MainWindow::on_bnCycle_clicked()
{
  
  ui->bnCycle->setEnabled(false);
  ui->tabWidget->setEnabled(false);
  QApplication::processEvents();
  
//  SELF_GPS->start();
//  emit newState(true);
  
  if(SELF_GPS) {
    
    SELF_GPS->start();
    
//    _serial = new QSerialPort(_available_devices.at(ui->cbDevices->currentIndex()));
    
//    if (!_serial)
//    {
//      log << svlog::Time << svlog::Critical << "Ошибка при создании устройства." 
//          << _serial->errorString() << svlog::endl;;
//      return;
//    }
    
//    if(!_serial->open(QIODevice::ReadWrite)) {
//      log << svlog::Time << svlog::Critical
//          << QString("Не удалось открыть порт %1").arg(_available_devices.at(ui->cbDevices->currentIndex()).portName())
//          << svlog::endl;
//      return;
//    }
    
//    _tdc100thr = new SvPullTDC1000(_serial, ui->spinTimer->value());
//    connect(_tdc100thr, SIGNAL(newData(QByteArray&)), this, SLOT(new_data(QByteArray&)));
//    _tdc100thr->start();
    
    emit newState(true);    

  }
  else
  {
    QMessageBox::critical(this, "Ошибка", "Критическая ошибка. Сообщите разработчику данную информацию:\n(on_bnCycle_clicked SELF_GPS = NULL).", QMessageBox::Ok);
    emit newState(false);
  }
//    SELF_GPS->stop();
    
//    _tdc100thr->stop();
//    _tdc100thr->deleteLater();
//    delete _tdc100thr;
//    _tdc100thr = nullptr;      
    
//    if(_serial) {
//      if(_serial->isOpen()) _serial->close();
//      delete _serial;
//      _serial = nullptr;
//    }
    
//    emit newState(false);
    
//  }
}
