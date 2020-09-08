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

// winsock�ł̒ʐM����
int DoWinsock(const char* pcAddress, int nPort)
{
    // Listen�J�n
    cout << "Establishing the listener..." << endl;
    SOCKET ListeningSocket = SetUpListener(pcAddress, htons(nPort));
    if (ListeningSocket == INVALID_SOCKET) {
        cout << endl << WSAGetLastErrorMessage("establish listener") <<
            endl;
        return 3;
    }

    // �N���C�A���g����
    while (1) {
        // �ڑ��҂��E�ڑ�����
        cout << "Waiting for a connection..." << flush;
        sockaddr_in sinRemote;
        SOCKET sd = AcceptConnection(ListeningSocket, sinRemote);
        if (sd != INVALID_SOCKET) {
            cout << "Accepted connection from " <<
                inet_ntoa(sinRemote.sin_addr) << ":" <<
                ntohs(sinRemote.sin_port) << "." << endl;
        }
        else {
            cout << endl << WSAGetLastErrorMessage(
                "accept connection") << endl;
            return 3;
        }

        // �p�P�b�g���N���C�A���g����󂯎��
        if (EchoIncomingPackets(sd)) {
            // �ڑ��I��
            cout << "Shutting connection down..." << flush;
            if (ShutdownConnection(sd)) {
                cout << "Connection is down." << endl;
            }
            else {
                cout << endl << WSAGetLastErrorMessage(
                    "shutdown connection") << endl;
                return 3;
            }
        }
        else {
            cout << endl << WSAGetLastErrorMessage(
                "echo incoming packets") << endl;
            return 3;
        }
    }

#if defined(_MSC_VER)
    return 0;       // warning eater
#endif
}

// Listener�̃Z�b�g�A�b�v
SOCKET SetUpListener(const char* pcAddress, int nPort)
{
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

// �ڑ��҂�����
SOCKET AcceptConnection(SOCKET ListeningSocket, sockaddr_in& sinRemote)
{
    int nAddrSize = sizeof(sinRemote);
    return accept(ListeningSocket, (sockaddr*)&sinRemote, &nAddrSize);
}


bool EchoIncomingPackets(SOCKET sd)
{
    // Read data from client
    char acReadBuffer[kBufferSize];
    int nReadBytes;
    do {
        nReadBytes = recv(sd, acReadBuffer, kBufferSize, 0);
        if (nReadBytes > 0) {
            cout << "Received " << nReadBytes <<
                " bytes from client." << endl;

            int nSentBytes = 0;
            while (nSentBytes < nReadBytes) {
                int nTemp = send(sd, acReadBuffer + nSentBytes,
                    nReadBytes - nSentBytes, 0);
                if (nTemp > 0) {
                    cout << "Sent " << nTemp <<
                        " bytes back to client." << endl;
                    nSentBytes += nTemp;
                }
                else if (nTemp == SOCKET_ERROR) {
                    return false;
                }
                else {
                    // Client closed connection before we could reply to
                    // all the data it sent, so bomb out early.
                    cout << "Peer unexpectedly dropped connection!" <<
                        endl;
                    return true;
                }
            }
        }
        else if (nReadBytes == SOCKET_ERROR) {
            return false;
        }
    } while (nReadBytes != 0);

    cout << "Connection closed by peer." << endl;
    return true;
}

