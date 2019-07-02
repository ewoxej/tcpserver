#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QObject>
class QTcpSocket;
const int bufferSize = 1024;
class TCPSocket : public QObject
{
   Q_OBJECT
public:
   explicit TCPSocket( QObject *parent = nullptr );
   void connectTcp( QString ip, quint16 port );
   void setPath( QString newPath );
signals:

public slots:
   void readTcpData();
private:
   QTcpSocket* socket;
   QString path;
};

#endif // TCPSOCKET_H
