#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
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

    template<class T>
    static std::vector<T*> getWidgetsFromLayout(QLayout* lay)
    {
        std::vector<T*> vec;
        for(int i = 0; i < lay->count(); i++)
        {
            QLayoutItem* item = lay->itemAt(i);
            if(item)
            {
                T* widget = qobject_cast<T*>(item->widget());
                if(widget)
                    vec.push_back(widget);
            }
        }
        return vec;
    }


private:
    Ui::MainWindow *ui;
    void createProcessVolumeWidgets();
    std::shared_ptr<IAudioSessionController> audioController;
    SerialHandler serial;
    QHBoxLayout* volumeSliderLayout;
private slots:
    void onProcessSliderChange(ProcessVolumeSlider* s);
    void onSerialError(SerialPortError e);
    void onSerialInput(const QJsonObject &json);

};
#endif // MAINWINDOW_H
