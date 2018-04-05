#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

extern SvSQLITE *SQLITE;
extern SvSerialEditor* SERIALEDITOR_UI;

//vsl::SvVessel* SELF_VESSEL;

QMap<int, gps::SvGPS*> GPSs;
QMap<int, ais::SvAIS*> AISs;
QMap<int, vsl::SvVessel*> VESSELs;
QMap<int, QListWidgetItem*> LISTITEMs;

//extern ais::SvAIS* SELF_AIS;
//extern gps::SvGPS* SELF_GPS;
//extern QMap<int, vsl::SvVessel*> VESSELS;
extern SvVesselEditor* VESSELEDITOR_UI;

extern QMap<quint32, ais::aisNavStat> NAVSTATs;


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
  
//  foreach (QSerialPortInfo spinf, splst) {
//    spinf.manufacturer()
//    ui->cbAISSelectInterface->addItem(spinf.portName());
//    ui->cbLAGSelectInterface->addItem(spinf.portName());
//    ui->cbNAVTEKSelectInterface->addItem(spinf.portName());
//  }

  _font_default.setItalic(false);
  _font_inactive.setItalic(true);
  _font_nolink.setItalic(true);
  
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
    /// список навигационных статусов
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAV_STATS), _query).type()) 
      _exception.raise(_query->lastError().databaseText());

    while(_query->next()) {    
      
      ais::aisNavStat stat;
      
      stat.ITU_id = _query->value("ITU_id").toUInt();
      stat.name = _query->value("status_name").toString();
      stat.static_voyage_interval = _query->value("static_voyage_interval").toUInt();
//      stat.voyage_interval = _query->value("voyage_interval").toUInt();
//      stat.dynamic_interval = _query->value("dynamic_interval").toUInt();

      NAVSTATs.insert(stat.ITU_id, stat);
    }
    _query->finish();
    
    
    /// параметры COM портов
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_FROM_SERIAL_PARAMS), _query).type()) 
      _exception.raise(_query->lastError().databaseText());

    while(_query->next()) {    
      
      idev::SvSimulatedDeviceTypes dt = idev::SvSimulatedDeviceTypes(_query->value("device_type").toUInt());
      
      switch (dt) {
        case idev::sdtLAG:
          
          _lag_serial_params.name =         _query->value("port_name").toString();        
          _lag_serial_params.description =  _query->value("description").toString();
          _lag_serial_params.baudrate =     _query->value("baudrate").toInt();                              
          _lag_serial_params.databits =     QSerialPort::DataBits(_query->value("data_bits").toInt());      
          _lag_serial_params.flowcontrol =  QSerialPort::FlowControl(_query->value("flow_control").toInt());
          _lag_serial_params.parity =       QSerialPort::Parity(_query->value("parity").toInt());           
          _lag_serial_params.stopbits =     QSerialPort::StopBits(_query->value("stop_bits").toInt());  
          ui->editLAGSerialInterface->setText(_lag_serial_params.description);
          break;
          
        case idev::sdtSelfAIS:
          _ais_serial_params.name =         _query->value("port_name").toString();
          _ais_serial_params.description =  _query->value("description").toString();
          _ais_serial_params.baudrate =     _query->value("baudrate").toInt();
          _ais_serial_params.databits =     QSerialPort::DataBits(_query->value("data_bits").toInt());
          _ais_serial_params.flowcontrol =  QSerialPort::FlowControl(_query->value("flow_control").toInt());
          _ais_serial_params.parity =       QSerialPort::Parity(_query->value("parity").toInt());
          _ais_serial_params.stopbits =     QSerialPort::StopBits(_query->value("stop_bits").toInt());
          ui->editAISSerialInterface->setText(_ais_serial_params.description);
          
        case idev::sdtNavteks:
          
//          _navteks_serial_params.name =        _query->value("port_name").toString();                          
//          _navteks_serial_params.baudrate =    _query->value("baudrate").toInt();                              
//          _navteks_serial_params.databits =    QSerialPort::DataBits(_query->value("data_bits").toInt());      
//          _navteks_serial_params.flowcontrol = QSerialPort::FlowControl(_query->value("flow_control").toInt());
//          _navteks_serial_params.parity =      QSerialPort::Parity(_query->value("parity").toInt());           
//          _navteks_serial_params.stopbits =    QSerialPort::StopBits(_query->value("stop_bits").toInt());      
          
        case idev::sdtEcho:
          
//          _echo_serial_params.name =  
//          _echo_serial_params.baudrate = 
//          _echo_serial_params.databits = 
//          _echo_serial_params.flowcontrol = 
//          _echo_serial_params.parity = 
//          _echo_serial_params.stopbits = 
          
        default:
          break;
      }
      
      
      
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
        
        if(!VESSELEDITOR_UI->last_error().isEmpty())
          _exception.raise(VESSELEDITOR_UI->last_error());
      }
        
      delete VESSELEDITOR_UI;
      
    }
  
    
    qInfo() << 2;
    
    /** --------- создаем собственное судно ----------- **/
    createSelfVessel(_query);
    
    _query->finish();

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
    
    connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(area_selection_changed()));
    connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
    
    connect(this, SIGNAL(newState(States)), this, SLOT(stateChanged(States)));
   
