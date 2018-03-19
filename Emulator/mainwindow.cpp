#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

extern SvSQLITE *SQLITE;

//vsl::SvVessel* SELF_VESSEL;

QMap<int, gps::SvGPS*> GPSs;
QMap<int, ais::SvAIS*> AISs;
QMap<int, vsl::SvVessel*> VESSELs;

//extern ais::SvAIS* SELF_AIS;
//extern gps::SvGPS* SELF_GPS;
//extern QMap<int, vsl::SvVessel*> VESSELS;
extern SvVesselEditor* VESSELEDITOR_UI;

extern QMap<int, ais::aisNavStat> NAVSTATS;

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
  ui->dockGraphics->resize(gw.size);
  ui->dockGraphics->move(gw.position);
//  ui->dockWidget->setWindowState(gw.state);
  
  /* параметры окна информации о текущем объекте */
  AppParams::WindowParams iw = AppParams::readWindowParams(this, "INFO WINDOW");
  ui->dockCarrentInfo->resize(iw.size);
  ui->dockCarrentInfo->move(iw.position);
//  ui->dockWidget->setWindowState(gw.state);
  
  ui->dspinAISRadius->setValue(AppParams::readParam(this, "GENERAL", "AISRadius", 2).toReal());
  
  /* лог */
  log = svlog::SvLog(ui->textLog);
  
  QList<QSerialPortInfo> splst = QSerialPortInfo::availablePorts();
  foreach (QSerialPortInfo spinf, splst) {
    ui->cbAISSelectInterface->addItem(spinf.portName());
    ui->cbLAGSelectInterface->addItem(spinf.portName());
    ui->cbNAVTEKSelectInterface->addItem(spinf.portName());
  }
  
  
}

bool MainWindow::init()
{
  try {
    QString map_file_name = AppParams::readParam(this, "General", "xml", "").toString();
    QString db_file_name = AppParams::readParam(this, "General", "db", "").toString();
    
    
    /** -------- создаем область отображения -------- **/
    _area = new area::SvArea(ui->dockGraphics);
    ui->vlayDock->addWidget(_area);
    
    if(_area->readBounds(map_file_name)) 
      _area->readMap(map_file_name);
    
    else 
      QMessageBox::warning(this, tr("Ошибка в файле XML"), tr("Неверные границы области (bounds)"));
  
    _area->setUp("area_1");
    
    
    /** ---------- открываем БД ----------- **/
    SQLITE = new SvSQLITE(this, db_file_name);
    QSqlError err = SQLITE->connectToDB();
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    _query = new QSqlQuery(SQLITE->db);
    
    /** грузим списки **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAV_STATS), _query).type()) 
      _exception.raise(_query->lastError().databaseText());

    while(_query->next()) {    
      
      ais::aisNavStat stat;
      
      int id = _query->value("id").toInt();
      stat.name = _query->value("status_name").toString();
      stat.static_voyage_interval = _query->value("static_voyage_interval").toUInt();
//      stat.voyage_interval = _query->value("voyage_interval").toUInt();
      stat.dynamic_interval = _query->value("dynamic_interval").toUInt();

      NAVSTATS.insert(id, stat);
    }
    
    _query->finish();
    
    qInfo() << 1;
    
    
    /*! необходима проверка, что в таблице vessels существует запись с флагом sef == true !*/
SV:
  
    /** ------ читаем информацию о собственном судне --------- **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(true),_query).type())
      _exception.raise(_query->lastError().databaseText());
    
    if(!_query->next()) {
      
      QMessageBox::warning(this, "", "В БД нет сведений о собственном судне", QMessageBox::Ok);
      _query->finish();
      
      VESSELEDITOR_UI = new SvVesselEditor(this, -1, true);
      if(QDialog::Accepted == VESSELEDITOR_UI->exec()) {
        
        goto SV;    
              
      }
      else {
        
        if(VESSELEDITOR_UI->result() != SvVesselEditor::rcNoError)
          _exception.raise(VESSELEDITOR_UI->last_error());
      }
        
      delete VESSELEDITOR_UI;
      
    }
  
    
    qInfo() << 2;
    
    /** --------- создаем собственное судно ----------- **/
    createSelfVessel(_query);
    _query->finish();
    
  //  _self_vessel = createSelfVessel(q); //self_vessel_id, self_gps_params, self_static_data, self_voyage_data, self_dynamic_data, true);
  //  _self_ais = _self_vessel->ais();
    _self_vessel->updateVessel();
  
    qInfo() << 3;
   
    
    /** ------ читаем список судов --------- **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(false), _query).type())
      _exception.raise(_query->lastError().databaseText());

    
    /** --------- создаем суда ----------- **/
    while(_query->next()) {
      vsl::SvVessel* vessel = createOtherVessel(_query);
      vessel->updateVessel();
      
    }
    _query->finish();
  
  qInfo() << 4;
    
    connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    
    connect(this, SIGNAL(newState(States)), this, SLOT(stateChanged(States)));
    
    return true;
  }
  
  catch(SvException &exception) {
    qInfo() << 5;
    if(_query) delete _query;
    if(_area) delete _area;
    if(_self_gps) delete _self_gps;    
    if(_self_ais) delete _self_ais;
    if(_self_vessel) delete _self_vessel;
    if(_self_lag) delete _self_lag;
    
    if(VESSELEDITOR_UI) delete VESSELEDITOR_UI;
    
    if(VESSELs.count()) {
      foreach (int key, VESSELs.keys()) {
        delete VESSELs.take(key);
      }
    }
    
    
    if(AISs.count()) {
      foreach (int key, AISs.keys()) {
        delete AISs.take(key);
      }
    }

    if(GPSs.count()) {
      foreach (int key, GPSs.keys()) {
        delete GPSs.take(key);
      }
    }    
    
    QMessageBox::critical(this, "Ошибка", QString("Ошибка инициализации:\n%1").arg(exception.err), QMessageBox::Ok);
    
    return false;
  }
}

