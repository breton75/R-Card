#include "sv_vesselposition.h"
#include "ui_sv_vesselposition.h"

SvVesselPosition::SvVesselPosition(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvVesselPositionDialog)
{
  ui->setupUi(this);
}

SvVesselPosition::~SvVesselPosition()
{
  delete ui;
}
