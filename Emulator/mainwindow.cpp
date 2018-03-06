#include "mainwindow.h"
#include "ui_mainwindow.h"

extern SvSQLITE *SQLITE;

//vsl::SvVessel* SELF_VESSEL;

QMap<int, ais::SvAIS*> AISs;
QMap<int, gps::SvGPS*> GPSs;
QMap<int, vsl::SvVessel*> VESSELs;

//extern ais::SvAIS* SELF_AIS;
//extern gps::SvGPS* SELF_GPS;
//extern QMap<int, vsl::SvVessel*> VESSELS;
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
  
  ui->dspinAISRadius->setValue(AppParams::readParam(this, "GENERAL", "AISRadius", 2).toReal());
  
}

bool MainWindow::init()
{
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
    
    delete _area;
    return false;
    
  }
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  /*! необходима проверка, что в таблице vessels существует запись с флагом sef == true !*/
SV:

  /** ------ читаем информацию о собственном судне --------- **/
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(true), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    return false;
  }
  
  if(!q->next()) {
    
    QMessageBox::warning(this, "", "В БД нет сведений о собственном судне", QMessageBox::Ok);
    q->finish();
    
    VESSELEDITOR_UI = new SvVesselEditor(this, -1, true);
    if(QDialog::Accepted == VESSELEDITOR_UI->exec()) {
      
      goto SV;    
            
    }
    else {
      
      if(VESSELEDITOR_UI->result() != SvVesselEditor::rcNoError)
        QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении записи:\n%1").arg(VESSELEDITOR_UI->last_error()), QMessageBox::Ok);
      
      delete VESSELEDITOR_UI;
      delete q;
      delete _area;
      
      return false;
      
    }
      
    delete VESSELEDITOR_UI;
    
  }

  
  
  
  /** --------- создаем собственное судно ----------- **/
  createSelfVessel(q);
  q->finish();
  
//  _self_vessel = createSelfVessel(q); //self_vessel_id, self_gps_params, self_static_data, self_voyage_data, self_dynamic_data, true);
//  _self_ais = _self_vessel->ais();
  _self_vessel->updateVessel();

  
 
  
  /** ------ читаем список судов --------- **/
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(false), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    
    delete q;
    delete _self_vessel;
    delete _area;
    
    return false;
  }
  
  /** --------- создаем суда ----------- **/
  while(q->next()) {
    vsl::SvVessel* vessel = createOtherVessel(q);
    vessel->updateVessel();
    
  }
  q->finish();

  
  
  connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  
  connect(this, SIGNAL(newState(States)), this, SLOT(stateChanged(States)));
  
  return true;
}

MainWindow::~MainWindow()
{
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveWindowParams(ui->dockWidget, ui->dockWidget->size(), ui->dockWidget->pos(), ui->dockWidget->windowState(), "AREA WINDOW");
  
  AppParams::saveParam(this, "GENERAL", "AISRadius", QVariant(ui->dspinAISRadius->value()));
  
  delete ui;
}

gps::gpsInitParams MainWindow::getGPSInitParams(QSqlQuery* q, ais::aisDynamicData& dynamic_data)
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
  
  // начальные координаты
  if(result.init_random_coordinates || 
     (!result.init_random_coordinates && !dynamic_data.geoposition.isValidCoordinates())) {
    
    geo::COORDINATES coord = geo::get_rnd_coordinates(_area->bounds());
    
    result.geoposition.latitude = coord.latitude;
    result.geoposition.longtitude = coord.longtitude;
    dynamic_data.geoposition.latitude = coord.latitude;
    dynamic_data.geoposition.longtitude = coord.longtitude;
    
  }
  else {
    
    result.geoposition.latitude = dynamic_data.geoposition.latitude; 
    result.geoposition.longtitude = dynamic_data.geoposition.longtitude; 
    
  }
  
  // начальный курс
  if(result.init_random_course ||
    (!result.init_random_course && !dynamic_data.geoposition.isValidCourse())) {
    
    result.geoposition.course = geo::get_rnd_course();
    dynamic_data.geoposition.course = result.geoposition.course;
    
  }
  else result.geoposition.course = dynamic_data.geoposition.course;
  
  // начальная скорость 
  if(result.init_random_speed ||
    (!result.init_random_speed && !dynamic_data.geoposition.isValidSpeed())) {
    
    result.geoposition.speed = geo::get_rnd_speed();
    dynamic_data.geoposition.speed = result.geoposition.speed;
    
  }
  else result.geoposition.speed = dynamic_data.geoposition.speed;
  
  return result;
}

