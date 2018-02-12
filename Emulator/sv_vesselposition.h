#ifndef SV_VESSELPOSITION_H
#define SV_VESSELPOSITION_H

#include <QDialog>

namespace Ui {
class SvVesselPositionDialog;
}

class SvVesselPosition : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvVesselPosition(QWidget *parent = 0);
  ~SvVesselPosition();
  
private:
  Ui::SvVesselPositionDialog *ui;
};

#endif // SV_VESSELPOSITION_H
