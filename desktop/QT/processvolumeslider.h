#ifndef PROCESSVOLUMESLIDER_H
#define PROCESSVOLUMESLIDER_H

#include <QWidget>
#include <QTimer>
#include "iaudiosessioncontroller.h"

namespace Ui {
class ProcessVolumeSlider;
}

class ProcessVolumeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit ProcessVolumeSlider(QWidget *parent = nullptr);
    ~ProcessVolumeSlider();
    void bindAudioSession(std::shared_ptr<IAudioSessionController> c, AudioSession s);
    QString getProcessName();
    int getCurrentVolume();
    void setCurrentVolume(int v);
signals:
    void onChange(ProcessVolumeSlider* p);
private slots:
    void volumeChanged(int value);

private:
    Ui::ProcessVolumeSlider *ui;
    AudioSession session;
    std::shared_ptr<IAudioSessionController> controller;
    int currentVolume = 0;
    QTimer *timer = nullptr;
};

#endif // PROCESSVOLUMESLIDER_H
