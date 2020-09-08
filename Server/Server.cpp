#include "ws-util.h"

#include <winsock2.h>
#include <iostream>

using namespace std;


// Constants
const int kBufferSize = 8 * 1024;

// Prototypes
SOCKET SetUpListener(const char* pcAddress, int nPort);
SOCKET AcceptConnection(SOCKET ListeningSocket, sockaddr_in& sinRemote);
bool EchoIncomingPackets(SOCKET sd);

// winsockでの通信処理
int DoWinsock(const char* pcAddress, int nPort){
  // Listen開始
  cout << "Establishing the listener..." << endl;
  SOCKET ListeningSocket = SetUpListener(pcAddress, htons(nPort));
  if (ListeningSocket == INVALID_SOCKET) {
    cout << endl << WSAGetLastErrorMessage("establish listener") << endl;
    return 3;
  }
  
  // クライアント処理
  while (1) {
    // 接続待ち・接続許可
    cout << "Waiting for a connection..." << flush;
    sockaddr_in sinRemote;
    SOCKET sd = AcceptConnection(ListeningSocket, sinRemote);
    if (sd != INVALID_SOCKET) {
      cout << "Accepted connection from " <<
        inet_ntoa(sinRemote.sin_addr) << ":" <<
        ntohs(sinRemote.sin_port) << "." << endl;
    }
    else {
      cout << endl << WSAGetLastErrorMessage("accept connection") << endl;
      return 3;
    }
    
    // パケットをクライアントから受け取る
    if (EchoIncomingPackets(sd)) {
      // 接続終了
      cout << "Shutting connection down..." << flush;
      if (ShutdownConnection(sd)) {
        cout << "Connection is down." << endl;
      }
      else {
        cout << endl << WSAGetLastErrorMessage("shutdown connection") << endl;
        return 3;
      }
    }
    else {
      cout << endl << WSAGetLastErrorMessage("echo incoming packets") << endl;
      return 3;
    }
  }
  
#if defined(_MSC_VER)
  return 0;       // warning eater
#endif
}

// Listenerのセットアップ
SOCKET SetUpListener(const char* pcAddress, int nPort){
  u_long nInterfaceAddr = inet_addr(pcAddress);
  if (nInterfaceAddr != INADDR_NONE) {
    SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd != INVALID_SOCKET) {
      sockaddr_in sinInterface;
      sinInterface.sin_family = AF_INET;
      sinInterface.sin_addr.s_addr = nInterfaceAddr;
      sinInterface.sin_port = nPort;
      if (bind(sd, (sockaddr*)&sinInterface,
               sizeof(sockaddr_in)) != SOCKET_ERROR) {
        listen(sd, 1);
        return sd;
      }
    }
  }
  
  return INVALID_SOCKET;
}

// 接続待ち処理
SOCKET AcceptConnection(SOCKET ListeningSocket, sockaddr_in& sinRemote){
  int nAddrSize = sizeof(sinRemote);
  return accept(ListeningSocket, (sockaddr*)&sinRemote, &nAddrSize);
}

// パケットを受け取る
bool EchoIncomingPackets(SOCKET sd){
  // 受信バッファ
  char readBuf[kBufferSize];
  int currLen;
  int totalLen = 0;
  
  do{
    currLen = recv(sd, readBuf, kBufferSize, 0);
    if(currLen < 0) {
      return false;
    }

    totalLen += currLen;
  } while (currLen != 0);

  cout << totalLen << " Bytes" << endl;
  cout << "Connection closed by peer." << endl;
}

