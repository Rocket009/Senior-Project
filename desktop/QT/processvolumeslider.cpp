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
    emit onChange(this);
}

void ProcessVolumeSlider::volumeChanged(int value)
{
    float v = value / 100.0f;
    controller->setVolumeForProcess(session, v);
    emit onChange(this);
}

QString ProcessVolumeSlider::getProcessName()
{
    return ui->ProcessNameLabel->text();
}

float ProcessVolumeSlider::getCurrentVolume()
{
    return ui->Master_Volume_Sider->value() / 100.0f;
}

void ProcessVolumeSlider::setCurrentVolume(float v)
{
    int vol = v * 100;
    ui->Master_Volume_Sider->setValue(vol);
}