//    QStringList l = nmea::ais_message_5(0, _self_ais->getStaticData(), _self_ais->getVoyageData(), _self_ais->navStatus());
//    QUdpSocket* udp = new QUdpSocket();
//    for(QString s: l) {
//      QByteArray b(s.toStdString().c_str(), s.size());
//      udp->writeDatagram(b, QHostAddress("192.168.44.228"), 29421);
//      Sleep(100);
//      log << s << svlog::endl;
////      qDebug() << s;
      
//    }
//    udp->close();
//    delete udp;
    
    bool b = false;
    QString s = "";
    int i = -100;
    i = s.toInt(&b);
    
    qDebug() << b << i << bool(!b && !s.isEmpty());
    
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

gps::gpsInitParams MainWindow::readGPSInitParams(QSqlQuery* q, ais::aisDynamicData& dynamic_data, int vessel_id)
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

ais::aisStaticVoyageData  MainWindow::readAISStaticVoyageData(QSqlQuery* q)
{
  ais::aisStaticVoyageData result;
  result.id = q->value("id").toUInt();
  result.mmsi = q->value("static_mmsi").toUInt();
  result.imo = q->value("static_imo").toUInt();
  result.callsign = q->value("static_callsign").toString();
  result.name = q->value("static_name").toString();
  result.pos_ref_A = q->value("static_pos_ref_A").isNull() ? 20 : q->value("static_pos_ref_A").toUInt();
  result.pos_ref_B = q->value("static_pos_ref_B").isNull() ? 20 : q->value("static_pos_ref_B").toUInt();
  result.pos_ref_C = q->value("static_pos_ref_C").isNull() ? 10 : q->value("static_pos_ref_C").toUInt();
  result.pos_ref_D = q->value("static_pos_ref_D").isNull() ? 10 : q->value("static_pos_ref_D").toUInt();
  result.vessel_ITU_id = q->value("static_type_ITU_id").toUInt();
//  result.vessel_type_name = q->value("static_vessel_type_name").toString();
  result.DTE = q->value("static_DTE").toUInt();
  result.talkerID = q->value("static_talker_id").toString();
  
  result.cargo_ITU_id = q->value("voyage_cargo_ITU_id").toUInt();
//  result.cargo_type_name = q->value("voyage_cargo_type_name").toString();
  result.destination = q->value("voyage_destination").toString();
  result.ETA_utc = q->value("voyage_ETA_utc").toTime();
  result.ETA_day = q->value("voyage_ETA_day").toUInt();
  result.ETA_month = q->value("voyage_ETA_month").toUInt();
  result.draft = q->value("voyage_draft").toReal();
  result.team = q->value("voyage_team").toUInt();
  
  return result;
}