MainWindow::~MainWindow()
{
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveWindowParams(ui->dockGraphics, ui->dockGraphics->size(), ui->dockGraphics->pos(), ui->dockGraphics->windowState(), "AREA WINDOW");
  AppParams::saveWindowParams(ui->dockCarrentInfo, ui->dockCarrentInfo->size(), ui->dockCarrentInfo->pos(), ui->dockCarrentInfo->windowState(), "INFO WINDOW");
  
  AppParams::saveParam(this, "GENERAL", "AISRadius", QVariant(ui->dspinAISRadius->value()));
  
  delete ui;
}

gps::gpsInitParams MainWindow::getGPSInitParams(QSqlQuery* q, ais::aisDynamicData& dynamic_data, int vessel_id)
{
  gps::gpsInitParams result;
  QDateTime dt = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел

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
    
    geo::COORDINATES coord = geo::get_rnd_coordinates(_area->bounds(), vessel_id + dt.time().second() * 1000 + dt.time().msec());
    
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
  result.mmsi = q->value("static_mmsi").toUInt();
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
  result.geoposition.course = q->value("dynamic_course").isNull() ? -1 : q->value("dynamic_course").toReal();
  result.geoposition.speed = q->value("dynamic_speed").isNull() ? -1 : q->value("dynamic_speed").toReal();
//  result.navstat = q->value("nav_status_id").toUInt();
  
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
      break;
    }
      
    default:
      break;
  }
  
  _current_state = state;
  QApplication::processEvents();
  
}

