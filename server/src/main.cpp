#define _WINSOCK2API_
#include <Windows.h>
#include "socket_serve.h"
#include <CommCtrl.h>
#include <shlobj.h>
#pragma comment(lib,"Shell32.lib")
#include <tchar.h>

#include <thread>
#include <string>

#include "Resource.h"

HINSTANCE hInst;
NOTIFYICONDATA Icon = { 0 };
HMENU popupMenu;
DWORD longIP;
UINT port = 800;
char serviceName[MAX_PATH];
char folderPath[MAX_PATH];

INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );
BOOL CommonFileDlg( HWND hWnd, LPWSTR pstrFileName, LPWSTR filters, BOOL open );

LRESULT CALLBACK WndProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
   switch( message )
   {
   case WM_COMMAND:
   {
      switch( static_cast<UINT>( LOWORD( wParam ) ) )
      {
      case IDB_SETTING:
      {
         DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), window, DlgProc );
      }
      break;
      case IDB_HELP:
      {
         MessageBox( window, L"This is a file sharing server. Configure it before running", L"Help", MB_OK );
      }
      break;
      case IDB_EXIT:
      {
         Shell_NotifyIcon( NIM_DELETE, &Icon );
         WSACleanup();
         PostQuitMessage( 0 );
      }
      break;
      }
   }
   break;
   case WM_USER:
   {
      POINT pt;
      GetCursorPos( &pt );
      if( lParam == WM_RBUTTONDOWN )
      {
         TrackPopupMenu( popupMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, window, nullptr );
      }
   }
   break;

   case WM_DESTROY:
   {
      DestroyMenu( popupMenu );
      PostQuitMessage( 0 );
      break;
   }
   default:
      return DefWindowProc( window, message, wParam, lParam );
   }
   return 0;
}

int APIENTRY _tWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPTSTR cmdLine, int showCmd )
{
   hInst = instance;
   WNDCLASSEX main = { 0 };
   main.cbSize = sizeof( WNDCLASSEX );
   main.hInstance = instance;
   main.lpszClassName = TEXT( "Main" );
   main.lpfnWndProc = WndProc;
   RegisterClassEx( &main );
   GetCurrentDirectoryA( MAX_PATH, folderPath );
   HWND window = CreateWindowEx( 0, TEXT( "Main" ), nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr );

   Icon.cbSize = sizeof( NOTIFYICONDATA );
   Icon.hWnd = window;
   Icon.uVersion = NOTIFYICON_VERSION;
   Icon.uCallbackMessage = WM_USER;
   Icon.hIcon = LoadIcon( nullptr, IDI_WINLOGO );
   Icon.uFlags = NIF_MESSAGE | NIF_ICON;
   Shell_NotifyIcon( NIM_ADD, &Icon );
   popupMenu = CreatePopupMenu();
   longIP = MAKEIPADDRESS( 127, 0, 0, 1 );
   TCHAR strhelp[] = L"Help";
   TCHAR strset[] = L"Setting";
   TCHAR strexit[] = L"Exit";
   AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_SETTING, strset );
   AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_HELP, strhelp );
   AppendMenu( popupMenu, MF_STRING | MF_MOUSESELECT, IDB_EXIT, strexit );
   MSG message;
   while( GetMessage( &message, nullptr, 0, 0 ) )
   {
      TranslateMessage( &message );
      DispatchMessage( &message );
   }
   Shell_NotifyIcon( NIM_DELETE, &Icon );
   return 0;
}

INT_PTR APIENTRY DlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
   static std::thread serverTh;
   TCHAR buffer[MAX_PATH];
   switch( message )
   {
   case WM_INITDIALOG:
   {
      DWORD bufSize = MAX_PATH;
      GetComputerNameA( serviceName, &bufSize );
      LONG style = GetWindowLong( GetDlgItem( hwndDlg, IDC_EDIT3 ), GWL_STYLE );
      SetWindowLongPtr( GetDlgItem( hwndDlg, IDC_EDIT3 ), GWL_STYLE, style | ES_NUMBER );
      SendMessage( GetDlgItem( hwndDlg, IDC_EDIT3 ), EM_SETLIMITTEXT, 4, 0 );
      SendMessage( GetDlgItem( hwndDlg, IDC_EDIT3 ), WM_SETTEXT, 0, reinterpret_cast<LPARAM>( std::to_wstring( port ).c_str() ) );
      SendMessage( GetDlgItem( hwndDlg, IDC_IPADDRESS1 ), IPM_SETADDRESS, 0, static_cast<LPARAM>( longIP ) );
      SendMessageA( GetDlgItem( hwndDlg, IDC_EDIT2 ), WM_SETTEXT, 0, reinterpret_cast<LPARAM>( serviceName ) );
      SetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT1 ), folderPath );
      return true;
   }
   case WM_COMMAND:
      switch( LOWORD( wParam ) )
      {
      case IDC_BUTTON1:
      {
         TCHAR current[MAX_PATH];
         BROWSEINFO bis;
         bis.hwndOwner = nullptr;
         bis.pidlRoot = nullptr;
         bis.pszDisplayName = buffer;
         bis.lpszTitle = L"HERE";
         bis.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
         bis.lpfn = nullptr;
         bis.lParam = reinterpret_cast<LPARAM>( current );
         LPITEMIDLIST lst = SHBrowseForFolder( &bis );
         SHGetPathFromIDListA( lst, folderPath );
         SetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT1 ), folderPath );
      }
      break;
      case IDOK:
      {
         const int portArraySize = 5;
         char cport[portArraySize];
         SendMessage( GetDlgItem( hwndDlg, IDC_IPADDRESS1 ), IPM_GETADDRESS, 0, LPARAM( &longIP ) );
         GetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT2 ), serviceName, MAX_PATH );
         GetWindowTextA( GetDlgItem( hwndDlg, IDC_EDIT3 ), cport, portArraySize );
         port = std::stoi( cport );
         serverTh = std::thread( socketInit, longIP, port );
         serverTh.detach();
         socketInit( longIP, port );
      }

      case IDCANCEL:
         EndDialog( hwndDlg, 0 );
         return TRUE;
      }
      break;
   }
   return FALSE;
}

BOOL CommonFileDlg( HWND hWnd, LPWSTR pstrFileName, LPWSTR filters, BOOL open )
{
   OPENFILENAME ofn;
   ZeroMemory( &ofn, sizeof( ofn ) );
   ofn.lStructSize = sizeof( ofn );
   ofn.hwndOwner = hWnd;
   ofn.lpstrFile = pstrFileName;
   ofn.lpstrFile[0] = '\0';
   ofn.nMaxFile = MAX_PATH;
   ofn.nFilterIndex = 1;
   ofn.lpstrFilter = filters;
   ofn.lpstrFileTitle = nullptr;
   ofn.Flags = !OFN_FILEMUSTEXIST;
   if( ( open ) ? GetOpenFileName( &ofn ) : GetSaveFileName( &ofn ) )
   {
      return 1;
   }
   return 0;
}
