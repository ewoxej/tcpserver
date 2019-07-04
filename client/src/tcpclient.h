#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QObject>
class QTcpSocket;
class TCPClient : public QObject
{
   Q_OBJECT
public:
   explicit TCPClient( QObject *parent = nullptr );
   void connectTcp( QString ip, quint16 port );
   void setPath( QString newPath );
signals:

public slots:
   void readTcpData();
private:
   static const int bufferSize;
   QTcpSocket* m_socket;
   QString m_path;
};

#endif // TCPSOCKET_H
