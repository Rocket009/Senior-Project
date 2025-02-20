#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSlider>
#include <QProgressBar>
#include "processvolumeslider.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    audioController = std::shared_ptr<AudioSessionController>(new AudioSessionController());
    createProcessVolumeWidgets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createProcessVolumeWidgets()
{
    ui->scrollArea->setWidgetResizable(false);
    QWidget *container = ui->scrollArea->widget();
    if(!container)
    {
        container = new QWidget();
        ui->scrollArea->setWidget(container);
    }
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(container->layout());
    if (!layout) {
        layout = new QHBoxLayout(container);
        container->setLayout(layout);
    }
    std::vector<AudioSession> sessions = audioController->getActiveAudioSessions();
    for(auto &sess : sessions)
    {

        ProcessVolumeSlider* s = new ProcessVolumeSlider();
        s->bindAudioSession(audioController, sess);
        layout->addWidget(s);
        s->show();
        ui->scrollArea->adjustSize();
        container->adjustSize();

    }
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    for(QObject* s : layout->children())
    {
        qDebug() << s->objectName();
    }
    container->setMinimumSize(container->sizeHint());
    container->adjustSize();
}




