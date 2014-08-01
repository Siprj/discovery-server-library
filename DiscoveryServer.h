#ifndef DISCOVERYSERVER_H
#define DISCOVERYSERVER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QHostAddress>

class QUdpSocket;

class DiscoveryServer : public QObject
{
    Q_OBJECT

public:
    explicit DiscoveryServer(quint16 port = 21000, QHostAddress bindAddress = QHostAddress::Any, QObject *parent = 0);
    virtual ~DiscoveryServer();

    bool startListen();
    void stopListen();

    void clearDeviceMap();

    quint16 getPort() const;
    void setPort(const quint16 &port);

    QHostAddress getBindAddress() const;
    void setBindAddress(const QHostAddress &value);

    QMap<QString, QHostAddress> getDeviceMap() const;

public slots:
    void discover();

signals:
    void newDeviceFound(QString, QHostAddress);
    //void deviceLost(QString, QHostAddress);

    void deviceMapUpdated();

private slots:
    void precessPendingDatagram();


private:
    void sendBroadcastToAll(QByteArray *data);              //this is responsible for sending broadcast on every interface and every address on interface
    bool insertDevice(QHostAddress ipAddress, QString string);      //return true if new device added otherwise return false

    QHostAddress bindAddress;               // UDP address where to listen for response
    quint16 port;                           // UDP Port
    QMap<QString, QHostAddress> deviceMap;  // <Name(description), IP address>
    QUdpSocket *udpSocket;

};

#endif // DISCOVERYSERVER_H
