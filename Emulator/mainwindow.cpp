#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>

extern SvSQLITE *SQLITE;
extern SvSerialEditor* SERIALEDITOR_UI;
extern SvNavtexEditor* NAVTEXEDITOR_UI;
extern SvNetworkEditor* NETWORKEDITOR_UI;

//vsl::SvVessel* SELF_VESSEL;

QMap<int, gps::SvGPS*> GPSs;
QMap<int, ais::SvAIS*> AISs;
QMap<int, vsl::SvVessel*> VESSELs;
QMap<int, QListWidgetItem*> LISTITEMs;

//extern ais::SvAIS* SELF_AIS;
//extern gps::SvGPS* SELF_GPS;
//extern QMap<int, vsl::SvVessel*> VESSELS;
extern SvVesselEditor* VESSELEDITOR_UI;
extern SvNavtexEditor* NAVTEXEDITOR_UI; 
extern QMap<quint32, ais::aisNavStat> NAVSTATs;


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  qRegisterMetaType<geo::GEOPOSITION>("geo::GEOPOSITION");
  
  ui->setupUi(this);
  
  setWindowTitle(QString("Имитатор судового оборудования v.%1").arg(APP_VERSION));
  
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
  
//  ui->dspinAISRadius->setValue(AppParams::readParam(this, "GENERAL", "AISRadius", 2).toReal());
  
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
    
    /// частоты для NAVTEX
    ui->cbNAVTEXReceiveFrequency->clear();
    QMap<int, QString> freqs = SvNavtexEditor::frequencies();
    for(int freq_id: freqs.keys())
      ui->cbNAVTEXReceiveFrequency->addItem(freqs.value(freq_id), freq_id);
    
    /// тревога
    ui->cbLAGAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
    ui->cbAISAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
    ui->cbNAVTEXAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
    
    /// типы сообщений LAG
    ui->cbLAGMessageType->clear();
    QMap<lag::MessageType, QString> mtypes = lag::SvLAG::msgtypes();
    for(lag::MessageType mtype: mtypes.keys())
      ui->cbLAGMessageType->addItem(mtypes.value(mtype), mtype);
    
    
    /// параметры устройств
    read_devices_params();    
    
    
    qInfo() << "init 1";
    
    
    /*! необходима проверка, что в таблице vessels существует запись с флагом sef == true !*/
SV:
  
    /** ------ читаем информацию о собственном судне --------- **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(true),_query).type())
      _exception.raise(_query->lastError().databaseText());
    
    if(!_query->next()) {
      
      QMessageBox::warning(this, "", "В БД нет сведений о собственном судне", QMessageBox::Ok);
      _query->finish();
      
      VESSELEDITOR_UI = new SvVesselEditor(this, -1, true);
      
      int result = VESSELEDITOR_UI->exec();

      if(QDialog::Accepted == result && 
         !VESSELEDITOR_UI->last_error().isEmpty()) {
        
        _exception.raise(VESSELEDITOR_UI->last_error());
              
      }
      else if(QDialog::Accepted == result) {
        
        goto SV;    
      }
        
      _exception.raise("Невозможно продолжить без собственного судна");
      
    }
  
    
    qInfo() << "init 2";
    
    /** --------- создаем собственное судно ----------- **/
    createSelfVessel(_query);
    
    _query->finish();

    _self_vessel->updateVessel();
  
    qInfo() << "init 3";
   
    
    /** ------ читаем список судов --------- **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(false), _query).type())
      _exception.raise(_query->lastError().databaseText());
    
    /** --------- создаем суда ----------- **/
    while(_query->next()) {
      vsl::SvVessel* vessel = createOtherVessel(_query);
      vessel->updateVessel();
      
    }
    _query->finish();
  
    qInfo() << "init 4";
  
    
    /*! необходима проверка, что в таблице navtex существует запись  !*/
