// Server-Client Message System.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Server-Client Message System.h"


using namespace std;
ofstream logFile;


void ClientSide();
void ServerSide();

void CreateSocket(bool serverSide = false);
void ConnectToServer(int programType = 1);
void SendAndRecieve(bool serverSide = false);
void CloseConnection();
void GetPortandIP();
void ReportError(int lineNum, string text, int errorCode = 0);
char* GetDateTime(bool formated = true);

SOCKET listenSocket = INVALID_SOCKET;
SOCKET newSocket = INVALID_SOCKET;

sockaddr_in newSock;

struct addrinfo* result = NULL,
	* ptr = NULL,
	hints;

int portNum, sockRead, iResult;

char* newIP = (char*)malloc(sizeof(char) * 15);
char sendBuffer[200] = "test message";
char recvBuffer[200]; //= (char*)malloc(200);

string user;

bool systemOn = true;
bool connected = false;




int main()
{
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult !=0 )
	{
		cout << "WSAStarup Failed";
		ReportError(__LINE__, "WSAStarup Failed ", WSAGetLastError());
	}
	GetPortandIP();

	//loop to ensure correct input
	while (systemOn)
	{
		int switchValue;
		cout << "What Type Of System\n";
		cout << "1. Client\n";
		cout << "2. Server\n";
		cout << "3. Quit\n";
		cin >> switchValue;

		//starts program as either server or client given user input
		switch (switchValue)
		{
		case 1:
			user = "Client";
			ClientSide();
			break;
		case 2:
			user = "Server";
			ServerSide();
			break;
		case 3:
			systemOn = false;
		}
	}
	return 0;
}

void ClientSide()
{

	cout << "WELCOME CLIENT SIDE OPERATIONS\n\n";
	cout << "IP Address and Port can be editied in the config file\n";
	cout << "IP: " << newIP << " | " << "Port: " << portNum << endl;

	CreateSocket();
}

void ServerSide()
{
	cout << "WELCOME SERVER SIDE OPERATIONS\n\n";
	cout << "IP Address and Port can be editied in the config file\n";
	cout << "IP: " << newIP << " | " << "Port: " << portNum << endl;

	//calls the CreateSocket function as the server
	CreateSocket(true);
}

void CreateSocket(bool serverSide)
{
	const char* ipNum = newIP;

	newSock.sin_family = AF_INET;
	newSock.sin_port = htons(portNum);
	newSock.sin_addr.s_addr = inet_addr(newIP);
	inet_pton(AF_INET, ipNum, &newSock.sin_addr);
	
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		cout << "ERROR";
		ReportError(__LINE__, "Socket Creation Failed %d", WSAGetLastError());
	}


	//for the server, checks for open connections from the given socket info
	if (serverSide)
	{
		//creates a socket from the given params if bind returns 1
		iResult = bind(listenSocket, (SOCKADDR*)&newSock, sizeof(newSock));
		if (iResult == SOCKET_ERROR)
		{
			cout << "Bind Failed";
			ReportError(__LINE__, "Bind Failed ", WSAGetLastError());
		}

		iResult = listen(listenSocket, 5);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Listen Failed";
			ReportError(__LINE__, "Listen Failed ", WSAGetLastError());
		}

		ConnectToServer(2);
	}
	else
	{
		ConnectToServer(1);
	}
}

void ConnectToServer(int programType)
{
	switch (programType)
	{
	case 1:	//FOR CLIENT
		//searches for connection to server
		if (connect(listenSocket, (struct sockaddr*)&newSock, sizeof(newSock)) < 0)
		{
			cout << "Could not connect ";
			ReportError(__LINE__, "Connection Failed ", WSAGetLastError());
		}
		connected = true;

		SendAndRecieve();

		break;

	case 2: //FOR SERVER
		//creates a connection between server and client 
		int addrLen = sizeof(newSock);
		newSocket = accept(listenSocket, (SOCKADDR *)&newSock, &addrLen);
		if (newSocket == INVALID_SOCKET)
		{
			cout << "No Accept";
			ReportError(__LINE__, "Accept connection failed. ", WSAGetLastError());
		}
		else
		{
			cout << "\nCONNECTED\n";
			connected = true;
			closesocket(listenSocket);
			SendAndRecieve(true);
		}
		break;
	}
}

