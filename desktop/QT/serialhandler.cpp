#include "SerialHandler.h"
#include <QJsonParseError>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include <chrono>

static constexpr std::chrono::seconds serialWriteTimeout = std::chrono::seconds{5};

#define RPI_USB_DESCRIPTION "Gadget Serial v2.4"

SerialHandler::SerialHandler(QObject *parent) : QObject(parent) {}

bool SerialHandler::connectSerial()
{
    if(timer == nullptr)
    {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=](){emit errorOccurred(SerialPortError::TimeoutError);});
        timer->setSingleShot(true);
    }
    if(serial == nullptr)
    {
        serial = new QSerialPort();
        connect(serial, &QSerialPort::errorOccurred, this, [=](QSerialPort::SerialPortError e){
            emit errorOccurred(static_cast<SerialPortError>(e));
        });
        connect(serial, &QSerialPort::readyRead, this, &SerialHandler::processIncomingData);
        connect(serial, &QSerialPort::bytesWritten, this, &SerialHandler::handleBytesWritten);
    }
    for(const auto &p : QSerialPortInfo::availablePorts())
    {
        if(p.description() == RPI_USB_DESCRIPTION)
        {
            port = p;
            break;
        }
    }

    if(!port.isNull())
    {
        serial->setPort(port);
        if (!serial->open(QIODevice::ReadWrite))
        {
            serial->setBaudRate(QSerialPort::Baud115200);
            return true;
        }
        else return false;
    }
    else return false;

}

bool SerialHandler::isOpen()
{
    return serial->isOpen();
}

void SerialHandler::disconnectSerial()
{
    if(isOpen()) serial->close();
}

SerialHandler::~SerialHandler() {
    if(serial->isOpen()) serial->close();
    disconnect(serial, nullptr, nullptr, nullptr);
    delete serial;
    serial = nullptr;
    delete timer;
    timer = nullptr;
}

void SerialHandler::findSerialPort()
{

}


void SerialHandler::sendJson(const QJsonObject &json) {
    if (!serial || !serial->isOpen()) {
        emit errorOccurred(SerialPortError::NotOpenError);
        return;
    }

    QByteArray data = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n"; // Append newline for easy parsing
    const qint64 written = serial->write(data);
    if(written == data.size())
    {
        bytesToWrite += written;
        timer->start(serialWriteTimeout);
    }
    else emit errorOccurred(SerialPortError::WriteError);
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
            emit errorOccurred(SerialPortError::UnknownError);
        }
    }
}

void SerialHandler::handleBytesWritten(qint64 bytes)
{
    bytesToWrite -= bytes;
    if(bytesToWrite == 0) timer->stop();
}
