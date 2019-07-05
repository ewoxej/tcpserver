#include "tcpclient.h"
#include <QDataStream>
#include <QDir>
#include <QTcpSocket>
#include <iostream>
#include <string>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

const int TCPClient::bufferSize = 1024;

TCPClient::TCPClient( QObject *parent ) : QObject( parent ), m_path( QDir::currentPath() ), ignoreRead(false)
{

}

void TCPClient::connectTcp( QString ip, quint16 port )
{
   m_socket = new QTcpSocket( this );
   connect( m_socket, SIGNAL( readyRead() ), SLOT( readTcpData() ) );
   m_socket->connectToHost( ip, port );
   m_socket->waitForConnected();
   m_socket->waitForReadyRead();
}

void TCPClient::setPath( QString newPath )
{
   m_path = newPath;
}

void TCPClient::readTcpData()
{
   QTextStream cin(stdin);
   char buff[bufferSize];
   int recvBytes;
   while( m_socket->isOpen() )
   {
      strcpy_s(buff,"false");
      recvBytes = m_socket->read( buff, bufferSize );
      if(recvBytes > 0)
      buff[recvBytes] = 0;
      //terminate C-string
      QString strBuff( buff );
      std::cout<<strBuff.toStdString()<<std::endl;
      strBuff = cin.readLine();
      QJsonObject jsonreq;
      QStringList list = strBuff.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      jsonreq["jsonrpc"] = "2.0";
      if(list[0] == "download")
         jsonreq["method"] = "exist";
      else
      jsonreq["method"] = list[0];
      QJsonArray jarray;
      if(list.size()>1)
      jarray.push_back( list[1]);
      jsonreq["params"] = jarray;
      jsonreq["id"] = 0;
      QJsonDocument doc( jsonreq );
      strBuff = doc.toJson();
      std::cout << strBuff.toStdString() << std::endl;
      m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
      if(list[0] == "download" && list.size()==2)
      {
receiveOneFile(list[1]);
      }
      if(list[0] == "upload" && list.size()==2)
      {
sendOneFile(list[1]);
      }
      if(!ignoreRead)
      m_socket->waitForReadyRead();
      else
      {
          ignoreRead = false;
      }
   }
}

void TCPClient::receiveOneFile(QString filename)
{
    QString strBuff;
    QByteArray barray;
    QJsonObject jsonreq;
    QJsonArray jarray;
    m_socket->waitForReadyRead();
    barray = m_socket->readAll();
    jsonreq["jsonrpc"] = "2.0";
    if(barray == "false")
    {
    ignoreRead = true;
    return;
    }
    jsonreq["method"] = "download";
    jarray.push_back(filename);
    jsonreq["params"] = jarray;
    jsonreq["id"] = 0;
    QJsonDocument doc( jsonreq );
    strBuff = doc.toJson();
    m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
    QFile file(filename);
    file.open(QIODevice::ReadWrite);
        m_socket->waitForReadyRead();
        char arr[4];
        unsigned long sz = 0;
        m_socket->read(arr,4);
        sz |= (arr[0] << 24) & 0xff000000;
        sz |= (arr[1] << 16) & 0x00ff0000;
        sz |= ( arr[2] << 8 ) & 0x0000ff00;
        sz |= arr[3] & 0x000000ff;
        unsigned long receivedBytes = 0;
        while(receivedBytes < sz)
        {
        barray.clear();
        m_socket->waitForReadyRead();
        barray = m_socket->readAll();
        receivedBytes+=barray.size();
        file.write(barray);
        }
        strBuff = "ok";
        m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
}

void TCPClient::sendOneFile(QString filename)
{
    m_socket->waitForBytesWritten();
char buffer[bufferSize];
QFile file(m_path+"//"+filename);
file.open(QIODevice::ReadOnly);
char arr[4]{0};
if(!file.isOpen() )
{
       m_socket->write(arr,4);
    return;
}
unsigned long filesize = QFileInfo(file).size();
   arr[0] = ( filesize >> 24 );
   arr[1] = ( filesize >> 16 );
   arr[2] = ( filesize >> 8 );
   arr[3] = filesize;
   m_socket->write(arr,4);
unsigned long sendedBytes = 0;
int readedBytes;
   while(sendedBytes < filesize)
   {
readedBytes=file.read(buffer,bufferSize);
m_socket->write(buffer,readedBytes);
sendedBytes+=readedBytes;
   }
}

qint64 byteArrayToFileSize(char *array)
{

}

void fileSizeToByteArray(qint64 fsize, char *array)
{

}
