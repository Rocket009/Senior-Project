#include "mainwindow.h"
#include "windowsaudiosessioncontroller.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WindowsAudioSessionController c;
    std::vector<AudioSession> se = c.getActiveAudioSessions();
    qDebug() << "Active audio sessions";
    for (const auto &session : se)
    {
        qDebug() << "PID:" << session.processId;
        qDebug() << "Name:" << session.friendlyName;
        qDebug() << "Icon:" << session.iconPath;
    }
    // c.setVolumeForProcess(8724, 0.5f);
    MainWindow w;
    w.show();
    return a.exec();
}
