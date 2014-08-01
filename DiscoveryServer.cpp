#include "DiscoveryServer.h"

#include <QNetworkInterface>
#include <QUdpSocket>
#include <QRegExp>
#include <QStringList>

#define DISCOVERY_WORD_SIZE 3           // Number of word splitted by spaces
#define DISCOVERY_POSITION_OF_IP 1      // Number of IP in message
#define DISCOVERY_POSITION_OF_NAME 2    // Number of Device Name
#define DISCOVERY_RESPONCE_PREFIX "<<DISCOVERY:"    // First part of response (first word)
#define DISCOVERY_RESPONCE_END ">>"
#define DISCOVERY_QUESTION "<<DISCOVERY>>"
#define DISCOVERY_IP_REGEXP_VALIDATOR "([01]?\\d\\d?|2[0-4]\\d|25[0-5])."\
                                      "([01]?\\d\\d?|2[0-4]\\d|25[0-5])."\
                                      "([01]?\\d\\d?|2[0-4]\\d|25[0-5])."\
                                      "([01]?\\d\\d?|2[0-4]\\d|25[0-5])"

DiscoveryServer::DiscoveryServer(quint16 port, QHostAddress bindAddress, QObject *parent):
    QObject(parent)
{
    this->port = port;
    this->bindAddress = bindAddress;
    udpSocket = new QUdpSocket;

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(precessPendingDatagram()));
}

DiscoveryServer::~DiscoveryServer()
{
    delete udpSocket;
}

bool DiscoveryServer::startListen()
{
    return udpSocket->bind(bindAddress, port);
}

void DiscoveryServer::stopListen()
{
    return udpSocket->close();
}

quint16 DiscoveryServer::getPort() const
{
    return port;
}

void DiscoveryServer::setPort(const quint16 &port)
{
    this->port = port;
}


QHostAddress DiscoveryServer::getBindAddress() const
{
    return bindAddress;
}

void DiscoveryServer::setBindAddress(const QHostAddress &value)
{
    bindAddress = value;
}

QMap<QString, QHostAddress> DiscoveryServer::getDeviceMap() const
{
    return deviceMap;
}

bool DiscoveryServer::insertDevice(QHostAddress ipAddress, QString string)
{
    bool deviceExist = false;

    QList<QHostAddress> existingAddresses = deviceMap.values();   //get ip addresses
    for(int i = 0; i < existingAddresses.size(); i++)           //check if device exist
    {
        if(ipAddress == existingAddresses.at(i))
        {
            deviceExist = true;
            break;
        }
    }

    if(!deviceExist)        //device don't exist insert it in to map
    {
        qDebug()<<"device don exist";
        deviceMap.insert(string, ipAddress);
        emit newDeviceFound(string, ipAddress);
        return true;
    }

    return false;
}

void DiscoveryServer::sendBroadcastToAll(QByteArray *data)
{
    // Get network interfaces list
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Interfaces iteration
    for (int i = 0; i < ifaces.size(); i++)
    {
        // Now get all IP addresses for the current interface
        QList<QNetworkAddressEntry> addresses = ifaces[i].addressEntries();

        // And for any IP address, if it is IPv4 and the interface is active, send the packet
        for (int y = 0; y < addresses.size(); y++)
        {
            if ((addresses[y].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addresses[y].broadcast().toString() != ""))
                udpSocket->writeDatagram(data->data(), data->length(), addresses[y].broadcast(), port);
        }
    }
}

void DiscoveryServer::discover()                        // send discovery datagram to braodcast address/es
{
    QByteArray datagram;
    datagram.append(QString(DISCOVERY_QUESTION));
    if(bindAddress == QHostAddress::Any)  // check if udpSocket is binded to all network or only to one
    {                                                   // don't use bindAddress for this purpose
        sendBroadcastToAll(&datagram);
    }
    else
    {
        // Get network interfaces list
        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
        for (int i = 0; i < ifaces.size(); i++)
        {
            // Now get all IP addresses for the current interface
            QList<QNetworkAddressEntry> addresses = ifaces[i].addressEntries();

            // And for any IP address, if it is IPv4 and the interface is active, send the packet
            for (int y = 0; y < addresses.size(); y++)
            {
                if ((addresses[y].ip() == bindAddress) && (addresses[y].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addresses[y].broadcast().toString() != ""))
                {
                    udpSocket->writeDatagram(datagram.data(), datagram.length(), addresses[y].broadcast(), port);
                }
            }
        }
    }
}

void DiscoveryServer::clearDeviceMap()
{
    deviceMap.clear();
}


void DiscoveryServer::precessPendingDatagram()
{
    int discoveryUpdated = 0;        //variable for controlling if discovery shut emit updated

    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        QString str = datagram.data();      //prepare data for parsing

        //validate packet format
        if(!str.contains(QRegExp("^"DISCOVERY_RESPONCE_PREFIX" "DISCOVERY_IP_REGEXP_VALIDATOR".*"DISCOVERY_RESPONCE_END)))
        {
            qDebug()<<"Discovery response error, invalid packet format";
            break;      //packet format is not valid
        }

        str = str.trimmed();                      //remove useless spaces
        QStringList list = str.split(' ');
        if(list.size() >= DISCOVERY_WORD_SIZE)      // check if the message have right number of parts
        {
            if(list.at(0) == DISCOVERY_RESPONCE_PREFIX)    //if this is true, than discovery reply chase been received
            {
                QString deviceIpStr, deviceString;
                deviceIpStr = list.at(DISCOVERY_POSITION_OF_IP);

                for(int i = DISCOVERY_POSITION_OF_NAME; i < list.size(); i++)
                {
                    QString helpString = list.at(i);
                    helpString.remove(QChar('>'));  //remove >> from last part of message
                    deviceString.append(helpString).append(" ");
                }
                deviceString = deviceString.trimmed();

                QHostAddress deviceAddress(deviceIpStr);
                discoveryUpdated = insertDevice(deviceAddress, deviceString);
            }
        }
        else
        {
            qDebug()<<"Discovery response error, device send wrong string";
        }
    }
    if(discoveryUpdated)
    {
        emit deviceMapUpdated();
    }
}