ais::aisStaticData  MainWindow::getAISStaticData(QSqlQuery* q)
{
  ais::aisStaticData result;
  result.id = q->value("id").toUInt();
  result.mmsi = q->value("static_mmsi").toString();
  result.imo = q->value("static_imo").toString();
  result.callsign = q->value("static_callsign").toString();
  result.length = q->value("static_length").isNull() ? 20 : q->value("static_length").toUInt();
  result.width = q->value("static_width").isNull() ? 20 : q->value("static_width").toUInt();
  result.type = q->value("static_vessel_type_name").toString();
  
  return result;
}

ais::aisVoyageData MainWindow::getAISVoyageData(QSqlQuery* q)
{
  ais::aisVoyageData result;
  result.cargo = q->value("voyage_cargo_type_name").toString();
  result.destination = q->value("voyage_destination").toString();
  result.draft = q->value("voyage_draft").toReal();
  result.team = q->value("voyage_team").toUInt();
  
  return result;
}

ais::aisDynamicData MainWindow::getAISDynamicData(QSqlQuery* q)
{
  ais::aisDynamicData result;
  result.geoposition.latitude = q->value("dynamic_latitude").isNull() ? -1.0 : q->value("dynamic_latitude").toReal();
  result.geoposition.longtitude = q->value("dynamic_longtitude").isNull() ? -1.0 : q->value("dynamic_longtitude").toReal();
  result.geoposition.course = q->value("dynamic_course").isNull() ? -1 : q->value("dynamic_course").toInt();
  result.geoposition.speed = q->value("dynamic_speed").isNull() ? -1 : q->value("dynamic_speed").toReal();
  result.navstat = q->value("dynamic_status_name").toString();
  
  return result;
}

void MainWindow::stateChanged(States state)
{
  switch (state) {
    
    case sRunned:
    {
      ui->tabWidget->setEnabled(false);
      ui->bnCycle->setEnabled(true);
      ui->bnCycle->setText("Стоп");
      ui->bnCycle->setStyleSheet("background-color: tomato");
      
      break;
    }
      
    case sStopped:
    {
      ui->tabWidget->setEnabled(true);
      ui->bnCycle->setEnabled(true);
      ui->bnCycle->setText("Старт");
      ui->bnCycle->setStyleSheet("");
      break;
    }
      
    case sRunning:
    case sStopping:
    {
      ui->bnCycle->setEnabled(false);
      ui->tabWidget->setEnabled(false);
    }
      
    default:
      break;
  }
  
  _current_state = state;
  QApplication::processEvents();
  
}

void MainWindow::on_bnCycle_clicked()
{
  
  switch (_current_state) {
    
    case sStopped:
    {
      emit newState(sRunning);
      
      emit startEmulation(ui->spinEmulationMultiplier->value());
      
//      foreach (gps::SvGPS* gps, GPSs.values()) {
        
//        gps->start(ui->spinEmulationMultiplier->value());
//      }
      
      emit newState(sRunned);
      break;
    }
      
    case sRunned:
    {
      emit newState(sStopping);
      
      emit stopEmulation();
      
      foreach (gps::SvGPS* gps, GPSs.values()) {
        gps->waitWhileRunned();
//        gps->stop();
      }
      
      emit newState(sStopped);
      break;
    }
      
    default:
      break;
  }
  
}

