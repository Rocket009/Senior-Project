#ifndef PROCESSVOLUMESLIDER_H
#define PROCESSVOLUMESLIDER_H

#include <QWidget>

namespace Ui {
class ProcessVolumeSlider;
}

class ProcessVolumeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessVolumeSlider(QWidget *parent = nullptr);
    ~ProcessVolumeSlider();

private:
    Ui::ProcessVolumeSlider *ui;
};

#endif // PROCESSVOLUMESLIDER_H
