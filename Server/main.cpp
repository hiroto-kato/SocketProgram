#include <winsock2.h>

#include <stdlib.h>
#include <iostream>

using namespace std;


// Prototypes
extern int DoWinsock(const char* pcHost, int nPort);


// Constants
// Default port to connect to on the server
const int kDefaultServerPort = 4242;


int main(int argc, char* argv[]){
  // 引数確認処理
  if (argc < 2) {
    cerr << "usage: " << argv[0] << " <server-address> " << "[server-port]" << endl << endl;
    cerr << "\tIf you don't pass server-port, it defaults to " << kDefaultServerPort << "." << endl;
    return 1;
  }
  
  // ホスト名取得
  const char* pcHost = argv[1];
  int nPort = kDefaultServerPort;
  if (argc >= 3) {
    nPort = atoi(argv[2]);
  }
  
  int nNumArgsIgnored = (argc - 3);
  if (nNumArgsIgnored > 0) {
    cerr << nNumArgsIgnored << " extra argument" <<
      (nNumArgsIgnored == 1 ? "" : "s") << " ignored.  FYI." << endl;
  }
  
  // Start Winsock up
  WSAData wsaData;
  int nCode;
  if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
    cerr << "WSAStartup() returned error code " << nCode << "." << endl;
    return 255;
  }
  
  int retval = DoWinsock(pcHost, nPort);
  
  // Shut Winsock back down and take off.
  WSACleanup();
  return retval;
}
