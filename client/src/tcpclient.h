#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
class QTcpSocket;

class TCPClient : public QObject
{
   Q_OBJECT
public:
   explicit TCPClient( QObject *parent = nullptr );
   void connectTcp( QString ip, quint16 port );
   void setPath( QString newPath );
   void receiveFiles();
   void receiveOneFile(QString filename);
   void uploadFiles(QString folderPath);
   void downloadFiles(QString path,QJsonArray folderPath);
   void sendOneFile(QString filename);
   template<typename... Args>
   QString makeRpcCallString(QString function,Args... args);
signals:

public slots:
   void readTcpData();
private:
   static const int bufferSize;
   QTcpSocket* m_socket;
   QString m_path;
   bool ignoreRead;
};

template<typename... Args>
QString TCPClient::makeRpcCallString(QString function,Args... args)
{
    QJsonObject jsonreq;
    QJsonArray jarray;
    jsonreq["jsonrpc"] = "2.0";
    jsonreq["method"] = function;
    jarray = {args...};
    jsonreq["params"] = jarray;
    jsonreq["id"] = 0;
    QJsonDocument doc( jsonreq );
    return doc.toJson();
}

#endif // TCPSOCKET_H
