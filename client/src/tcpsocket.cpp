#include "tcpsocket.h"
#include <QDir>
#include <QTcpSocket>
#include <iostream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

TCPSocket::TCPSocket( QObject *parent ) : QObject( parent ), path( QDir::currentPath() )
{

}

void TCPSocket::connectTcp( QString ip, quint16 port )
{
   socket = new QTcpSocket( this );
   connect( socket, SIGNAL( readyRead() ), SLOT( readTcpData() ) );
   socket->connectToHost( ip, port );
   socket->waitForConnected();
   socket->waitForReadyRead();
}

void TCPSocket::setPath( QString newPath )
{
   path = newPath;
}

void TCPSocket::readTcpData()
{
   char buff[bufferSize];
   int recv_bytes;
   while( socket->isOpen() )
   {
      recv_bytes = socket->read( buff, bufferSize );
      buff[recv_bytes] = 0;
      std::wcout << buff << std::endl;
      std::cin >> buff;
      std::string strBuff( buff );
      QJsonObject jsonreq;
      jsonreq["jsonrpc"] = "2.0";
      jsonreq["method"] = buff;
      QJsonArray jarray;
      jarray.push_back( path );
      if( strBuff == "download" || strBuff == "upload" )
      {
         std::cout << "Enter filename:";
         std::cin >> buff;
         jarray.push_back( buff );

      }

      jsonreq["params"] = jarray;
      jsonreq["id"] = 0;
      QJsonDocument doc( jsonreq );
      strBuff = doc.toJson( QJsonDocument::Compact ).toStdString();
      std::cout << strBuff << std::endl;
      socket->write( strBuff.c_str(), strBuff.length() );
      socket->waitForReadyRead();
   }
}