ais::aisDynamicData MainWindow::readAISDynamicData(QSqlQuery* q)
{
  ais::aisDynamicData result;
  result.geoposition.latitude = q->value("dynamic_latitude").isNull() ? -1.0 : q->value("dynamic_latitude").toReal();
  result.geoposition.longtitude = q->value("dynamic_longtitude").isNull() ? -1.0 : q->value("dynamic_longtitude").toReal();
  result.geoposition.course = q->value("dynamic_course").isNull() ? -1 : q->value("dynamic_course").toReal();
  result.geoposition.speed = q->value("dynamic_speed").isNull() ? -1 : q->value("dynamic_speed").toReal();
 
  return result;
}

ais::aisNavStat MainWindow::readNavStat(QSqlQuery* q)
{
  ais::aisNavStat result;
  result.ITU_id = q->value("nav_status_ITU_id").toUInt(); 
  result.name = q->value("nav_status_name").toString(); 
  result.static_voyage_interval = q->value("nav_status_static_voyage_interval").toUInt(); 
//  result.ITU_id = q->value("nav_status_ITU_id").toUInt(); 
  return result;
}

void MainWindow::stateChanged(States state)
{
  switch (state) {
    
    case sRunned:
    {
      ui->tabWidget->setEnabled(true);
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
  switch (_current_state) {
    
    case sStopped:
    {
      emit newState(sRunning);
      
      try {
        
        /** открываем порты устройств **/
        /// LAG
        if(ui->checkLAGEnabled->isChecked()) {
          
          _self_lag->setSerialPortParams(_lag_serial_params);
          if(!_self_lag->open()) _exception.raise(QString("ЛАГ: %1").arg(_self_lag->lastError()));
        }
        
        /// AIS
        if(ui->checkAISEnabled->isChecked()) {
         
          _self_ais->setSerialPortParams(_ais_serial_params);
          if(!_self_ais->open()) _exception.raise(QString("АИС: %1").arg(_self_ais->lastError()));
          
        }
        
        
//        if(!_self_lag->open()) _exception.raise(_self_lag->lastError());
      }
      
      catch(SvException &e) {
        if(_self_ais->isOpened()) _self_ais->close();
        if(_self_lag->isOpened()) _self_lag->close();
        
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

      /// LAG      
      if(ui->checkLAGEnabled->isChecked()) _self_lag->close();
      
      /// AIS
      if(ui->checkAISEnabled->isChecked()) _self_ais->close();
      
      
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
void MainWindow::area_selection_changed()
{
  /// ищем все выделения и удаляем их
  for (SvMapObject* item: _area->scene->mapObjects()) {
    if(item->selection()) {  
      _area->scene->removeMapObject(item->selection());
      item->deleteSelection();
    }
  }

  /// если ничего не  выделено, то сбрасываем
  if(_area->scene->selectedItems().isEmpty()) {
    _selected_vessel_id = -1;
    _area->setLabelInfo("");
  }
  else {
    /// создаем новое выделение
    SvMapObject* mo = (SvMapObject*)(_area->scene->selectedItems().first());
    mo->setSelection(new SvMapObjectSelection(_area, mo));
    _area->scene->addMapObject(mo->selection());
    mo->selection()->setVisible(true);
    _area->scene->setMapObjectPos(mo, mo->geoPosition());
    _selected_vessel_id = mo->id();
    updateMapObjectInfo(mo);
  }

  /// выделяем судно в списке  
  disconnect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
  
  if((_selected_vessel_id == -1) /*|| (_self_vessel->id == _selected_vessel_id)*/) {\
    ui->listVessels->setCurrentRow(-1);
    ui->textMapObjectInfo->setText("");
  }
  else ui->listVessels->setCurrentItem(LISTITEMs.value(_selected_vessel_id));

  bool b = ui->listVessels->currentRow() > -1;
  ui->actionEditVessel->setEnabled(b); 
  ui->actionRemoveVessel->setEnabled(b); 
  ui->bnRemoveVessel->setEnabled(b);
  ui->bnEditVessel->setEnabled(b);
  
  connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
  
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
      
        ui->textMapObjectInfo->setText(QString("Текущее судно:\nID:\t%1\nCallsign:\t%2\nMMSI:\t%3\nIMO:\t%4\nDest:\t%5\nDraft:\t%6\nTeam:\t%7\n\n"
                                        "Текущая геопозиция:\nШирота:\t%8\nДолгота:\t%9\nКурс:\t%10%11\nСкорость:\t%12 км/ч\nСтатус:\t%13")
                            .arg(a->vesselId())
                            .arg(a->staticVoyageData()->callsign)
                            .arg(a->staticVoyageData()->mmsi)
                            .arg(a->staticVoyageData()->imo)
                            .arg(a->staticVoyageData()->destination)
                            .arg(a->staticVoyageData()->draft)
                            .arg(a->staticVoyageData()->team)
                            .arg(a->dynamicData()->geoposition.latitude, 0, 'g', 4)
                            .arg(a->dynamicData()->geoposition.longtitude, 0, 'g', 4)
                            .arg(a->dynamicData()->geoposition.course)
                            .arg(QChar(176))
                            .arg(a->dynamicData()->geoposition.speed, 0, 'g', 2)
                            .arg(a->navStatus()->name));
        
      }
        
      break;
    }
        
      
    default:
      break;
  }
}

vsl::SvVessel* MainWindow::createSelfVessel(QSqlQuery* q)
{  
  /*! _area должна уже быть проинициализирована !! */
  
  // читаем информацию из БД  
  int vessel_id = q->value("id").toUInt();
  
  ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
  ais::aisDynamicData dynamic_data = readAISDynamicData(q);
  gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, vessel_id);
  ais::aisNavStat nav_stat = readNavStat(q);
  
  
  /** ----- создаем устройства ------ **/
  // GPS
  _self_gps = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, _self_gps);
  
  // АИС
  _self_ais = new ais::SvSelfAIS(vessel_id, static_voyage_data, dynamic_data, log);
  _self_ais->setReceiveRange(ui->dspinAISRadius->value()); /** надо переделать */
  AISs.insert(vessel_id, _self_ais);
  _self_ais->setNavStatus(nav_stat);
  
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
  
  
  LISTITEMs.insert(vessel_id, new QListWidgetItem(QIcon(":/icons/Icons/lock.png"), QString("%1\t%2 [Собственный]").arg(vessel_id).arg(static_voyage_data.name)));
  ui->listVessels->addItem(LISTITEMs.value(vessel_id));
     
  
  return _self_vessel;
  
}

vsl::SvVessel* MainWindow::createOtherVessel(QSqlQuery* q)
{  
  /*! _area должна уже быть проинициализирована !! */
  
  // читаем информацию из БД  
  int vessel_id = q->value("id").toUInt();
  bool is_active = q->value("is_active").toBool();
  
  ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
  ais::aisDynamicData dynamic_data = readAISDynamicData(q);
  gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, vessel_id);
  ais::aisNavStat nav_stat = readNavStat(q);                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  gps::SvGPS* newGPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds());
  GPSs.insert(vessel_id, newGPS);
 
  
  // АИС
  ais::SvOtherAIS* newAIS;
  newAIS = new ais::SvOtherAIS(vessel_id, static_voyage_data, dynamic_data);
  AISs.insert(vessel_id, newAIS);
  newAIS->setNavStatus(nav_stat);
  
  /** --------- создаем объект судна -------------- **/
  vsl::SvVessel* newVessel = new vsl::SvVessel(this, vessel_id);
  VESSELs.insert(vessel_id, newVessel);

  newVessel->mountGPS(newGPS);
  newVessel->mountAIS(newAIS);
