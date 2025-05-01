#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <map>
#include <string>
#include "MGSocket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace MG_Socket {

    static u_short port = 8080;
    static size_t bufferSize = 1024;
    static bool ready = false;
    static bool serverStarted = false;

    static SOCKET serverSocket;

    static std::list<CONNECT_HANDLE> connectHandles;
    static std::list<RECEIVE_HANDLE> receiveHandles;
    static std::list<DISCONNECT_HANDLE> disconnectHandles;

    static std::list<std::thread*> threads;

    void Receive(SOCKET clientSocket);

    bool SocketInit() {
        WSADATA wsaData;

        // Winsock�̏�����
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            return false;
        }
        ready = true;
        return true;
    }

    void SocketUpdate() {
        std::list<std::thread*> _threads = { threads };
        for (auto itr = _threads.begin(); itr != _threads.end(); itr++) {
            std::thread* thread = *itr;
            if (thread->joinable()) {
                thread->join();
                threads.remove(thread);
                delete thread;
            }
        }
    }

    void SocketUninit() {
        ready = false;
        StopServer();
        WSACleanup();
        for (auto itr = threads.begin(); itr != threads.end(); itr++) {
            std::thread* thread = *itr;
            if (thread->joinable()) {
                thread->join();
                delete thread;
            }
        }
        threads.clear();
    }

    std::string GetSelfIPString() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
            return "";
        }

        // �z�X�g���ɑΉ�����A�h���X�����擾
        addrinfo hints = {};
        hints.ai_family = AF_INET;  // IPv4�A�h���X���w��
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* result = nullptr;
        if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
            return "";
        }

        for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            sockaddr_in* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
            char ipAddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ipAddress, INET_ADDRSTRLEN);
            return ipAddress;
        }
        return "";
    }

    bool StartServer() {

        if (serverStarted) {
            return true;
        }

        int result;

        // �T�[�o�[�p�\�P�b�g�̍쐬
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            return false;
        }

        // �T�[�o�[�A�h���X�̐ݒ�
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        // �\�P�b�g���o�C���h
        result = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            closesocket(serverSocket);
            return false;
        }

        // �N���C�A���g����̐ڑ���ҋ@
        result = listen(serverSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            closesocket(serverSocket);
            return false;
        }

        serverStarted = true;
        std::thread* AcceptThread = new std::thread(ServerAccept);
        AcceptThread->detach();
        threads.push_back(AcceptThread);

        return true;
    }

    void StopServer() {
        if (serverStarted) {
            serverStarted = false;
            closesocket(serverSocket);
        }
    }

    void ServerAccept() {

        while (serverStarted) {

            // �N���C�A���g����̐ڑ����󂯓���
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                // �ڑ����s
                continue;
            }
            bool startReceive = true;
            for (auto itr = connectHandles.begin(); itr != connectHandles.end(); itr++) {
                CONNECT_HANDLE handle = *itr;
                handle(clientSocket);
            }

            if (startReceive) {
                std::thread* receiveThread = new std::thread(Receive, clientSocket);
                receiveThread->detach();
                threads.push_back(receiveThread);
            }
        }
    }

    SOCKET Connect(std::string addr) {

        // �N���C�A���g�p�\�P�b�g�̍쐬
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            return 0;
        }

        // �T�[�o�[�A�h���X�̐ݒ�
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, addr.c_str(), &serverAddr.sin_addr);

        // �T�[�o�[�ɐڑ�
        int result = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            closesocket(clientSocket);
            return 0;
        }

        for (auto itr = connectHandles.begin(); itr != connectHandles.end(); itr++) {
            CONNECT_HANDLE handle = *itr;
            handle(clientSocket);
        }

        std::thread* receiveThread = new std::thread(Receive, clientSocket);
        receiveThread->detach();
        threads.push_back(receiveThread);

        return clientSocket;
    }

    void Receive(SOCKET clientSocket) {
        int _bufferSize = bufferSize;
        char* buffer = new char[_bufferSize];

        if (!buffer) {
            // �o�b�t�@�������s
            closesocket(clientSocket);
            return;
        }

        int result;
        while (true) {
            memset(buffer, 0, _bufferSize);
            result = recv(clientSocket, buffer, _bufferSize, 0);
            if (result > 0) {
                for (auto itr = receiveHandles.begin(); itr != receiveHandles.end(); itr++) {
                    RECEIVE_HANDLE handle = *itr;
                    handle(clientSocket, buffer, _bufferSize);
                }
            }
            else {
                // �ڑ��ؒf
                for (auto itr = disconnectHandles.begin(); itr != disconnectHandles.end(); itr++) {
                    DISCONNECT_HANDLE handle = *itr;
                    handle(clientSocket, result);
                }
                break;
            }
        }
        delete[] buffer;
        closesocket(clientSocket);
    }

    void Disconnect(SOCKET clientSocket) {
        closesocket(clientSocket);
    }


    void AddConnectHandle(CONNECT_HANDLE handle) {
        connectHandles.push_back(handle);
    }

    void RemoveConnectHandle(CONNECT_HANDLE handle) {
        connectHandles.remove(handle);
    }

    void AddReceiveHandle(RECEIVE_HANDLE handle) {
        receiveHandles.push_back(handle);
    }

    void RemoveReceiveHandle(RECEIVE_HANDLE handle) {
        receiveHandles.remove(handle);
    }

    void AddDisconnectHandle(DISCONNECT_HANDLE handle) {
        disconnectHandles.push_back(handle);
    }

    void RemoveDisconnectHandle(DISCONNECT_HANDLE handle) {
        disconnectHandles.remove(handle);
    }

    void SocketSendMessage(SOCKET socket, std::string message) {
        send(socket, message.c_str(), strlen(message.c_str()), 0);
    }
}
