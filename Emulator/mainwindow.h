#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMessageBox>

#include "sv_area.h"
#include "../../svlib/sv_settings.h"
#include "../../svlib/sv_sqlite.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  
public slots:
//  void on_bnStart_clicked();
  
private:
  Ui::MainWindow *ui;
  
  area::SvArea* _area;
  
};

#endif // MAINWINDOW_H