//  newVessel->mountLAG(SELF_LAG);
  
  newVessel->assignMapObject(new SvMapObjectOtherVessel(_area, vessel_id));
  
  newVessel->setActive(is_active);
    
  _area->scene->addMapObject(newVessel->mapObject());
  newVessel->mapObject()->setVisible(true);
  newVessel->mapObject()->setZValue(1);
  connect(&newVessel->mapObject()->signalHandler, SIGNAL(mouseDoubleClick(int)), this, SLOT(editVessel(int)));
  
  newVessel->mapObject()->setIdentifier(new SvMapObjectIdentifier(_area, newVessel->mapObject()));
  _area->scene->addMapObject(newVessel->mapObject()->identifier());
  newVessel->mapObject()->identifier()->setVisible(true);
  newVessel->mapObject()->identifier()->setZValue(1);
  newVessel->mapObject()->setActive(is_active);
      
  
  // подключаем
  connect(newGPS, SIGNAL(newGPSData(const geo::GEOPOSITION&)), newAIS, SLOT(newGPSData(const geo::GEOPOSITION&)));
  
  connect(newAIS, &ais::SvOtherAIS::broadcast_message, _self_ais, &ais::SvSelfAIS::on_receive_message);
  
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
  connect(newVessel, &vsl::SvVessel::updateMapObjectPos, this, &updateMapObjectInfo);
  
  connect(_self_ais, &ais::SvSelfAIS::interrogateRequest, newAIS, &ais::SvOtherAIS::on_interrogate);
  
  connect(this, &MainWindow::setMultiplier, newGPS, &gps::SvGPS::set_multiplier);
  
  connect(this, &MainWindow::startGPSEmulation, newGPS, &gps::SvGPS::start);
  connect(this, &MainWindow::stopGPSEmulation, newGPS, &gps::SvGPS::stop);
  
  connect(this, &MainWindow::startAISEmulation, newAIS, &ais::SvOtherAIS::start);
  connect(this, &MainWindow::stopAISEmulation, newAIS, &ais::SvOtherAIS::stop);
  
  LISTITEMs.insert(vessel_id, new QListWidgetItem(QIcon(), QString("%1\t%2").arg(vessel_id).arg(static_voyage_data.name)));
  ui->listVessels->addItem(LISTITEMs.value(vessel_id));
  
  LISTITEMs.value(vessel_id)->setTextColor(newVessel->isActive() ? QColor(DEFAULT_VESSEL_PEN_COLOR) : QColor(INACTIVE_VESSEL_COLOR));
  LISTITEMs.value(vessel_id)->setFont(newVessel->isActive() ? _font_default : _font_inactive);
  LISTITEMs.value(vessel_id)->setIcon(newVessel->isActive() ? QIcon(":/icons/Icons/bullet_white.png") : QIcon(":/icons/Icons/bullet_red.png"));
  
  return newVessel;
  
}

