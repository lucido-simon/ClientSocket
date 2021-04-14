#include "Client.h"

Client::Client()
{
	ConnectSocket = INVALID_SOCKET;
	result = NULL;

	commandeClient = "Commandes disponibles pour le client : \n\tDeconnecte :\n\t-quit\n\t-changeip <ip>\n\t-connect\n\n\tConnecte :\n\t-disconnect\n\t-quit\n\n";

	ConnectSocket = INVALID_SOCKET;
	bWaiting = false;
	bInput = false;
	bConnected = false;
	bRunning = true;
	bDisconnectedOnPurpose = true;

	IP = "86.210.102.116";
	port = "27015";

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
}

void Client::closeConnection()
{
	bConnected = false;
	std::cout << "--FERMETURE CONNECTION .." << std::endl;
	shutdown(ConnectSocket, SD_BOTH);
	closesocket(ConnectSocket);
	WSACleanup();
	std::cout << "--CONNECTION FERMEE" << std::endl;
}

int Client::init()
{
	std::cout << "--Initialisation.." << std::endl;
	int iResult;
	wsaData = WSADATA();

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	// Resolve the server address and port
	
	iResult = getaddrinfo(IP.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	return 0;
}

int Client::start()
{
	std::cout << "--Demarrage.." << std::endl;

	int iResult;

	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	std::cout << "--Connection a " << IP <<  std::endl;
	iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);;
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}


	bConnected = true;
	bDisconnectedOnPurpose = false;

	std::cout << "--Connecte.." << std::endl << std::endl << std::endl << std::endl << std::endl;

	return 0;
}

bool Client::waiting()
{
	fd_set rfd;
	FD_ZERO(&rfd);
	FD_SET(ConnectSocket, &rfd);
	timeval t;
	t.tv_usec = 10000;
	int r = select(ConnectSocket + 1, &rfd, NULL, NULL, &t);

	if (r > 0)
	{
		return true;
	}

	return false;

}

int Client::receive(std::string* s)
{
	char recvbuf[2048];
	int iResult;

	iResult = recv(ConnectSocket, recvbuf, 2048, 0);

	if (iResult < 0)
	{
		printf("recv failed with error: %d\n", WSAGetLastError());
		std::cout << "ERREUR receive" << std::endl;
		closeConnection();
		
		return iResult;
	}

	recvbuf[iResult] = NULL;
	*s = recvbuf;

	return iResult;
}

int Client::ssend(std::string s, std::string code)
{
	code += s;
	int sRes = send(ConnectSocket, code.c_str(), code.size(), 0);
	return sRes;
}

int Client::ssend(std::string s)
{
	return ssend(s, "10");
}

int Client::shutdownClient()
{
	disconnect();
	WSACleanup();

	return 0;
}

void Client::displayMessage(std::string message)
{
	message.erase(message.begin(), message.begin() + 2);
	std::cout << message;
}

void Client::waitingCheck()
{
	while (bRunning)
	{
		if (bWaiting == false && bConnected )
			if (waiting())
			{
				bWaiting = true;
			}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Client::treatWaiting()
{
	std::string message;
	receive(&message);

	bWaiting = false;

	if (message.size() < 2)
	{
		std::cout << "--ERROR : message trop court : " << message << std::endl;
		return;
	}

	if (message.find("\n") == std::string::npos)
	{
		std::string t;
		t += message[0];
		t += message[1];
		int code = stoi(t);

		treatMessage(message, code);
	}

	else 
	{
		std::vector<std::pair<std::string, int>> messages;

		while (message.find("\n") != std::string::npos && message.size() > 1)
		{
			std::string codeS;
			std::string subMessage;

			size_t pos = message.find("\n");
			subMessage = message.substr(0, pos + 1);
			message.erase(0, pos + 1);
		
			codeS += subMessage[0];
			codeS += subMessage[1];

			messages.push_back(std::pair<std::string, int>(subMessage, stoi(codeS))); // Si erreur c'est surment que codeS ne peut pas etre cast en int
		}

		for (size_t i = 0; i < messages.size(); i++)
		{
			treatMessage(messages[i].first, messages[i].second);
		}
	}

}

void Client::treatMessage(std::string message, int code)
{
	switch (code)
	{
	case 10:
		displayMessage(message);
		break;

	case 12:
	{
		double duration = (std::clock() - timerPing) / (double)CLOCKS_PER_SEC;
		std::cout << "Ping : " << duration * 1000 << " ms" << std::endl;
	}
	break;

	default:
		std::cout << "--ERROR : erreur de formattage message, code inconnu : ";
		displayMessage(message);
		break;
	}
	bWaiting = false;
}

void Client::treatInput(std::string input)
{

	if (input.size() == 0);
	if (input.find("-ping") == 0)
	{
		if (bConnected)
		{
			ssend(input, "12");
			timerPing = std::clock();
		}
		else std::cout << "--Vous devez etre connecte a un serveur pour utiliser cette commande\n";

	}

	else if (input.find("-quit") == 0)
	{
		bRunning = false;
	}


	else if (input.find("-disconnect") == 0)
	{
		if (bConnected)
			disconnect();
		else std::cout << "--Vous devez etre connecte a un serveur pour utiliser cette commande\n";
	}

	else if (input.find("-changeip") == 0)
	{
		if (bConnected)
			std::cout << "--Vous devez etre deconnecte pour utiliser cette commande\n";
		else
		{
			input.erase(input.begin(), input.begin() + 10);
			IP = input;
		}
	}

	else if (input.find("-connect") == 0)
	{
		if (bConnected)
			std::cout << "--Vous devez etre deconnecte pour utiliser cette commande\n";
		else
		{
			init();
			start();
		}
	}

	else if (input.find("-commandeClient") == 0)
		std::cout << commandeClient;

	else
	{
		if(bConnected)
			ssend(input);
		else std::cout << "--Commande non valide\n";
	}

	bInput = false;
}

void Client::run()
{

	std::string input;

	std::thread t1 = std::thread(&Client::clavier, this, &input);;
	std::thread t2 = std::thread(&Client::waitingCheck, this);;

	std::cout << commandeClient << std::endl;

	while (bRunning)
	{
		while (bConnected && bRunning)
		{
			if (bWaiting)
				treatWaiting();
		
			if (bInput)
				treatInput(input);

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		while (!bConnected && bRunning)
		{
			if (bInput)
				treatInput(input);

			else
			{
				if (!bDisconnectedOnPurpose)
				{
					std::cout << "--Deconnecte, tentative de reconnexion" << std::endl;
					init();
					start();
				}
			}

		}
		
	}

	std::cout << "--ARRET EN COURS.." << std::endl;
	t1.join();
	std::cout << "--THREAD D'ENTREE CLAVIER ARRETER.." << std::endl;
	t2.join();
	std::cout << "--THREAD WAITING ARRETER" << std::endl;

	shutdownClient();
	std::cout << "--SHUTDOWN COMPLET, CLIENT ARRETER" << std::endl;

}

void Client::clavier(std::string* entree)
{
	while (bRunning)
	{
		std::getline(std::cin, *entree);
		bInput = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Client::disconnect()
{
	bDisconnectedOnPurpose = true;
	closeConnection();
}
