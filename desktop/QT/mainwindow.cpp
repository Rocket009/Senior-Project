#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSlider>
#include <QProgressBar>
#include <QJsonObject>
#include "processvolumeslider.h"
#ifdef WIN32
    #include "windowsaudiosessioncontroller.h"
#elif LINUX
// todo
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef WIN32
    audioController = std::shared_ptr<IAudioSessionController>(new WindowsAudioSessionController());
#elif LINUX
    audioController = std::shared_ptr<IAudioSessionController>(new IAudioSessionController);
#endif
    connect(&serial, &SerialHandler::errorOccurred, this, &MainWindow::onSerialError);
    connect(&serial, &SerialHandler::jsonReceived, this, &MainWindow::onSerialInput);
    createProcessVolumeWidgets();
    serial.connectSerial();
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
    volumeSliderLayout = layout;
    std::vector<AudioSession> sessions = audioController->getActiveAudioSessions();
    for(auto &sess : sessions)
    {

        if(sess.friendlyName.isNull() || sess.friendlyName.isEmpty())
            continue;
        ProcessVolumeSlider* s = new ProcessVolumeSlider();
        s->bindAudioSession(audioController, sess);
        connect(s, &ProcessVolumeSlider::onChange, this, &MainWindow::onProcessSliderChange);
        layout->addWidget(s);
        s->show();
        ui->scrollArea->adjustSize();
        container->adjustSize();

    }
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    container->setMinimumSize(container->sizeHint());
    container->adjustSize();
}

void MainWindow::onProcessSliderChange(ProcessVolumeSlider* s)
{
    QJsonObject j
    {
        {"Process", QJsonObject {
                {"Volume", s->getCurrentVolume()},
                {"Name", s->getProcessName()}
            }
        }
    };

    if(serial.isOpen()) serial.sendJson(j);
}

void MainWindow::onSerialError(SerialPortError e)
{
    if(e == SerialPortError::NoError)
        return;
    qWarning() << e;
}

void MainWindow::onSerialInput(const QJsonObject &json)
{
    qDebug() << json;
    if(json.contains("Process"))
    {
        QJsonObject p = json["Process"].toObject();
        QJsonValue v = p["Volume"];
        QJsonValue n = p["Name"];
        if(v.type() == QJsonValue::Double && n.type() == QJsonValue::String)
        {
            std::vector<ProcessVolumeSlider*> vec = MainWindow::getWidgetsFromLayout<ProcessVolumeSlider>(volumeSliderLayout);
            for(int i = 0; i < vec.size(); i++)
            {
                if(vec[i]->getProcessName() == n.toString())
                {
                    vec[i]->setCurrentVolume(v.toDouble());
                }
            }
        }
    }
}