void MainWindow::update_vessel_by_id(int id)
{
  foreach (vsl::SvVessel* vessel, VESSELs.values()) {
    
    if(vessel->id != id) continue;
    
    if(_self_ais->distanceTo(vessel->ais()) > _self_ais->receiveRange() * 1000) {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(true);
  
      LISTITEMs.value(id)->setIcon(QIcon(":/icons/Icons/link_break.png"));
      LISTITEMs.value(id)->setFont(_font_nolink);
      LISTITEMs.value(id)->setTextColor(QColor(OUTDATED_VESSEL_COLOR));
      
    }
    else {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(false);
      
      LISTITEMs.value(id)->setIcon(QIcon(":/icons/Icons/bullet_green.png"));
      LISTITEMs.value(id)->setFont(_font_default);
      LISTITEMs.value(id)->setTextColor(QColor(DEFAULT_VESSEL_PEN_COLOR));
      
    }  
    
    vessel->updateVessel();
    
  }
}

void MainWindow::on_actionNewVessel_triggered()
{
  VESSELEDITOR_UI = new SvVesselEditor(this);
  if(VESSELEDITOR_UI->exec() != QDialog::Accepted) {
    
    if(!VESSELEDITOR_UI->last_error().isEmpty())
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

void MainWindow::on_actionEditVessel_triggered()
{
  editVessel(_selected_vessel_id);
}

void MainWindow::editVessel(int id)
{
  if(_current_state != sStopped)
    return;
  
  VESSELEDITOR_UI = new SvVesselEditor(0, id);
  if(VESSELEDITOR_UI->exec() != QDialog::Accepted) {
    
    if(!VESSELEDITOR_UI->last_error().isEmpty())
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при редактировании записи:\n%1").arg(VESSELEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete VESSELEDITOR_UI;
    
    return;
  }
  
  delete VESSELEDITOR_UI;
    
  
  /** ------ читаем информацию о судне --------- **/
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
//  qDebug() << QString(SQL_SELECT_VESSEL_WHERE_ID).arg(vessel_id);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_ID).arg(id), q).type()) {
    
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
      
    /** --------- изменяем данные судна ----------- **/
    
    ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
    ais::aisDynamicData *dynamic_data = VESSELs.value(id)->ais()->dynamicData();
    gps::gpsInitParams gps_params = readGPSInitParams(q, *dynamic_data, id);
    q->finish();
    
//    dynamic_data.geoposition = VESSELs.value(id)->ais()->getDynamicData()->geoposition;
    
    vsl::SvVessel* vessel = VESSELs.value(id);
    
    vessel->ais()->setStaticVoyageData(static_voyage_data);
    vessel->gps()->setInitParams(gps_params);
    
    vessel->updateVessel();
    
    LISTITEMs.value(id)->setText(QString("%1\t%2").arg(id).arg(static_voyage_data.name));
  }
}


void MainWindow::on_bnAISEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_ais_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty())
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete SERIALEDITOR_UI;
    
    return;
    
  }
  
  _ais_serial_params.name = SERIALEDITOR_UI->params.name;
  _ais_serial_params.description = SERIALEDITOR_UI->params.description;
  _ais_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _ais_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _ais_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _ais_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _ais_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;
  
  ui->editAISSerialInterface->setText(_ais_serial_params.description);

}