NX:
    /** ------ читаем данные станции НАВТЕКС --------- **/
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX), _query).type())
      _exception.raise(_query->lastError().databaseText());
    
    if(!_query->next()) {
      
      QMessageBox::warning(this, "", "В БД нет сведений о станции NAVTEX", QMessageBox::Ok);
      _query->finish();
      
      NAVTEXEDITOR_UI = new SvNavtexEditor(this, -1);
      if(QDialog::Accepted == NAVTEXEDITOR_UI->exec()) {
        
        goto NX;    
              
      }
      else if(!NAVTEXEDITOR_UI->last_error().isEmpty()) {
        
        _exception.raise(NAVTEXEDITOR_UI->last_error());

      }
      
      _exception.raise("Невозможно продолжить работу без станции НАВТЕКС");
      
    }
    
    /** --------- создаем станцию НАВТЕКС ----------- **/
    createNavtex(_query);
    
    _query->finish();
    
    
    connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(area_selection_changed()));
    connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
    
    connect(this, SIGNAL(newState(States)), this, SLOT(stateChanged(States)));
   
    connect(this, &MainWindow::new_lag_message_type, _self_lag, &lag::SvLAG::setMessageType);
    
    _timer_x10.setSingleShot(true);
    connect(&_timer_x10, &QTimer::timeout, this, &MainWindow::setX10Emulation);
    
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

    
    return true;
  }
  
  catch(SvException &exception) {
    qInfo() << exception.err;
    if(_query) delete _query;
    if(_area) delete _area;
    
    if(VESSELEDITOR_UI) delete VESSELEDITOR_UI;
    if(NAVTEXEDITOR_UI) delete NAVTEXEDITOR_UI;

    // удаляются также все устройства ais, gps и lag
    if(VESSELs.count()) {
      foreach (int key, VESSELs.keys()) {
        delete VESSELs.take(key);
      }
    }
    
    QMessageBox::critical(this, "Ошибка", QString("Ошибка инициализации:\n%1").arg(exception.err), QMessageBox::Ok);
    
    return false;
  }
}

MainWindow::~MainWindow()
{
  if(_current_state == sRunned)
    on_bnCycle_clicked();
  
  while(_current_state != sStopped)
    qApp->processEvents();
  
  save_devices_params();
  
  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
  AppParams::saveWindowParams(ui->dockGraphics, ui->dockGraphics->size(), ui->dockGraphics->pos(), ui->dockGraphics->windowState(), "AREA WINDOW");
  AppParams::saveWindowParams(ui->dockCarrentInfo, ui->dockCarrentInfo->size(), ui->dockCarrentInfo->pos(), ui->dockCarrentInfo->windowState(), "INFO WINDOW");
  
//  AppParams::saveParam(this, "GENERAL", "AISRadius", QVariant(ui->dspinAISRadius->value()));
  
  delete ui;
}

gps::gpsInitParams MainWindow::readGPSInitParams(QSqlQuery* q, ais::aisDynamicData& dynamic_data, QDateTime lastUpdate)
{
  gps::gpsInitParams result;
//  QDateTime dt = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел

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
    
    geo::COORDINATES coord = geo::get_rnd_coordinates(_area->bounds(), lastUpdate.time().second() * 1000 + lastUpdate.time().msec());
    
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
      
      ui->checkAISEnabled->setEnabled(false);
      ui->checkLAGEnabled->setEnabled(false);
      ui->checkNAVTEXEnabled->setEnabled(false);
      ui->bnAISEditSerialParams->setEnabled(false);
      ui->bnLAGEditSerialParams->setEnabled(false);
      ui->bnNAVTEKEditSerialParams->setEnabled(false);
      
      ui->dspinAISRadius->setEnabled(false);
      ui->bnAddVessel->setEnabled(false);
      ui->bnEditVessel->setEnabled(false);
      ui->bnRemoveVessel->setEnabled(false);
      ui->bnSetActive->setEnabled(false);
      
      ui->spinLAGUploadInterval->setEnabled(false);
      ui->cbLAGMessageType->setEnabled(false);
      
      ui->spinNAVTEXUploadInterval->setEnabled(false);
      ui->cbNAVTEXReceiveFrequency->setEnabled(false);
      
      ui->bnCycle->setEnabled(true);
      
      ui->bnCycle->setIcon(QIcon());
      ui->bnCycle->setText("Стоп");
      ui->bnCycle->setStyleSheet("background-color: tomato");
      
      break;
    }
      
    case sStopped:
    {
      ui->tabWidget->setEnabled(true);
      
      ui->checkAISEnabled->setEnabled(true);
      ui->checkLAGEnabled->setEnabled(true);
      ui->checkNAVTEXEnabled->setEnabled(true);
      ui->bnAISEditSerialParams->setEnabled(true);
      ui->bnLAGEditSerialParams->setEnabled(true);
      ui->bnNAVTEKEditSerialParams->setEnabled(true);
      
      ui->dspinAISRadius->setEnabled(true);
      ui->bnAddVessel->setEnabled(true);
      ui->bnEditVessel->setEnabled(true);
      ui->bnRemoveVessel->setEnabled(true);
      ui->bnSetActive->setEnabled(true);
      
      ui->spinLAGUploadInterval->setEnabled(true);
      ui->cbLAGMessageType->setEnabled(true);
      
      ui->spinNAVTEXUploadInterval->setEnabled(true);
      ui->cbNAVTEXReceiveFrequency->setEnabled(true);
      
      ui->bnCycle->setEnabled(true);
      ui->bnCycle->setIcon(QIcon(":/icons/Icons/start.ico"));
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
        
        /// сохраняем параметры устройств
        save_devices_params();
        
        /** открываем порты устройств **/
        /// LAG
        if(ui->checkLAGEnabled->isChecked()) {
          
          _self_lag->setSerialPortParams(_lag_serial_params);
          _self_lag->setMessageType(lag::MessageType(ui->cbLAGMessageType->currentData().toInt()));
          if(!_self_lag->open()) _exception.raise(QString("ЛАГ: %1").arg(_self_lag->lastError()));
        }
        
        /// AIS
        if(ui->checkAISEnabled->isChecked()) {
         
          _self_ais->setSerialPortParams(_ais_serial_params);
          _self_ais->setReceiveRange(ui->dspinAISRadius->value());
          if(!_self_ais->open()) _exception.raise(QString("АИС: %1").arg(_self_ais->lastError()));
          
        }
        
        /// NAVTEX
        if(ui->checkNAVTEXEnabled->isChecked()) {
         
          _navtex->setSerialPortParams(_navtex_serial_params);
          _navtex->setReceiveFrequency(ui->cbNAVTEXReceiveFrequency->currentData().toUInt());

          if(!_navtex->open()) _exception.raise(QString("НАВТЕКС: %1").arg(_navtex->lastError()));
          
        }
        
      }
      
      catch(SvException &e) {
        if(_self_ais->isOpened()) _self_ais->close();
        if(_self_lag->isOpened()) _self_lag->close();
        if(_navtex->isOpened()) _navtex->close();
        
        QMessageBox::critical(this, "Ошибка", QString("Ошибка открытия порта.\n%1").arg(e.err), QMessageBox::Ok);
        emit newState(sStopped);
        return;
      }
      
