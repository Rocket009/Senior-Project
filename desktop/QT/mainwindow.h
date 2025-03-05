#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "iaudiosessioncontroller.h"
#include "processvolumeslider.h"
#include "serialhandler.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    void createProcessVolumeWidgets();
    std::shared_ptr<IAudioSessionController> audioController;
    SerialHandler serial;
private slots:
    void onProcessSliderChange(ProcessVolumeSlider* s);
    void onSerialError(SerialPortError e);
    void onSerialInput(const QJsonObject &json);

};
#endif // MAINWINDOW_H
