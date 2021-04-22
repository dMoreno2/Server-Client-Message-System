// Server-Client Message System.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Server-Client Message System.h"


using namespace std;
ofstream logFile;
ofstream messageLog;


void ProgramStart(int switchValue);

void CreateSocket(bool serverSide = false);
//void ConnectToServer(int programType = 1);
char* MakeMessage(int seq, int count);
void SendAndRecieve(bool serverSide = false);
void CloseConnection();
void GetPortandIP();
void GenerateMessageLog(string textString);
void WriteMessagesToFile();
void ReportError(int lineNum, string text, int errorCode = 0);
char* GetDateTime(bool formated = true);

SOCKET listenSocket = INVALID_SOCKET;

struct sockaddr_in newSock, otherSock;

int portNum, sockLen = sizeof(otherSock), iResult;

char* newIP = (char*)malloc(sizeof(char) * 15);
const char* response = "R";
char recvBuffer[200]; //= (char*)malloc(200);

string user;

bool systemOn = true;
bool connected = false;

thread WriteToLog;

queue <string> textQueue;



int main()
{
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << "WSAStarup Failed";
		ReportError(__LINE__, "WSAStarup Failed ", WSAGetLastError());
	}

	thread WriteToLog(WriteMessagesToFile);
	WriteToLog.detach();

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
			ProgramStart(1);
			break;
		case 2:
			user = "Server";
			ProgramStart(2);
			break;
		case 3:
			systemOn = false;
		}
	}

	while (!textQueue.empty())
	{
		printf("Please Wait While Information Is written to the log\n");
		std::this_thread::sleep_for(0.1s);
	}

	printf("Task Finished\n");
	printf("Exiting\n");
	WriteToLog.~thread();

	return 0;
}

void ProgramStart(int switchValue)
{
	cout << "WELCOME TO " << user << " SIDE OPERATIONS\n\n";
	cout << "IP Address and Port can be editied in the config file\n";
	cout << "IP: " << newIP << " | " << "Port: " << portNum << endl;

	switch (switchValue)
	{
	case 1:
		//Client
		CreateSocket();
		break;
	case 2:
		//Server
		CreateSocket(true);
		break;
	}
	//calls the CreateSocket function as the server
}

