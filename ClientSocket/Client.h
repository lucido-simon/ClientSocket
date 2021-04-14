#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define DEFAULT_BUFLEN 512


#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <thread>
#include <string>
#include <ctime>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")


class Client
{
private:
	WSADATA wsaData;
	
	std::clock_t timerPing;
	std::string commandeClient;

	std::string IP;
	std::string port;

	bool bInput;
	bool bWaiting;
	bool bRunning;
	bool bConnected;
	bool bDisconnectedOnPurpose;

	addrinfo* result = NULL;
	addrinfo hints;
	SOCKET ConnectSocket;

public:
	Client();
	void closeConnection();
	int init();
	int start();
	bool waiting();
	int receive(std::string* s);
	int ssend(std::string s, std::string code);
	int ssend(std::string s);
	int shutdownClient();
	void displayMessage(std::string message);
	void waitingCheck();
	void treatWaiting();
	void treatMessage(std::string message, int code);
	void treatInput(std::string input);
	void run();
	void clavier(std::string *entree);
	void disconnect();




};

