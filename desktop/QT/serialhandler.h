#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>

class SerialHandler : public QThread {
    Q_OBJECT

public:
    explicit SerialHandler(const QString &portName, int baudRate = QSerialPort::Baud115200, QObject *parent = nullptr);
    ~SerialHandler();

    void run() override;
    void stop();

public slots:
    void sendJson(const QJsonObject &json);

signals:
    void jsonReceived(const QJsonObject &json);
    void errorOccurred(const QString &error);

private:
    QString portName;
    int baudRate;
    QSerialPort *serial;
    QMutex mutex;
    bool running;

    void processIncomingData();
};

#endif // SERIALHANDLER_H
// #ifndef SERIALHANDLER_H
// #define SERIALHANDLER_H

// #include <QObject>
// #include <QThread>
// #include <QVector>
// #include <QMutex>

// class SerialHandler : public QThread
// {
//     Q_OBJECT
// public:
//     // These methods will be called on the parent thread
//     explicit SerialHandler(QObject *parent = nullptr);
//     ~SerialHandler();
//     void connectDevice(const QString &portName);
//     QVector<QString> getSerialPorts();
//     void disconnectDevice();
//     QString getConnectedDevice();
//     void transmitData(const QString &s);
// signals:
//     void onDataRecv(const QString &s);
//     void error(const QString &s);
// private:
//     void run() override;
//     QString m_portName;
//     QString m_responce;
//     QMutex m_mutex;
// };

// class SerialError : public std::exception
// {
// public:
//     const char* what() const noexcept override
//     {
//         return "Serial Error Occurred";
//     }
// };

// #endif // SERIALHANDLER_H