//      emit setMultiplier(ui->spinEmulationMultiplier->value());
      
      emit startGPSEmulation(0);
      
      emit startAISEmulation(0);
      
      emit startLAGEmulation(ui->spinLAGUploadInterval->value());
      
      emit startNAVTEXEmulation(ui->spinNAVTEXUploadInterval->value());
      
      emit newState(sRunned);
      
      break;
    }
      
    case sRunned:
    {
      emit newState(sStopping);
      
      emit stopNAVTEXEmulation();
      
      emit stopLAGEmulation();
      
      emit stopAISEmulation();
      
      emit stopGPSEmulation();
      
      /// сохраняем последние геопозиции 
      foreach (gps::SvGPS* curgps, GPSs.values()) {
        curgps->waitWhileRunned();
        
        updateGPSInitParams(curgps);
      }
      
      /** закрываем порты **/
      if(ui->checkLAGEnabled->isChecked()) _self_lag->close();
      if(ui->checkAISEnabled->isChecked()) _self_ais->close();
      if(ui->checkNAVTEXEnabled->isChecked()) _navtex->close();
      
      emit newState(sStopped);
      
      emit setMultiplier(1);
          
      break;
    }
      
    default:
      break;
  }
  
}

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
//    _area->setLabelInfo("");
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
    
  ui->listVessels->setCurrentItem(LISTITEMs.value(_selected_vessel_id));
  
  bool b = ui->listVessels->currentRow() > -1;
  ui->actionEditVessel->setEnabled(b); 
  ui->actionRemoveVessel->setEnabled(b && (_selected_vessel_id != _self_vessel->id)); 
  ui->bnRemoveVessel->setEnabled(b && (_selected_vessel_id != _self_vessel->id));
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
  QDateTime last_update = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
  
  ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
  ais::aisDynamicData dynamic_data = readAISDynamicData(q);
  gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, last_update);
  ais::aisNavStat nav_stat = readNavStat(q);
  
  
  /** ----- создаем устройства ------ **/
  // GPS
  _self_gps = new gps::SvGPS(vessel_id, gps_params, _area->bounds(), last_update);
  GPSs.insert(vessel_id, _self_gps);
  
  // АИС
  _self_ais = new ais::SvSelfAIS(vessel_id, static_voyage_data, dynamic_data, log, last_update);
  _self_ais->setReceiveRange(ui->dspinAISRadius->value()); /** надо переделать */
  AISs.insert(vessel_id, _self_ais);
  _self_ais->setNavStatus(nav_stat);
  
  // LAG
  _self_lag = new lag::SvLAG(vessel_id, dynamic_data.geoposition, log);

  // эхолот
  _self_multi_echo = new ech::SvECHO(vessel_id, dynamic_data.geoposition, log);
  
  
  /** --------- создаем объект собственного судна -------------- **/
  _self_vessel = new vsl::SvVessel(this, vessel_id);
  VESSELs.insert(vessel_id, _self_vessel);
  
  _self_vessel->mountGPS(_self_gps);
  _self_vessel->mountAIS(_self_ais);
  _self_vessel->mountLAG(_self_lag);
  _self_vessel->mountECHO(_self_multi_echo);
  
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
  QDateTime last_update = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
  
  ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
  ais::aisDynamicData dynamic_data = readAISDynamicData(q);
  gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, last_update);
  ais::aisNavStat nav_stat = readNavStat(q);                     
  
  /** ----- создаем устройства ------ **/
  // GPS
  gps::SvGPS* newGPS = new gps::SvGPS(vessel_id, gps_params, _area->bounds(), last_update);
  GPSs.insert(vessel_id, newGPS);
 
  
  // АИС
  ais::SvOtherAIS* newAIS;
  newAIS = new ais::SvOtherAIS(vessel_id, static_voyage_data, dynamic_data, last_update);
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