void MainWindow::on_bnLAGEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_lag_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(SERIALEDITOR_UI->result() != SvSerialEditor::rcNoError)
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete SERIALEDITOR_UI;
    
    return;
    
  }
  
  _lag_serial_params.name = SERIALEDITOR_UI->params.name;
  _lag_serial_params.description = SERIALEDITOR_UI->params.description;
  _lag_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _lag_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _lag_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _lag_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _lag_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;

  ui->editLAGSerialInterface->setText(_lag_serial_params.description);
}

void MainWindow::on_bnNAVTEKEditSerialParams_clicked()
{
//  SERIALEDITOR_UI = new SvSerialEditor(_navtek_serial, this);
//  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

//  }
  
//  delete SERIALEDITOR_UI;

}

void MainWindow::currentVesselListItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
  disconnect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(area_selection_changed()));
  
  foreach (SvMapObject* obj, _area->scene->mapObjects()) 
    obj->setSelected(obj->id() == LISTITEMs.key(current)); 
  
  area_selection_changed();
  
  connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(area_selection_changed()));
  
}

void MainWindow::on_listVessels_doubleClicked(const QModelIndex &index)
{
   editVessel(LISTITEMs.key(ui->listVessels->item(index.row())));
}

void MainWindow::on_bnSetActive_clicked()
{
  if(_selected_vessel_id == _self_vessel->id)
    return;
  
  if(VESSELs.find(_selected_vessel_id) == VESSELs.end())
    return;
  
  vsl::SvVessel* vessel = VESSELs.value(_selected_vessel_id);
  vessel->setActive(!vessel->isActive());
  vessel->mapObject()->setActive(vessel->isActive());

  LISTITEMs.value(_selected_vessel_id)->setTextColor(vessel->isActive() ? QColor(DEFAULT_VESSEL_COLOR) : QColor(INACTIVE_VESSEL_COLOR));
  LISTITEMs.value(_selected_vessel_id)->setFont(vessel->isActive() ? _font_default : _font_inactive);
  LISTITEMs.value(_selected_vessel_id)->setIcon(vessel->isActive() ? QIcon(":/icons/Icons/bullet_white.png") : QIcon(":/icons/Icons/bullet_red.png"));
  
//  update_vessel_by_id(_selected_vessel_id);
}
