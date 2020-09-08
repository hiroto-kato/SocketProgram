#include "ws-util.h"

#include <winsock2.h>

#include <iostream>
#include <string.h>
#include <time.h>

using namespace std;

// Comment this out to disable the shutdown-delay functionality.
#define SHUTDOWN_DELAY

// Constants
const double seconds = 5;
const int kBufferSize = 8 * 1024;

#if defined(SHUTDOWN_DELAY)
const int kShutdownDelay = 3;
#endif

// Prototypes
u_long LookupAddress(const char* pcHost);
SOCKET EstablishConnection(u_long nRemoteAddr, u_short nPort);
bool SendEcho(SOCKET sd);
//int ReadReply(SOCKET sd);


// winsockでの通信処理
int DoWinsock(const char* pcHost, int nPort)
{
  // Serverのアドレスを探す
  cout << "Looking up address..." << flush;
  u_long nRemoteAddress = LookupAddress(pcHost);
  if (nRemoteAddress == INADDR_NONE) {
    cerr << endl << WSAGetLastErrorMessage("lookup address") << endl;
    return 3;
  }
  in_addr Address;
  memcpy(&Address, &nRemoteAddress, sizeof(u_long));
  cout << inet_ntoa(Address) << ":" << nPort << endl;
  
  // Serverと接続
  cout << "Connecting to remote host..." << flush;
  SOCKET sd = EstablishConnection(nRemoteAddress, htons(nPort));
  if (sd == INVALID_SOCKET) {
    cerr << endl << WSAGetLastErrorMessage("connect to server") << endl;
    return 3;
  }
  cout << "connected, socket " << sd << "." << endl;
  
  // Serverにパケットを送る
  int nBytes;
  bool isSend = SendEcho(sd);
  if (isSend) {
    cout << endl;
  }
  else {
    cerr << endl << WSAGetLastErrorMessage("send echo packet") << endl;
    return 3;
  }
  
#if defined(SHUTDOWN_DELAY)
  cout << "Will shut down in " << kShutdownDelay << " seconds... (one dot per second): " << flush;
  for (int i = 0; i < kShutdownDelay; ++i) {
    Sleep(1000);
    cout << '.' << flush;
  }
  cout << endl;
#endif
  
  // 切断
  cout << "Shutting connection down..." << flush;
  if (ShutdownConnection(sd)) {
    cout << "Connection is down." << endl;
  }
  else {
    cout << endl << WSAGetLastErrorMessage("Shutdown connection") << endl;
  }
  
  cout << "All done!" << endl;
  
  return 0;
}


// アドレスを探す
u_long LookupAddress(const char* pcHost)
{
  u_long nRemoteAddr = inet_addr(pcHost);
  if (nRemoteAddr == INADDR_NONE) {
    hostent* pHE = gethostbyname(pcHost);
    if (pHE == 0) {
      return INADDR_NONE;
    }
    nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
  }
  
  return nRemoteAddr;
}


// 接続確立
SOCKET EstablishConnection(u_long nRemoteAddr, u_short nPort)
{
  // ソケットの作成
  SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd != INVALID_SOCKET) {
    sockaddr_in sinRemote;
    sinRemote.sin_family = AF_INET;
    sinRemote.sin_addr.s_addr = nRemoteAddr;
    sinRemote.sin_port = nPort;
    if (connect(sd, (sockaddr*)&sinRemote, sizeof(sockaddr_in)) ==
        SOCKET_ERROR) {
      sd = INVALID_SOCKET;
    }
  }
  
  return sd;
}


// パケットを送る
bool SendEcho(SOCKET sd){
  char *kBuf = NULL;
  int currLen = 0;
  double totalLen = 0;
  double time = 0;
  clock_t start = clock();
  do{
    currLen = send(sd, "", kBufferSize, 0);
    if(currLen < 0){
      return false;
    }

    totalLen += currLen;
    time = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
  }while(time < seconds);

  // 速度
  cout << time << " sec " << totalLen << " Bytes " << totalLen / time << " bps" << endl;
  return true;
}