nav::SvNAVTEX* MainWindow::createNavtex(QSqlQuery* q)
{  
  /** ----- создаем устройство ------ **/
  int id = q->value("id").toUInt();
  
  _navtex = new nav::SvNAVTEX(log, id);
  
  nav::navtexData ndata;
  ndata.region_id = q->value("station_region_id").toUInt();
  ndata.message_id = q->value("message_id").toUInt();
  ndata.message_designation = q->value("message_designation").toString();
  ndata.region_station_name = q->value("region_station_name").toString();
  ndata.message_text = q->value("message_text").toString();
//  ndata.region_country = q->value("region_country").toString();
  ndata.message_letter_id = q->value("message_letter_id").toString();
  ndata.region_letter_id = q->value("region_letter_id").toString();
  ndata.message_last_number = q->value("message_last_number").toUInt();
  ndata.transmit_frequency_id = q->value("transmit_frequency").toUInt();
  ndata.transmit_frequency = SvNavtexEditor::frequencies().value(ndata.transmit_frequency_id);
  
  _navtex->setData(ndata);
  
  update_NAVTEX_data();
  
  connect(this, &MainWindow::startNAVTEXEmulation, _navtex, &nav::SvNAVTEX::start);
  connect(this, &MainWindow::stopNAVTEXEmulation, _navtex, &nav::SvNAVTEX::stop);
  
  return _navtex;
  
}

void MainWindow::update_vessel_by_id(int id)
{
  foreach (vsl::SvVessel* vessel, VESSELs.values()) {
    
    if(vessel->id != id) continue;
    
    if(_self_ais->distanceTo(vessel->ais()) > _self_ais->receiveRange() * 1000) {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(true);
  
      LISTITEMs.value(id)->setIcon(QIcon(":/icons/Icons/link-broken1.ico"));
      LISTITEMs.value(id)->setFont(_font_nolink);
      LISTITEMs.value(id)->setTextColor(QColor(OUTDATED_VESSEL_COLOR));
      
    }
    else {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(false);
      
      LISTITEMs.value(id)->setIcon(QIcon(":/icons/Icons/link3.ico"));
      LISTITEMs.value(id)->setFont(_font_default);
      LISTITEMs.value(id)->setTextColor(QColor(DEFAULT_VESSEL_PEN_COLOR));
      
      vessel->updateVessel();
    }  
    
    
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
    QDateTime last_update = q->value("gps_last_update").toDateTime();
    ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
    ais::aisDynamicData *dynamic_data = VESSELs.value(id)->ais()->dynamicData();
    gps::gpsInitParams gps_params = readGPSInitParams(q, *dynamic_data, last_update);
    q->finish();
    
//    dynamic_data.geoposition = VESSELs.value(id)->ais()->getDynamicData()->geoposition;
    
    vsl::SvVessel* vessel = VESSELs.value(id);
    
    vessel->ais()->setStaticVoyageData(static_voyage_data);
    vessel->gps()->setInitParams(gps_params);
    
    vessel->updateVessel();
    
    LISTITEMs.value(id)->setText(QString("%1\t%2").arg(id).arg(static_voyage_data.name));
  }
}

void MainWindow::update_NAVTEX_data()
{
  ui->textNAVTEXParams->setHtml(QString("<!DOCTYPE html><p><strong>Станция:</strong>\t%1</p>" \
                                        "<p><strong>Тип сообщения:</strong>\t%2</p>" \
                                        "<p><strong>Заголовок сообщения:</strong>\t%3%4%5</p>" \
                                        "<p><strong>Частота передачи:</strong>\t%6</p>" \
                                        "<p><strong>Сообщение:</strong></p>" \
                                        "<p>%7</p>")
                                .arg(_navtex->data()->region_station_name)
                                .arg(_navtex->data()->message_designation)
                                .arg(_navtex->data()->region_letter_id)
                                .arg(_navtex->data()->message_letter_id)
                                .arg(QString("%1").arg(_navtex->data()->message_last_number, 2, 10).replace(" ", "0"))
                                .arg(_navtex->data()->transmit_frequency)
                                .arg(_navtex->data()->message_text));
  
}