void SendAndRecieve(bool serverSide)
{
	//sendBuffer = (char*)malloc(200);
	//recvBuffer = 
	char* tempBuffer = (char*)malloc(200);
	//int currentTime = (int)GetDateTime(false);

	while (connected)
	{
		/*if ((int)GetDateTime(false) == currentTime + 3 )
		{*/
			if (serverSide)
			{
				//cout << "-Enter Message- ";
				//cout << "-Or 'Close Connection'-\n";
				//fgets(sendBuffer, 100, stdin);
				iResult = send(newSocket, sendBuffer, 100, 0); //sends data to client/server
				if (iResult == SOCKET_ERROR)
				{
					cout << "Send Failed ";
					ReportError(__LINE__, "Send Failed ", WSAGetLastError());
				}
				if (iResult > 0)
				{
					serverSide = !serverSide;
				}
				else
				{
					cout << "INVALID SEND - NOTHING PASSED";
					cout << "-Enter Message- ";
					cout << "-Or 'E to Exit'-\n";
					fgets(tempBuffer, 100, stdin);
				}
			}
			int recvResult = recv(newSocket, recvBuffer, sizeof(recvBuffer), 0); //reads buffer of open connect to see if anything is present and prints a result if true
			if (recvResult < 0)
			{
				cout << "Receve Error " << WSAGetLastError();
				//ReportError(__LINE__, "Receve Error ", WSAGetLastError());
			}
			if (recvResult > 0)
			{
				cout << recvBuffer;
				ReportError(__LINE__, recvBuffer);
			}
			if (recvResult > 0 && serverSide == false)
			{
				serverSide = !serverSide;
			}

			if (sendBuffer == "Close Connection" || tempBuffer == "Close Connection")
			{
				connected = false;
			}

			if (GetKeyState('E') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
			{
				string userResponse;
				cout << "E Detected, Wanna Exit? - Y/N \n";
				cin >> userResponse;
				if (userResponse == "Y" || userResponse == "y")
				{
					connected == false;
					break;
				}
			}
		//}
	}

	//free(sendBuffer);
	CloseConnection();
}

void CloseConnection()
{
	iResult = shutdown(newSocket, SD_SEND);
	if (iResult == INVALID_SOCKET)
	{
		cout << "Shutdown Failure ";
		ReportError(WSAGetLastError(), "Shutdown Failure: ");
	}

	closesocket(newSocket); //closes the open sockets

	WSACleanup(); //standard winsock function to cleanup after and close all existing socket resources	
	exit(EXIT_FAILURE);
}


void GetPortandIP()
{
	ifstream configFile;

	string ip;

	int port;

	configFile.open("config.txt", ios::in);
	configFile.seekg(4, ios::beg);
	configFile >> ip;


	configFile.seekg(21);
	configFile >> port;

	newIP = (char*)malloc(sizeof(ip));
	char* castedIP = strcpy(new char[ip.length() + 1], ip.c_str());

	for (size_t i = 0; i < sizeof(ip); i++)
	{
		newIP[i] = castedIP[i];
	}

	portNum = port;
}


//error file function that outputs a list of errors whilst runnin the porgram
void ReportError(int lineNum, string text, int errorCode)
{
	string file = user + " " +"log.txt";

	logFile.open(file, ios::app);
	if (errorCode > 0)
	{
		logFile << user << "- " << text << " " << "line: " << lineNum << " " << "Error Code: " << errorCode << " " << GetDateTime() << endl;
		CloseConnection();
	}
	else
	{
		logFile << user << "- " << text << " " << "line: " << lineNum << "" << GetDateTime() << endl;
	}
	//exit(EXIT_FAILURE);
}

char* GetDateTime(bool formated)
{
	if (formated)
	{
		//usues the time_t api to get the current system time
		time_t now = time(0);
		//assigns time to data as a pointer
		char* date = ctime(&now);

		return date;
	}
	else
	{
		time_t currentTime = time(0);
		struct tm* timeSince = gmtime(&currentTime);
		return (char*)timeSince->tm_sec;
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