void MainWindow::on_bnCycle_clicked()
{
  QSerialPortInfo info;
  info = QSerialPortInfo::availablePorts().first();
  _self_lag->setSerialPortInfo(info);
  
  switch (_current_state) {
    
    case sStopped:
    {
      emit newState(sRunning);
      
      try {
        
        /** открываем порты устройств **/
        if(!_self_lag->open()) _exception.raise(_self_lag->lastError());
//        if(!_self_lag->open()) _exception.raise(_self_lag->lastError());
//        if(!_self_lag->open()) _exception.raise(_self_lag->lastError());
      }
      
      catch(SvException &e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка открытия порта.\n%1").arg(e.err), QMessageBox::Ok);
        emit newState(sStopped);
        return;
      }
      
      emit setMultiplier(ui->spinEmulationMultiplier->value());
      
      emit startGPSEmulation(0);
      
      emit startAISEmulation(0);
      
      emit startLAGEmulation(ui->spinLAGUploadDataPeriod->value());
      
      emit newState(sRunned);
      break;
    }
      
    case sRunned:
    {
      emit newState(sStopping);
      
      emit stopLAGEmulation();
      
      emit stopAISEmulation();
      
      emit stopGPSEmulation();
      
      foreach (gps::SvGPS* gps, GPSs.values()) {
        gps->waitWhileRunned();
//        gps->stop();
      }
      
      
      /** закрываем порты **/
      _self_lag->close();
      
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

  updateMapObjectInfo(mo);
  
}

void MainWindow::updateMapObjectInfo(SvMapObject* mapObject)
{
  if(!mapObject->isSelected())
    return;
  
  switch (mapObject->type()) {
    
    case motSelfVessel:
    case motVessel: 
    {
      
      if(AISs.find(mapObject->id()) != AISs.end()) {
        
        ais::SvAIS* a = AISs.value(mapObject->id());
      
        ui->textEdit_2->setText(QString("Текущий объект:\nID:\t%1\nCallsign:\t%2\nMMSI:\t%3\nIMO:\t\t%4\nDest:\t%5\nDraft:\t%6\nTeam:\t%7\n\n"
                                        "Текущая геопозиция:\nШирота:\t%8\nДолгота:\t%9\nКурс:\t%10%11\nСкорость:\t%12 км/ч\nСтатус:\t%13")
                            .arg(a->vesselId())
                            .arg(a->getStaticData()->callsign)
                            .arg(a->getStaticData()->mmsi)
                            .arg(a->getStaticData()->imo)
                            .arg(a->getVoyageData()->destination)
                            .arg(a->getVoyageData()->draft)
                            .arg(a->getVoyageData()->team)
                            .arg(a->getDynamicData()->geoposition.latitude, 0, 'g', 4)
                            .arg(a->getDynamicData()->geoposition.longtitude, 0, 'g', 4)
                            .arg(a->getDynamicData()->geoposition.course)
                            .arg(QChar(176))
                            .arg(a->getDynamicData()->geoposition.speed, 0, 'g', 2)
                            .arg(NAVSTATS.value(a->navStatus()).name));
        
      }
        
      break;
    }
        
      
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
  gps::gpsInitParams gps_params = getGPSInitParams(q, dynamic_data, vessel_id);
                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  _self_gps = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, _self_gps);
  
  // АИС
  _self_ais = new ais::SvSelfAIS(vessel_id, static_data, voyage_data, dynamic_data, log);
  _self_ais->setReceiveRange(ui->dspinAISRadius->value()); /** надо переделать */
  AISs.insert(vessel_id, _self_ais);
  
  // LAG
  _self_lag = new lag::SvLAG(vessel_id, dynamic_data.geoposition, log);

  
  /** --------- создаем объект собственного судна -------------- **/
  _self_vessel = new vsl::SvVessel(this, vessel_id);
  VESSELs.insert(vessel_id, _self_vessel);
  
  _self_vessel->mountGPS(_self_gps);
  _self_vessel->mountAIS(_self_ais);
  _self_vessel->mountLAG(_self_lag);
  
  _self_vessel->assignMapObject(new SvMapObjectSelfVessel(_area, vessel_id));
     
  _area->scene->addMapObject(_self_vessel->mapObject());
  _self_vessel->mapObject()->setVisible(true);
  _self_vessel->mapObject()->setZValue(1);
  
  // подключаем
  connect(_self_gps, SIGNAL(newGPSData(const geo::GEOPOSITION&)), _self_ais, SLOT(newGPSData(const geo::GEOPOSITION&)));
  connect(_self_gps, SIGNAL(newGPSData(const geo::GEOPOSITION&)), _self_lag, SLOT(newGPSData(const geo::GEOPOSITION&)));
  
  connect(_self_ais, &ais::SvSelfAIS::updateSelfVessel, _self_vessel, &vsl::SvVessel::updateVessel);
  connect(_self_ais, &ais::SvSelfAIS::updateVesselById, this, &MainWindow::update_vessel_by_id);
  connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, this, &updateMapObjectInfo);
  
  connect(this, &MainWindow::setMultiplier, _self_gps, &gps::SvGPS::set_multiplier);
  connect(this, &MainWindow::startGPSEmulation, _self_gps, &gps::SvGPS::start);
  connect(this, &MainWindow::stopGPSEmulation, _self_gps, &gps::SvGPS::stop);
  
  connect(this, &MainWindow::startLAGEmulation, _self_lag, &lag::SvLAG::start);
  connect(this, &MainWindow::stopLAGEmulation, _self_lag, &lag::SvLAG::stop);
  
  ui->textEdit->setText(QString("ID:\t%1\nCallsign:\t%2\nMMSI:\t%3\nIMO:\t\t%4\nDest:\t%5\nDraft:\t%6\nTeam:\t%7")
                        .arg(vessel_id)
                        .arg(static_data.callsign)
                        .arg(static_data.mmsi)
                        .arg(static_data.imo)
                        .arg(voyage_data.destination)
                        .arg(voyage_data.draft)
                        .arg(voyage_data.team));
  
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
  gps::gpsInitParams gps_params = getGPSInitParams(q, dynamic_data, vessel_id);
                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  gps::SvGPS* newGPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, newGPS);
 
  
  // АИС
  ais::SvOtherAIS* newAIS;
  newAIS = new ais::SvOtherAIS(vessel_id, static_data, voyage_data, dynamic_data);
  AISs.insert(vessel_id, newAIS);

  
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
  
  newVessel->mapObject()->setIdentifier(new SvMapObjectIdentifier(_area, newVessel->mapObject(), vessel_id));
  _area->scene->addMapObject(newVessel->mapObject()->identifier());
  newVessel->mapObject()->identifier()->setVisible(true);
  newVessel->mapObject()->identifier()->setZValue(1);
      
  // подключаем
  connect(newGPS, SIGNAL(newGPSData(const geo::GEOPOSITION&)), newAIS, SLOT(newGPSData(const geo::GEOPOSITION&)));
  
  connect(newAIS, &ais::SvOtherAIS::broadcast_ais_data, _self_ais, &ais::SvSelfAIS::on_receive_ais_data);
  
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, this, &updateMapObjectInfo);
  
  connect(this, &MainWindow::setMultiplier, newGPS, &gps::SvGPS::set_multiplier);
  
  connect(this, &MainWindow::startGPSEmulation, newGPS, &gps::SvGPS::start);
  connect(this, &MainWindow::stopGPSEmulation, newGPS, &gps::SvGPS::stop);
  
  connect(this, &MainWindow::startAISEmulation, newAIS, &ais::SvOtherAIS::start);
  connect(this, &MainWindow::stopAISEmulation, newAIS, &ais::SvOtherAIS::stop);
  
  ui->listVessels->addItem(QString("%1 %2").arg(vessel_id).arg(static_data.callsign));
  
  return newVessel;
  
}

void MainWindow::update_vessel_by_id(int id)
{
  foreach (vsl::SvVessel* vessel, VESSELs.values()) {
    
    if(vessel->id != id) continue;
    
    qreal dst = _self_ais->distanceTo(vessel->ais());
    if(_self_ais->distanceTo(vessel->ais()) > _self_ais->receiveRange() * 1000) {
      
      ((SvMapObjectVessel*)(vessel->mapObject()))->setOutdated(true);
    }
    else {
      
      ((SvMapObjectVessel*)(vessel->mapObject()))->setOutdated(false);
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

void MainWindow::on_listVessels_currentRowChanged(int currentRow)
{
  QStringList lst = ui->listVessels->currentItem()->text().split(" ");
  int id = QString(lst.first()).toInt();
  
  
  if(VESSELs.find(id) != VESSELs.end()) {
    
    foreach (QGraphicsItem* item, _area->scene->selectedItems()) {
      item->setSelected(false);
    } 
    
        
    vsl::SvVessel* v = VESSELs.value(id);
    
    v->mapObject()->setSelected(true);
  
  }
  
}