//void MainWindow::onFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason)
void MainWindow::selectionChanged()
{
  if(_area->scene->selectedItems().isEmpty()) {
   
    // ищем все выделения и удаляем их
    foreach (SvMapObject* item, _area->scene->mapObjects()) {
      
      if(item->selection()) {  
        _area->scene->removeItem(item->selection());
        item->deleteSelection();
      }
    }
    
    _area->setLabelInfo("");
    return;
  }
  
  SvMapObject* mo = (SvMapObject*)(_area->scene->selectedItems().first());
  
  mo->setSelection(new SvMapObjectSelection(_area, mo));
  _area->scene->addMapObject(mo->selection());
  mo->selection()->setVisible(true);
  _area->scene->setMapObjectPos(mo, mo->geoPosition());

  updateMapObjectInfo(mo, mo->geoPosition());
  
}

void MainWindow::updateMapObjectInfo(SvMapObject* mapObject, const geo::GEOPOSITION& geopos)
{
  if(!mapObject->isSelected())
    return;
  
  switch (mapObject->type()) {
    
    case motSelfVessel:
    case motVessel:
      
      if(VESSELs.find(mapObject->id()) != VESSELs.end()) {
      
        vsl::SvVessel* vsl = VESSELs.value(mapObject->id());
        
        _area->setLabelInfo(QString("Текущий объект: %1  шир.: %2  долг.: %3  курс: %4%5  скорость: %6 км/ч  статус: %7")
                            .arg(vsl->ais()->getStaticData()->callsign)
                            .arg(geopos.latitude, 0, 'g', 4)
                            .arg(geopos.longtitude, 0, 'g', 4)
                            .arg(geopos.course)
                            .arg(QChar(248))
                            .arg(geopos.speed, 0, 'g', 2)
                            .arg(vsl->ais()->getDynamicData()->navstat)); 
            
        
      }
        
      break;
        
      
    default:
      break;
  }
}

vsl::SvVessel *MainWindow::createSelfVessel(QSqlQuery* q)
{  
  /*! _area должна уже быть проинициализирована !! */
  
  // читаем информацию из БД  
  int vessel_id = q->value("id").toUInt();
  
  ais::aisStaticData static_data = getAISStaticData(q); 
  ais::aisVoyageData voyage_data = getAISVoyageData(q);
  ais::aisDynamicData dynamic_data = getAISDynamicData(q);
  gps::gpsInitParams gps_params = getGPSInitParams(q, dynamic_data);
                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  _self_gps = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, _self_gps);
  _self_gps->open();
 
  
  // АИС
  _self_ais = new ais::SvSelfAIS(vessel_id, static_data, voyage_data, dynamic_data);
  _self_ais->setReceiveRange(ui->dspinAISRadius->value()); /** надо переделать */
  AISs.insert(vessel_id, _self_ais);
  _self_ais->open();
  
  
  // LAG
  
  
  /** --------- создаем объект собственного судна -------------- **/
  _self_vessel = new vsl::SvVessel(this, vessel_id);
  VESSELs.insert(vessel_id, _self_vessel);
  
  _self_vessel->mountGPS(_self_gps);
  _self_vessel->mountAIS(_self_ais);
//  _self_vessel->mountLAG(SELF_LAG);
  
  _self_vessel->assignMapObject(new SvMapObjectSelfVessel(_area, vessel_id));
     
  _area->scene->addMapObject(_self_vessel->mapObject());
  _self_vessel->mapObject()->setVisible(true);
  _self_vessel->mapObject()->setZValue(1);
  
  // подключаем
  connect(_self_gps, SIGNAL(newGPSData(const geo::GEOPOSITION&)), _self_ais, SLOT(newGPSData(const geo::GEOPOSITION&)));
  
  connect(_self_ais, &ais::SvSelfAIS::updateSelfVessel, _self_vessel, &vsl::SvVessel::updateVessel);
  connect(_self_ais, &ais::SvSelfAIS::updateVesselById, this, &MainWindow::on_update_vessel_by_id);
  connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, this, &updateMapObjectInfo);
  
  connect(this, &MainWindow::startEmulation, _self_gps, &gps::SvGPS::start);
  connect(this, &MainWindow::stopEmulation, _self_gps, &gps::SvGPS::stop);
  
//  connect(this, &MainWindow::startEmulation, _self_ais, &ais::SvAIS::start);
//  connect(this, &MainWindow::stopEmulation, _self_ais, &ais::SvAIS::stop);
  
  
  return _self_vessel;
  
}

