#include "SerialHandler.h"
#include <QJsonParseError>
#include <QDebug>

SerialHandler::SerialHandler(const QString &portName, int baudRate, QObject *parent)
    : QThread(parent), portName(portName), baudRate(baudRate), running(true), serial(nullptr) {}

SerialHandler::~SerialHandler() {
    stop();
}

void SerialHandler::run() {
    serial = new QSerialPort();
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);

    if (!serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred("Failed to open serial port: " + serial->errorString());
        return;
    }

    while (running) {
        if (serial->waitForReadyRead(100)) {
            processIncomingData();
        }
        QThread::msleep(10);  // Reduce CPU usage
    }

    serial->close();
    delete serial;
    serial = nullptr;
}

void SerialHandler::stop() {
    QMutexLocker locker(&mutex);
    running = false;
}

void SerialHandler::sendJson(const QJsonObject &json) {
    if (!serial || !serial->isOpen()) {
        emit errorOccurred("Serial port is not open");
        return;
    }

    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n"; // Append newline for easy parsing
    serial->write(data);
    serial->flush();
}

void SerialHandler::processIncomingData() {
    static QByteArray buffer;
    buffer.append(serial->readAll());

    while (buffer.contains('\n')) {
        int index = buffer.indexOf('\n');
        QByteArray jsonData = buffer.left(index).trimmed();
        buffer.remove(0, index + 1);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            emit jsonReceived(doc.object());
        } else {
            emit errorOccurred("Failed to parse JSON: " + parseError.errorString());
        }
    }
}
