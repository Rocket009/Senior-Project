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

void ProcessVolumeSlider::bindAudioSession(std::shared_ptr<AudioSessionController> c, AudioSession s)
{
    controller = c;
    session = s;
    float current_volf = controller->getVolumeForProcess(s);
    int current_vol =  current_volf * 100.0f;
    ui->Master_Volume_Sider->setValue(current_vol);
    ui->ProcessNameLabel->setText(s.friendlyName);
    qDebug() << "Name: " << s.friendlyName << " current val " << current_volf;
}

void ProcessVolumeSlider::volumeChanged(int value)
{
    controller->setVolumeForProcess(session, value / 100.0f);
}
