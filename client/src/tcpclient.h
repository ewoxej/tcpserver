#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QObject>
class QTcpSocket;

qint64 byteArrayToFileSize(char* array);
void fileSizeToByteArray(qint64 fsize,char* array);
class TCPClient : public QObject
{
   Q_OBJECT
public:
   explicit TCPClient( QObject *parent = nullptr );
   void connectTcp( QString ip, quint16 port );
   void setPath( QString newPath );
   void receiveFiles();
   void receiveOneFile(QString filename);
   void sendOneFile(QString filename);
signals:

public slots:
   void readTcpData();
private:
   static const int bufferSize;
   QTcpSocket* m_socket;
   QString m_path;
   bool ignoreRead;
};

#endif // TCPSOCKET_H
