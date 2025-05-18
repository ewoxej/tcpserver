#include <iostream>
#include "tcpclient.h"
#include <QHostAddress>
#include <QDir>
#include <QRegExpValidator>
int main( int argc, char* argv[] )
{
   QString serveraddr;
   quint16 port = -1;
   if( argc < 3 )
   {
      std::cout << "Command line arguments should be next: ip port path" << std::endl;
      return -1;
   }
   std::cout <<
      "Enter \"sync\" to synchronize client folder with server folder.\n"
      "download(upload) filename - to download(upload) a file,\n"
      "\"filelist\" - to enumerate files in server folder" << std::endl;
   serveraddr = argv[1];
   QHostAddress address(serveraddr);
   if (QAbstractSocket::IPv4Protocol != address.protocol() && QAbstractSocket::IPv6Protocol != address.protocol())
   {
       std::cout << "IP adress is incorrect" << std::endl;
       return -1;
   }
   port = QString(argv[2]).toInt();
   if(port <0 )
   {
       std::cout << "Port is incorrect" << std::endl;
       return -1;
   }
   TCPClient socket;
   if( argc == 4 )
   {
       QDir dir;
       if(dir.exists(argv[3]))
      socket.setPath( argv[3] );
       else
       {
           std::cout<<"entered path is incorrect, default value will be used"<<"\n";
       }
   }
   socket.connectTcp( serveraddr, port );
   while( socket.isOpen() )
   {
      socket.readWriteData();
   }
   return 0;
}
