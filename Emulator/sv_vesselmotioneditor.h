#ifndef SV_VESSELMOTIONEDITOR_H
#define SV_VESSELMOTIONEDITOR_H

#include <QDialog>

namespace Ui {
class SvVesselMotionEditor;
}

class SvVesselMotionEditor : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvVesselMotionEditor(QWidget *parent = 0);
  ~SvVesselMotionEditor();
  
private:
  Ui::SvVesselMotionEditor *ui;
};

#endif // SV_VESSELMOTIONEDITOR_H