void MainWindow::on_bnAISEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_ais_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
    
      return;
    }
    
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

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
    
      return;
    }
    
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
  SERIALEDITOR_UI = new SvSerialEditor(_navtex_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
        
      return;
    }
    
  }
  
  _navtex_serial_params.name = SERIALEDITOR_UI->params.name;
  _navtex_serial_params.description = SERIALEDITOR_UI->params.description;
  _navtex_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _navtex_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _navtex_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _navtex_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _navtex_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;

  ui->editNAVTEXSerialInterface->setText(_navtex_serial_params.description);

}

void MainWindow::on_bnECHOEditSerialParams_clicked()
{
  NETWORKEDITOR_UI = new SvNetworkEditor(_echo_network_params, this);
  if(NETWORKEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NETWORKEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NETWORKEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NETWORKEDITOR_UI;
        
      return;
    }
    
  }
  
  _echo_network_params.ifc = NETWORKEDITOR_UI->params.ifc;
  _echo_network_params.protocol = NETWORKEDITOR_UI->params.protocol;
  _echo_network_params.ip = NETWORKEDITOR_UI->params.ip;
  _echo_network_params.port = NETWORKEDITOR_UI->params.port;
  
  delete NETWORKEDITOR_UI;

  ui->editECHOInterface->setText(_echo_network_params.description);
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
  switch (_current_state) {
    case sStopped:
      
      editVessel(LISTITEMs.key(ui->listVessels->item(index.row())));
      break;
      
    case sRunned: {
      
      SvMapObject* selobj = nullptr;
      foreach (SvMapObject* obj, _area->scene->mapObjects()) {
        if(obj->id() != LISTITEMs.key(ui->listVessels->item(index.row())))
          continue;
        
        selobj = obj;
        break;
      }
      
      if(selobj)      
        _area->centerSelected();
      
      break;
      
    }
  }
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

