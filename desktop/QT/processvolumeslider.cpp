#include "processvolumeslider.h"
#include "ui_processvolumeslider.h"

ProcessVolumeSlider::ProcessVolumeSlider(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProcessVolumeSlider)
{
    ui->setupUi(this);
}

ProcessVolumeSlider::~ProcessVolumeSlider()
{
    delete ui;
}