vsl::SvVessel *MainWindow::createOtherVessel(QSqlQuery* q)
{  
  /*! _area должна уже быть проинициализирована !! */
  
  // читаем информацию из БД  
  int vessel_id = q->value("id").toUInt();
  
  ais::aisStaticData static_data = getAISStaticData(q); 
  ais::aisVoyageData voyage_data = getAISVoyageData(q);
  ais::aisDynamicData dynamic_data = getAISDynamicData(q);
  gps::gpsInitParams gps_params = getGPSInitParams(q, dynamic_data);
                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  gps::SvGPS* newGPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, newGPS);
  newGPS->open();
 
  
  // АИС
  ais::SvOtherAIS* newAIS;
  newAIS = new ais::SvOtherAIS(vessel_id, static_data, voyage_data, dynamic_data);
  AISs.insert(vessel_id, newAIS);
  newAIS->open();
  
  
  // LAG
  
  
  /** --------- создаем объект судна -------------- **/
  vsl::SvVessel* newVessel = new vsl::SvVessel(this, vessel_id);
  VESSELs.insert(vessel_id, newVessel);

  newVessel->mountGPS(newGPS);
  newVessel->mountAIS(newAIS);
//  newVessel->mountLAG(SELF_LAG);
  
  newVessel->assignMapObject(new SvMapObjectVessel(_area, vessel_id));
    
  _area->scene->addMapObject(newVessel->mapObject());
  newVessel->mapObject()->setVisible(true);
  newVessel->mapObject()->setZValue(1);
  
  // подключаем
  connect(newGPS, SIGNAL(newGPSData(const geo::GEOPOSITION&)), newAIS, SLOT(newGPSData(const geo::GEOPOSITION&)));
  
  connect(newAIS, &ais::SvOtherAIS::broadcast_ais_data, _self_ais, &ais::SvSelfAIS::on_receive_ais_data);
  
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, this, &updateMapObjectInfo);
  
  connect(this, &MainWindow::startEmulation, newGPS, &gps::SvGPS::start);
  connect(this, &MainWindow::startEmulation, newAIS, &ais::SvOtherAIS::start);
  
  connect(this, &MainWindow::stopEmulation, newGPS, &gps::SvGPS::stop);
  connect(this, &MainWindow::stopEmulation, newAIS, &ais::SvOtherAIS::stop);
  
  
  return newVessel;
  
}

void MainWindow::on_update_vessel_by_id(int id)
{
  foreach (vsl::SvVessel* vessel, VESSELs.values()) {
    
    if(vessel->id != id) continue;
    
    if(_self_ais->distanceTo(vessel->ais()) > _self_ais->receiveRange()) {
      
      vessel->mapObject()->setOutdated(true);
    }
    else {
      
      vessel->mapObject()->setOutdated(false);
      vessel->updateVessel();
      
    }  
    
  }
}

void MainWindow::on_actionNewVessel_triggered()
{
  VESSELEDITOR_UI = new SvVesselEditor(this);
  if(VESSELEDITOR_UI->exec() != QDialog::Accepted) {
    
    if(VESSELEDITOR_UI->result() != SvVesselEditor::rcNoError)
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении записи:\n%1").arg(VESSELEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete VESSELEDITOR_UI;
    
    return;
  }
  
  delete VESSELEDITOR_UI;
    
  
  /** ------ читаем информацию о судне --------- **/
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
//  qDebug() << QString(SQL_SELECT_VESSEL_WHERE_ID).arg(vessel_id);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_LAST_INSERTED_VESSEL), q).type()) {
    
    QMessageBox::critical(this, "Ошибка", q->lastError().databaseText(), QMessageBox::Ok);
    q->finish();
    delete q;
    
    return;
    
  }
  else {
  
    if(!q->next()) {
      
      QMessageBox::critical(this, "Ошибка", "Неизвестная ошибка", QMessageBox::Ok);
      q->finish();
      delete q;
      
      return;
    }
      
    /** --------- создаем судно ----------- **/
    vsl::SvVessel* vessel = createOtherVessel(q);
    q->finish();
    vessel->updateVessel();
    
  }
    
  
  
}
