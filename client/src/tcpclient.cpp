#include "tcpclient.h"
#include <QDir>
#include <QTcpSocket>
#include <iostream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

const int TCPClient::bufferSize = 1024;

TCPClient::TCPClient( QObject *parent ) : QObject( parent ), m_path( QDir::currentPath() )
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
   char buff[bufferSize];
   int recv_bytes;
   while( m_socket->isOpen() )
   {
      recv_bytes = m_socket->read( buff, bufferSize );
      buff[recv_bytes] = 0;
      std::wcout << buff << std::endl;
      std::cin >> buff;
      std::string strBuff( buff );
      QJsonObject jsonreq;
      jsonreq["jsonrpc"] = "2.0";
      jsonreq["method"] = buff;
      QJsonArray jarray;
      jarray.push_back( m_path );
      if( strBuff == "download" || strBuff == "upload" )
      {
         std::cout << "Enter filename:";
         std::cin >> buff;
         jarray.push_back( buff );

      }

      jsonreq["params"] = jarray;
      jsonreq["id"] = 0;
      QJsonDocument doc( jsonreq );
      strBuff = doc.toJson().toStdString();
      std::cout << strBuff << std::endl;
      m_socket->write( strBuff.c_str(), strBuff.length() );
      m_socket->waitForReadyRead();
   }
}