void MainWindow::read_devices_params()
{
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_FROM_DEVICES_PARAMS), _query).type()) 
    _exception.raise(_query->lastError().databaseText());

  while(_query->next()) {    
    
    idev::SvSimulatedDeviceTypes dt = idev::SvSimulatedDeviceTypes(_query->value("device_type").toUInt());
    
    switch (dt) {
      case idev::sdtLAG: {
        
        _lag_serial_params.name =         _query->value("port_name").toString();        
        _lag_serial_params.description =  _query->value("description").toString();
        _lag_serial_params.baudrate =     _query->value("baudrate").toInt();                              
        _lag_serial_params.databits =     QSerialPort::DataBits(_query->value("data_bits").toInt());      
        _lag_serial_params.flowcontrol =  QSerialPort::FlowControl(_query->value("flow_control").toInt());
        _lag_serial_params.parity =       QSerialPort::Parity(_query->value("parity").toInt());           
        _lag_serial_params.stopbits =     QSerialPort::StopBits(_query->value("stop_bits").toInt());  
        ui->editLAGSerialInterface->setText(_lag_serial_params.description);
        
        ui->checkLAGEnabled->setChecked(_query->value("is_active").toBool());
        ui->spinLAGUploadInterval->setValue(_query->value("upload_interval").toUInt());
        
        ui->spinLAGAlarmId->setValue(_query->value("alarm_id").toUInt());
        ui->cbLAGAlarmState->setCurrentIndex(_query->value("alarm_state").toUInt());
        ui->editLAGAlarmMessageText->setText(_query->value("alarm_text").toString());
              
        QVariant args = parse_args(_query->value("args").toString(), ARG_LAG_MSGTYPE);
        quint32 msgtype = args.canConvert<quint32>() ? args.toUInt() : 0;
        ui->cbLAGMessageType->setCurrentIndex(ui->cbLAGMessageType->findData(msgtype < ui->cbLAGMessageType->count() ? msgtype : lag::lmtVBW));
        
        break;
      }
        
      case idev::sdtSelfAIS: {
        
        _ais_serial_params.name =         _query->value("port_name").toString();
        _ais_serial_params.description =  _query->value("description").toString();
        _ais_serial_params.baudrate =     _query->value("baudrate").toInt();
        _ais_serial_params.databits =     QSerialPort::DataBits(_query->value("data_bits").toInt());
        _ais_serial_params.flowcontrol =  QSerialPort::FlowControl(_query->value("flow_control").toInt());
        _ais_serial_params.parity =       QSerialPort::Parity(_query->value("parity").toInt());
        _ais_serial_params.stopbits =     QSerialPort::StopBits(_query->value("stop_bits").toInt());
        ui->editAISSerialInterface->setText(_ais_serial_params.description);
        
        ui->checkAISEnabled->setChecked(_query->value("is_active").toBool());
        
        ui->spinAISAlarmId->setValue(_query->value("alarm_id").toUInt());
        ui->cbAISAlarmState->setCurrentIndex(_query->value("alarm_state").toUInt());
        ui->editAISAlarmMessageText->setText(_query->value("alarm_text").toString());
        
        QVariant args = parse_args(_query->value("args").toString(), ARG_AIS_RECEIVERANGE);
        ui->dspinAISRadius->setValue(args.canConvert<qreal>() ? args.toReal() : 50);
        
        
      }
        
      case idev::sdtNavtex: {
         
        _navtex_serial_params.name =        _query->value("port_name").toString(); 
        _navtex_serial_params.description = _query->value("description").toString();
        _navtex_serial_params.baudrate =    _query->value("baudrate").toInt();                              
        _navtex_serial_params.databits =    QSerialPort::DataBits(_query->value("data_bits").toInt());      
        _navtex_serial_params.flowcontrol = QSerialPort::FlowControl(_query->value("flow_control").toInt());
        _navtex_serial_params.parity =      QSerialPort::Parity(_query->value("parity").toInt());           
        _navtex_serial_params.stopbits =    QSerialPort::StopBits(_query->value("stop_bits").toInt());      
        ui->editNAVTEXSerialInterface->setText(_navtex_serial_params.description);
        
        ui->checkNAVTEXEnabled->setChecked(_query->value("is_active").toBool());
        ui->spinNAVTEXUploadInterval->setValue(_query->value("upload_interval").toUInt());
        
        ui->spinNAVTEXAlarmId->setValue(_query->value("alarm_id").toUInt());
        ui->cbNAVTEXAlarmState->setCurrentIndex(_query->value("alarm_state").toUInt());
        ui->editNAVTEXAlarmMessageText->setText(_query->value("alarm_text").toString());
                
        QVariant args = parse_args(_query->value("args").toString(), ARG_NAV_RECV_FREQ);
        int indx = ui->cbNAVTEXReceiveFrequency->findData(args.canConvert<quint32>() ? args.toUInt() : 1);
        ui->cbNAVTEXReceiveFrequency->setCurrentIndex(indx < ui->cbNAVTEXReceiveFrequency->count() ? indx : 1);
        
      }
        
      case idev::sdtEcho: {
        
        QVariant args;
        
        _echo_network_params.ifc = _query->value("network_interface").toUInt();
        _echo_network_params.protocol = _query->value("network_protocol").toUInt();
        _echo_network_params.ip = _query->value("network_ip").toUInt();
        _echo_network_params.port = _query->value("network_port").toUInt();
        _echo_network_params.description = _query->value("description").toString();
        ui->editECHOInterface->setText(_echo_network_params.description);
        
        ui->spinECHOAlarmId->setValue(_query->value("alarm_id").toUInt());
        ui->cbECHOAlarmState->setCurrentIndex(_query->value("alarm_state").toUInt());
        ui->editECHOAlarmMessageText->setText(_query->value("alarm_text").toString());
        
        args = parse_args(_query->value("args").toString(), ARG_ECHO_EMIT_COUNT);
        ui->spinECHOEmittersCount->setValue(args.canConvert<uint>() ? args.toUInt() : 28);

      }
        
      default:
        break;
    }
    
    
    
  }
  _query->finish();
  
}

void MainWindow::save_devices_params()
{
  QSqlError err;
  
  /// ----------- LAG ------------- ///
  try {

    err = check_params_exists(idev::sdtLAG);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkLAGEnabled->isChecked())
                          .arg(ui->spinLAGUploadInterval->value())
                          .arg(QString("-%1 %2").arg(ARG_LAG_MSGTYPE).arg(ui->cbLAGMessageType->currentIndex()))
                          .arg(ui->spinLAGAlarmId->value())
                          .arg(ui->cbLAGAlarmState->currentIndex())
                          .arg(ui->editLAGAlarmMessageText->text())
                          .arg(idev::sdtLAG));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров ЛАГ:\n%1").arg(e.err) << svlog::endl;
  }
   
  /// ----------- AIS ------------- ///
  try {
    
    err = check_params_exists(idev::sdtSelfAIS);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkAISEnabled->isChecked())
                          .arg(1000)
                          .arg(QString("-%1 %2").arg(ARG_AIS_RECEIVERANGE).arg(ui->dspinAISRadius->value(), 0, 'f', 1))
                          .arg(ui->spinAISAlarmId->value())
                          .arg(ui->cbAISAlarmState->currentIndex())
                          .arg(ui->editAISAlarmMessageText->text())
                          .arg(idev::sdtSelfAIS));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
   
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров АИС:\n%1").arg(e.err) << svlog::endl;
  }
    
  /// ----------- NAVTEX ------------- ///
  try {
    
    err = check_params_exists(idev::sdtNavtex);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkNAVTEXEnabled->isChecked())
                          .arg(ui->spinNAVTEXUploadInterval->value())
                          .arg(QString("-%1 %2").arg(ARG_NAV_RECV_FREQ).arg(ui->cbNAVTEXReceiveFrequency->currentData().toUInt()))
                          .arg(ui->spinNAVTEXAlarmId->value())
                          .arg(ui->cbNAVTEXAlarmState->currentIndex())
                          .arg(ui->editNAVTEXAlarmMessageText->text())
                          .arg(idev::sdtNavtex));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров НАВТЕКС:\n%1").arg(e.err) << svlog::endl;
  }
    
}

