#include "processvolumeslider.h"
#include "ui_processvolumeslider.h"

ProcessVolumeSlider::ProcessVolumeSlider(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProcessVolumeSlider)
{
    ui->setupUi(this);
    connect(ui->Master_Volume_Sider, &QSlider::valueChanged, this, &ProcessVolumeSlider::volumeChanged);
    connect(ui->Master_Volume_Sider, &QSlider::valueChanged, ui->Master_Volume_Progress, &QProgressBar::setValue);
}

ProcessVolumeSlider::~ProcessVolumeSlider()
{
    delete ui;
}

void ProcessVolumeSlider::bindAudioSession(std::shared_ptr<IAudioSessionController> c, AudioSession s)
{
    controller = c;
    session = s;
    float current_volf = controller->getVolumeForProcess(s);
    int current_vol =  current_volf * 100.0f;
    ui->Master_Volume_Sider->setValue(current_vol);
    ui->ProcessNameLabel->setText(s.friendlyName);
    currentVolume = current_vol;
    emit onChange(this);
}

void ProcessVolumeSlider::volumeChanged(int value)
{
    if(currentVolume != value)
    {
        float v = value / 100.0f;
        controller->setVolumeForProcess(session, v);
        currentVolume = value;
        emit onChange(this);
    }
}

QString ProcessVolumeSlider::getProcessName()
{
    return ui->ProcessNameLabel->text();
}

int ProcessVolumeSlider::getCurrentVolume()
{
    return ui->Master_Volume_Sider->value();
}

void ProcessVolumeSlider::setCurrentVolume(int v)
{
    if(currentVolume != v)
    {
        ui->Master_Volume_Sider->setValue(v);
        currentVolume = v;
    }

}
