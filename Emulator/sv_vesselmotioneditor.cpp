#include "sv_vesselmotioneditor.h"
#include "ui_sv_vesselmotioneditor.h"

SvVesselMotionEditor::SvVesselMotionEditor(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvVesselMotionEditor)
{
  ui->setupUi(this);
}

SvVesselMotionEditor::~SvVesselMotionEditor()
{
  delete ui;
}
