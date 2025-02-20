#ifndef PROCESSVOLUMESLIDER_H
#define PROCESSVOLUMESLIDER_H

#include <QWidget>
#include "audiosessioncontroller.h"

namespace Ui {
class ProcessVolumeSlider;
}

class ProcessVolumeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessVolumeSlider(QWidget *parent = nullptr);
    ~ProcessVolumeSlider();
    void bindAudioSession(std::shared_ptr<AudioSessionController> c, AudioSession s);
private slots:
    void volumeChanged(int value);

private:
    Ui::ProcessVolumeSlider *ui;
    AudioSession session;
    std::shared_ptr<AudioSessionController> controller;
};

#endif // PROCESSVOLUMESLIDER_H