QVariant MainWindow::parse_args(QString args, QString arg)
{
  //! обязателен первый аргумент!! парсер считает, что там находится путь к программе
  QStringList arg_list;
  arg_list << "dumb_path_to_app" << args.split(" ");
  
  QCommandLineParser parser;
  parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
  
  parser.addOption(QCommandLineOption(ARG_LAG_MSGTYPE, "Тип сообщения для LAG", "0", "0"));
  parser.addOption(QCommandLineOption(ARG_AIS_RECEIVERANGE, "Дальность приема для AIS в км", "50", "50"));
  parser.addOption(QCommandLineOption(ARG_NAV_RECV_FREQ, "Частота приемника NAVTEX", "1", "1"));
  
  parser.addOption(QCommandLineOption(ARG_ECHO_INTERFACE, "Сетевой интерфейс для эхолота", "", ""));
  parser.addOption(QCommandLineOption(ARG_ECHO_PROTOCOL , "Протокол для эхолота", "UDP", "UDP"));
  parser.addOption(QCommandLineOption(ARG_ECHO_IP, "IP для эхолота", "127.0.0.1", "127.0.0.1"));
  parser.addOption(QCommandLineOption(ARG_ECHO_PORT, "Порт для эхолота", "35580", "35580"));
  parser.addOption(QCommandLineOption(ARG_ECHO_EMIT_COUNT, "Кол-во излучателей для эхолота", "10", "10"));
  
  QVariant result = QVariant();
  if (parser.parse(arg_list)) {
    
    result = QVariant::fromValue(parser.value(arg));
    
  }
  
  return result;
}

void MainWindow::on_bnEditNAVTEX_clicked()
{
  NAVTEXEDITOR_UI = new SvNavtexEditor(this, _navtex->id());
  if(NAVTEXEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NAVTEXEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NAVTEXEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NAVTEXEDITOR_UI;
        
      return;
    }
    
  }
  
  nav::navtexData data;
//  memcpy(&data, _navtex->data(), sizeof(nav::navtexData));
  
  data.message_id = NAVTEXEDITOR_UI->t_message_id;
  data.region_id = NAVTEXEDITOR_UI->t_region_id;
  data.message_text = NAVTEXEDITOR_UI->t_message_text;
  data.message_designation = NAVTEXEDITOR_UI->t_message_designation;
  data.message_letter_id = NAVTEXEDITOR_UI->t_message_letter_id;
  data.region_letter_id = NAVTEXEDITOR_UI->t_region_letter_id;
  data.region_station_name = NAVTEXEDITOR_UI->t_region_station_name;
  data.transmit_frequency = NAVTEXEDITOR_UI->t_transmit_frequency;
  data.transmit_frequency_id = NAVTEXEDITOR_UI->t_transmit_frequency_id;
  data.message_last_number = NAVTEXEDITOR_UI->t_message_last_number;
  
  delete NAVTEXEDITOR_UI;

  _navtex->setData(data);
  
  update_NAVTEX_data();
  
}

void MainWindow::on_bnNAVTEXAlarmSend_clicked()
{
  _navtex->alarm(ui->spinNAVTEXAlarmId->value(),
                 ui->cbNAVTEXAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editNAVTEXAlarmMessageText->text().left(62));
}

void MainWindow::on_bnLAGAlarmSend_clicked()
{
  _self_lag->alarm(ui->spinLAGAlarmId->value(),
                 ui->cbLAGAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editLAGAlarmMessageText->text().left(62));    
}

void MainWindow::on_bnAISAlarmSend_clicked()
{
  _self_ais->alarm(ui->spinAISAlarmId->value(),
                 ui->cbAISAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editAISAlarmMessageText->text().left(62));
}

void MainWindow::on_cbLAGMessageType_currentIndexChanged(int index)
{
  emit new_lag_message_type(lag::MessageType(ui->cbLAGMessageType->currentData().toInt()));
}

