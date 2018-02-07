#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_bnStart_clicked()
{
  /* начальный угол поворота */
  QTime t(0,0,0);
  qsrand(t.secsTo(QTime::currentTime()));
  qreal start_angle = qrand() % 360;
  
  
  
}
