#include "tcpclient.h"
#include <QDataStream>
#include <QDir>
#include <QTcpSocket>
#include <iostream>
#include <string>


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
   QString barray;
   while( m_socket->isOpen() )
   {
      barray = "false";
      barray = m_socket->readAll();
      std::cout<<barray.toStdString()<<std::endl;
      barray = cin.readLine();
      QStringList list = barray.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      if(list[0]=="download" && list.size()>1)
      barray = makeRpcCallString("exist",list[1]);
      else if(list.size()>1)
      barray = makeRpcCallString(list[0],list[1]);
      else
      barray = makeRpcCallString(list[0]);
      std::cout << barray.toStdString() << std::endl;
      m_socket->write( barray.toStdString().c_str(), barray.length() );
      if(list[0] == "sync")
      {
          m_socket->waitForReadyRead();
          m_socket->readAll();
          barray = makeRpcCallString("filelist");
          m_socket->write( barray.toStdString().c_str(), barray.length() );
          m_socket->waitForReadyRead();
          barray = m_socket->readAll();
          QByteArray bar;
          bar.append(barray);
          QJsonDocument doc(QJsonDocument::fromJson(bar));
          QJsonArray jarr = doc.array();
          downloadFiles("",jarr);
          //uploadFiles("");
      }
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
    QFile file(m_path+"//"+filename);
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

void TCPClient::uploadFiles(QString folderPath)
{
    QDir dir(m_path+"\\"+folderPath);
        QStringList nameFilter;
        QFileInfoList clientFolderList = dir.entryInfoList( nameFilter, QDir::Files | QDir::Dirs);
        QFileInfo fileinfo;
        QString strBuff;
        QByteArray barray;
for(auto i : clientFolderList)
{
    if(i.isDir() && i.fileName()!= "." && i.fileName()!="..")
    {
        QString fpath = i.path() + "\\" + i.fileName();
    uploadFiles(i.fileName());
    }
    if(i.isFile())
    {
   strBuff = makeRpcCallString("upload",folderPath+"\\"+i.fileName());
   m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
   sendOneFile(folderPath+"\\"+i.fileName());
    m_socket->waitForReadyRead();
    barray = m_socket->readAll();
    }
}
ignoreRead = true;
}

void TCPClient::downloadFiles(QString path,QJsonArray jarr)
{
    QString strBuff;
    QByteArray barray;
    QString absPath = m_path + "\\" + path;
for(int i=0;i<jarr.size();i++)
{
    if(jarr[i].isArray())
    {
        QDir dir;
        QFile fileForDelete(absPath + "\\" +jarr[i-1].toString());
        fileForDelete.remove();
        dir.mkdir(absPath + "\\" +jarr[i-1].toString());
        downloadFiles(path + "\\" + jarr[i-1].toString(),jarr[i].toArray());
    }
    else
    {
    strBuff = makeRpcCallString("exist",jarr[i].toString());
    m_socket->write( strBuff.toStdString().c_str(), strBuff.length() );
    receiveOneFile(path+ "\\" +jarr[i].toString());
    if(QFile(jarr[i].toString()).exists())
    {
     m_socket->waitForReadyRead();
     barray = m_socket->readAll();
    }
    }
}
ignoreRead = true;
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