void MainWindow::setX10Emulation()
{
  emit setMultiplier(10);
  ui->bnCycle->setStyleSheet("background-color: rgb(85, 170, 255);");
  ui->bnCycle->setText("Старт х10");
  update();
}

void MainWindow::on_bnCycle_pressed()
{
    _timer_x10.start(2000);
}

void MainWindow::on_bnCycle_released()
{
    _timer_x10.stop();
}

void MainWindow::updateGPSInitParams(gps::SvGPS* g)
{
  QString upd = "";
  gps::gpsInitParams params = g->initParams();
  
  if(!params.init_random_coordinates) {
    
    upd.append(QString("dynamic_latitude=%1,dynamic_longtitude=%2,")
                         .arg(g->currentGeoposition()->latitude)
                         .arg(g->currentGeoposition()->longtitude));
    
    params.geoposition.latitude = g->currentGeoposition()->latitude;
    params.geoposition.longtitude = g->currentGeoposition()->longtitude;
    
  }
  
  if(!params.init_random_course) {
    upd.append(QString("dynamic_course=%1,").arg(g->currentGeoposition()->course));
    params.geoposition.course = g->currentGeoposition()->course;
  }
  
  if(!params.init_random_speed) {
    upd.append(QString("dynamic_speed=%1,").arg(g->currentGeoposition()->speed));
    params.geoposition.speed = g->currentGeoposition()->speed;
  }        
  
  if(!params.init_random_pitch) {
    upd.append(QString("dynamic_pitch=%1,").arg(g->currentGeoposition()->pitch));
    params.geoposition.pitch = g->currentGeoposition()->pitch;
  }
  
  if(!params.init_random_roll) {
    upd.append(QString("dynamic_roll=%1,").arg(g->currentGeoposition()->roll));
    params.geoposition.roll = g->currentGeoposition()->roll;
  }
  
  if(upd.isEmpty()) return;
  
  upd.chop(1);
  QString sql = QString("UPDATE ais SET %1 where vessel_id=%2").arg(upd)
                .arg(g->vesselId());
  
  QSqlError e = SQLITE->execSQL(sql);
  
  if(e.type() != QSqlError::NoError) {
    log << svlog::Error << svlog::Time << e.text() << svlog::endl;
    return;
  
  }
  
  g->setInitParams(params);
  
  return;
}

void MainWindow::on_bnDropDynamicData_clicked()
{
  int msgbtn = QMessageBox::question(0, "Подтверждение", "Текущие данные о местоположении судна(ов) будут сброшены.\nДля выбранного судна - \"Да\"\n"
                           "Для всех судов - \"Да для всех\".\n"
                           "Вы уверены?", QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No);
  
  if(!QList<int>({QMessageBox::Yes, QMessageBox::YesToAll}).contains(msgbtn)) return;

  QString sql;
  switch (msgbtn) {
    case QMessageBox::Yes: 
      
      sql = QString("UPDATE ais SET dynamic_latitude=NULL, "
                    "dynamic_longtitude=NULL, "
                    "dynamic_course=NULL, dynamic_speed=NULL, "
                    "dynamic_pitch=NULL, dynamic_roll=NULL "
                    "where vessel_id=%1").arg(_selected_vessel_id);
      break;
      
    case QMessageBox::YesToAll: 
      
      sql = "UPDATE ais SET dynamic_latitude=NULL, "
            "dynamic_longtitude=NULL, "
            "dynamic_course=NULL, dynamic_speed=NULL, "
            "dynamic_pitch=NULL, dynamic_roll=NULL";
      break;
  }
  
  try {
    
    QSqlError e = SQLITE->execSQL(sql);
    if(e.type() != QSqlError::NoError) _exception.raise(e.text());
    
    
    // читаем информацию из БД  
    /// ------ читаем список судов --------- ///
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS), _query).type())
      _exception.raise(_query->lastError().databaseText());
    
    /** --------- читаем данные судна ----------- **/
    while(_query->next()) {
      
      int vessel_id = _query->value("id").toUInt();
      QDateTime last_update = _query->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
      
      ais::aisDynamicData dynamic_data = readAISDynamicData(_query);
      gps::gpsInitParams gps_params = readGPSInitParams(_query, dynamic_data, last_update);
      
      AISs.value(vessel_id)->setDynamicData(dynamic_data);
      GPSs.value(vessel_id)->setInitParams(gps_params);
      
//      update_vessel_by_id(vessel_id);
      
      VESSELs.value(vessel_id)->updateVessel();
      
    }
    _query->finish();
    
  }
  
  catch(SvException &e) {
    log << svlog::Critical << svlog::Time << e.err << svlog::endl;
  }
}

