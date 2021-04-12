// Server-Client Message System.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Server-Client Message System.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS


using namespace std;
ofstream logFile;

void ClientSide();
void ServerSide();

void CreateSocket(bool serverSide = false);
void ConnectToServer(int programType = 1);
void SendAndRecieve(bool serverSide = false);
void CloseConnection();
void ReportError(int lineNum, string text);

struct sockaddr_in newSock;
struct in_addr newAddress;

int sockId, newSocket, portNum, sockRead;

char* newIP = (char*)malloc(10);
char* buffer;

bool systemOn = true;
bool connected = false;


int main()
{
	//loop to ensure correct input
	while (systemOn)
	{
		int switchValue;
		cout << "What Type Of System\n";
		cout << "1. Client\n";
		cout << "2. Server\n";
		cin >> switchValue;

		//starts program as either server or client given user input
		switch (switchValue)
		{
		case 1:ClientSide();
			break;
		case 2:ServerSide();
			break;
		}
	}
	return 0;
}

void ClientSide()
{
	cout << "WELCOME CLIENT SIDE OPERATIONS\n\n";
	cout << "Please Enter Port Number: ";
	cin >> portNum;

	cout << "\nPlease Enter IP Number: ";
	cin >> newIP;

	CreateSocket();
}

void ServerSide()
{
	cout << "WELCOME SERVER SIDE OPERATIONS\n\n";
	cout << "IP Address can be editied in the header file\n";
	cout << "Please Enter Port Number: ";
	cin >> portNum;

	cout << "\nPlease Enter IP Number: ";
	cin >> newIP;

	//calls the CreateSocket function as the server
	CreateSocket(true);
}

void CreateSocket(bool serverSide)
{
	const char* ipNum = newIP;

	sockId = socket(AF_INET, SOCK_STREAM, 0);

	if (sockId == NULL)
	{
		cout << "ERROR";
		ReportError(__LINE__, "Socket Creation Failed ");
	}

	newSock.sin_family = AF_INET;
	newSock.sin_port = htons(portNum);
	newSock.sin_addr.s_addr = INADDR_ANY;

	//for the server, checks for open connections from the given socket info
	if (serverSide)
	{
		//creates a socket from the given params if bind returns 1
		if (bind(sockId, (struct sockaddr*)&newSock, sizeof(newSock)) < 0)
		{
			cout << "Bind Failed";
			ReportError(__LINE__, "Bind Failed ");
		}

		listen(sockId, 5);
		ConnectToServer(2);
	}
}

void ConnectToServer(int programType)
{
	switch (programType)
	{
	case 1:
		//searches for connection to server
		if (connect(sockId, (struct sockaddr*)&newSock, sizeof(newSock)) < 0)
		{
			cout << "Could not connect";
			ReportError(__LINE__, "Connection Failed ");
		}
		connected = true;

		SendAndRecieve();

		break;

	case 2:
		//creates a connection between server and client 
		if (newSocket = accept(sockId, (struct sockaddr*)&newSock, (int*)sizeof(newSock)) < 0)
		{
			cout << "No Accept";
			ReportError(__LINE__, "Accept connection failed. ");
		}
		else
		{
			connected = true;
			SendAndRecieve();
		}
		break;
	}
}

void SendAndRecieve(bool serverSide)
{
	while (connected)
	{
		buffer = (char*)malloc(200);

		if (sockRead = recv(newSocket, buffer, 200, 0) > 0) //reads buffer of open connect to see if anything is present and prints a result if true
		{
			cout << sockRead;
		}

		cout << "-Enter Message-\n";
		cout << "-Or 'Close Connection'-\n";
		fgets(buffer, 100, stdin);

		if (buffer == "Close Connection")
		{
			connected = false;
			systemOn = false;
		}
		send(newSocket, buffer, 200, 0); //sends data to client/server
	}

	free(buffer);
	CloseConnection();
}

void CloseConnection()
{
	closesocket(newSocket); //closes the open sockets

	WSACleanup(); //standard winsock function to cleanup after and close all existing socket resources	
}


//error file function that outputs a list of errors whilst runnin the porgram
void ReportError(int lineNum, string text)
{
	logFile.open("log.txt", ios::app);
	logFile << text << " " << "line: " << lineNum << endl;
	exit(EXIT_FAILURE);
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