void CreateSocket(bool serverSide)
{
	const char* ipNum = newIP;

	if (serverSide)
	{
		listenSocket = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		listenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	if (listenSocket == INVALID_SOCKET)
	{
		cout << "ERROR";
		ReportError(__LINE__, "Socket Creation Failed %d", WSAGetLastError());
	}

	if (serverSide)
	{
		newSock.sin_family = AF_INET;
		newSock.sin_addr.s_addr = INADDR_ANY;
		newSock.sin_port = htons(portNum);
	}

	else
	{
		otherSock.sin_family = AF_INET;
		otherSock.sin_port = htons(portNum);
		otherSock.sin_addr.S_un.S_addr = inet_addr(ipNum);
	}

	//for the server, checks for open connections from the given socket info
	if (serverSide)
	{
		//creates a socket from the given params if bind returns 1
		iResult = bind(listenSocket, (struct sockaddr*)&newSock, sizeof(newSock));
		if (iResult == SOCKET_ERROR)
		{
			cout << "Bind Failed";
			ReportError(__LINE__, "Bind Failed ", WSAGetLastError());
		}
		//ConnectToServer(2);

		cout << "\nCONNECTED\n";
		connected = true;
		SendAndRecieve(true);
	}
	else
	{
		//ConnectToServer(1);
		connected = true;
		SendAndRecieve();
	}
}

//void ConnectToServer(int programType)
//{
//	switch (programType)
//	{
//	case 1:	//FOR CLIENT
//		//searches for connection to server
//		if (connect(listenSocket, (struct sockaddr*)&newSock, sizeof(newSock)) < 0)
//		{
//			cout << "Could not connect ";
//			ReportError(__LINE__, "Connection Failed ", WSAGetLastError());
//		}
//		connected = true;
//
//		SendAndRecieve();
//
//		break;
//
//	case 2: //FOR SERVER
//		//creates a connection between server and client 
//		int addrLen = sizeof(newSock);
//		newSocket = accept(listenSocket, (SOCKADDR *)&newSock, &addrLen);
//		if (newSocket == INVALID_SOCKET)
//		{
//			cout << "No Accept";
//			ReportError(__LINE__, "Accept connection failed. ", WSAGetLastError());
//		}
//		else
//		{
//			cout << "\nCONNECTED\n";
//			connected = true;
//			closesocket(listenSocket);
//			SendAndRecieve(true);
//		}
//		break;
//	}
//}

char* MakeMessage(int seq, int count)
{
	SYSTEMTIME lt;

	GetLocalTime(&lt);

	string ack = "ACK";
	string R = "R";
	string E = "E";
	string recieved = "recieved at:";
	string seqNum = to_string(seq);
	string milSeconds = to_string(lt.wMilliseconds);
	string seconds = to_string(lt.wSecond);
	string mins = to_string(lt.wMinute);
	string hours = to_string(lt.wHour);
	string text;

	if (count % 4 == 0)
	{
		text = R + " " + seqNum + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
	}
	if (count % 4 == 1)
	{
		text = ack + " " + R + " " + seqNum + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
	}
	if (count % 4 == 2)
	{
		text = ack + " " + R + " " + seqNum + " " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
	}
	if (count % 4 == 3)
	{
		text = ack + " " + seqNum + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
		//to do: roudtrip value calc
	}
	if (count == 10)
	{
		text = E + " " + seqNum + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
		//to do: roudtrip value calc
	}


	char* temp = (char*)malloc(sizeof(text));

	temp = strcpy(new char[text.length() + 1], text.c_str());

	return temp;
}

void SendAndRecieve(bool serverSide)
{
	char* tempBuffer = (char*)malloc(200);
	int seqNum = 000;
	int count = 0;
	int recvResult;

	while (connected)
	{
		switch (serverSide)
		{
		case true:
			recvResult = recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
			if (recvResult < 0)
			{
				cout << "Receve Error " << WSAGetLastError();
			}
			if (recvResult > 0)
			{
				cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				//Sleep(3000);
				serverSide = !serverSide;
			}
			break;

		case false:
			tempBuffer = MakeMessage(seqNum, count);
			iResult = sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
			if (iResult == SOCKET_ERROR)
			{
				cout << "Send Failed ";
				ReportError(__LINE__, "Send Failed ", WSAGetLastError());
			}
			if (iResult > 0)
			{
				Sleep(3000);
				serverSide = !serverSide;
				count++;
				if (count % 4 == 0)
				{
					seqNum++;
				}
			}
			break;
		}

		if (GetKeyState('E') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
		{
			string userResponse;
			cout << "E Detected, Wanna Exit? - Y/N \n";
			cin >> userResponse;
			if (userResponse == "Y" || userResponse == "y")
			{
				tempBuffer = MakeMessage(seqNum, 10);
				sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
				recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
				sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
				connected == false;
				break;
			}
		}
	}
	CloseConnection();
}

void CloseConnection()
{
	iResult = shutdown(listenSocket, SD_SEND);
	if (iResult == INVALID_SOCKET)
	{
		cout << "Shutdown Failure ";
		ReportError(WSAGetLastError(), "Shutdown Failure: ");
	}

	closesocket(listenSocket); //closes the open sockets

	WSACleanup(); //standard winsock function to cleanup after and close all existing socket resources	
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


void WriteMessagesToFile()
{
	string messageFile = user + " " + "messagelog.txt";
	messageLog.open(messageFile, ios::app);

	while (0 == 0)
	{
		if (!textQueue.empty())
		{
			messageLog << textQueue.front() << endl;
			textQueue.pop();
		}
		this_thread::sleep_for(10ms);
	}
}


//error file function that outputs a list of errors whilst runnin the porgram
void ReportError(int lineNum, string text, int errorCode)
{
	string file = user + " " + "log.txt";

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
		return (char*)timeSince;
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
