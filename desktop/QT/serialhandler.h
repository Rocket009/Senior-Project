#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

typedef QSerialPort::SerialPortError SerialPortError;

class SerialHandler : public QObject {
    Q_OBJECT

public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    bool connectSerial();
    void disconnectSerial();
    bool isOpen();

public slots:
    void sendJson(const QJsonObject &json);

signals:
    void jsonReceived(const QJsonObject &json);
    void errorOccurred(SerialPortError error);

private:
    QSerialPortInfo port;
    int baudRate;
    QSerialPort *serial = nullptr;
    void findSerialPort();
    qint64 bytesToWrite = 0;
    QTimer *timer = nullptr;
private slots:
    void processIncomingData();
    void handleBytesWritten(qint64 bytes);

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
