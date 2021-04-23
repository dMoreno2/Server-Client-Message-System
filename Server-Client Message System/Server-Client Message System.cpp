// Server-Client Message System.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Server-Client Message System.h"


using namespace std;
ofstream logFile;
ofstream messageLog;


void ProgramStart(int switchValue);

void CreateSocket(bool serverSide = false);
//void ConnectToServer(int programType = 1);
char* MakeMessage(int seq, int count, bool specialVal = false);
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

int currentTime = 0, previousTime = 0, delayTime = 0;

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


char* MakeMessage(int seq, int count, bool specialVal)
{
	SYSTEMTIME lt;

	GetLocalTime(&lt);

	string ack = "ACK";
	string R = "R";
	string E = "E";
	string recieved = "Recieved at:";
	string seqNum = "00" + to_string(seq);
	string milSeconds = to_string(lt.wMilliseconds);
	string seconds = to_string(lt.wSecond);
	string mins = to_string(lt.wMinute);
	string hours = to_string(lt.wHour);
	string text;

	if (specialVal)
	{
		if (count % 3 == 0)
		{
			text = E + " " + seqNum + " " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
			// R E 002 12:12:122

			//create char pointer at the size of text to hold text passed out of function
			char* temp = (char*)malloc(sizeof(text));

			//copies text onto the char pointer temp
			temp = strcpy(new char[text.length() + 1], text.c_str());

			//passes temp back out to send
			return temp;
		}
		if (count % 3 == 1)
		{
			text = ack + " " + E + " " + seqNum + " " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
			// ACK E 002 12:12:122

			//create char pointer at the size of text to hold text passed out of function
			char* temp = (char*)malloc(sizeof(text));

			//copies text onto the char pointer temp
			temp = strcpy(new char[text.length() + 1], text.c_str());

			//passes temp back out to send
			return temp;
		}
		if (count % 3 == 2)
		{
			text = ack + " " + seqNum + " " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
			// E 002 12:12:122

			//create char pointer at the size of text to hold text passed out of function
			char* temp = (char*)malloc(sizeof(text));

			//copies text onto the char pointer temp
			temp = strcpy(new char[text.length() + 1], text.c_str());

			//passes temp back out to send
			return temp;
		}
	}

	if (count % 4 == 0)
	{
		//request text
		text = R + " - " + seqNum + " - " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
		previousTime = stoi(seconds) * 1000;
		previousTime += stoi(milSeconds);
		//cout << "Initial Time:" << previousTime << endl;
		// R - 001 - 12:12:122
	}
	if (count % 4 == 1)
	{
		//accepted request text
		text = ack + " - " + R + " - " + seqNum + " - " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
		// ACK - R - 001 - 12:12:122
	}
	if (count % 4 == 2)
	{
		// Acknowleged request accepted text
		text = ack + " - " + R + " - " + seqNum + " - " + recieved + " " + hours + ":" + mins + ":" + seconds + ":" + milSeconds;
		currentTime = stoi(seconds) * 1000;
		currentTime += stoi(milSeconds);
		//cout << "Current Time:" << currentTime << endl;
		// ACK - R - 001 - recived at: 12:12:122
	}
	if (count % 4 == 3)
	{
		delayTime = (currentTime - previousTime);
		//cout << "Delay:" << delayTime << endl;
		text = ack + " - " + seqNum + " - " + hours + ":" + mins + ":" + seconds + ":" + milSeconds + " - " + "Round Trip Delay: " + to_string(delayTime) + " Milliseconds";
		// ACK - 001 - 12:12:122 - Round Trip Delay: 1 seond 
		//to do: roudtrip value calc

	}


	//create char pointer at the size of text to hold text passed out of function
	char* temp = (char*)malloc(sizeof(text));

	//copies text onto the char pointer temp
	temp = strcpy(new char[text.length() + 1], text.c_str());

	//passes temp back out to send
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
		//checks if E check is pressed
		if (user == "Server" && GetKeyState('E') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
		{
			string userResponse;
			cout << "E Detected, Wanna Exit? - Y/N \n";
			//gets user input
			cin >> userResponse;
			if (userResponse == "Y" || userResponse == "y")
			{
				count = 0;
				seqNum++;

				//generates End message, sends to partner, prints message to screen, adds message to queue, waits 3 seconds 
				tempBuffer = MakeMessage(seqNum, count, true);
				sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
				//cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				Sleep(1000);

				count += 2;

				//recives message, prints message to screen, adds message to queue, waits 3 seconds
				recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
				cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				Sleep(1000);

				//generates End message, sends to partner, prints message to screen, adds message to queue, waits 3 seconds 
				tempBuffer = MakeMessage(seqNum, count, true);
				sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
				//cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				Sleep(1000);

				//recives message, prints message to screen, adds message to queue, waits 3 seconds
				recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
				cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				Sleep(1000);

				//generates End message, sends to partner, prints message to screen, adds message to queue, waits 3 seconds 
				tempBuffer = MakeMessage(seqNum, count, true);
				sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
				//cout << "MESSAGE: " << recvBuffer << endl;
				textQueue.push(recvBuffer);
				Sleep(1000);

				connected == false;

			}
			break;
		}

		//checks if running program as sever or client
		switch (serverSide)
		{
		case true:
			//checks that bytes are recieved from recvfrom, returns size in bytes or -1 if error
			recvResult = recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
			if (recvResult < 0)
			{
				//passes out error code t screen
				cout << "Receve Error " << WSAGetLastError();
			}
			if (recvResult > 0)
			{
				//printes recieved message
				cout << "MESSAGE: " << recvBuffer << endl;
				//adds text recived to a queue for writing later
				textQueue.push(recvBuffer);
				//Sleep(3000);
				serverSide = !serverSide;
			}
			if (recvResult == 0 && recvBuffer[0] == 'E')
			{
				serverSide = true;
			}
			break;

		case false:
			//creates message to be sent to the partner
			tempBuffer = MakeMessage(seqNum, count);
			//gets the value of sendto, returns value in bytes or -1 if error occures
			//sends message defined in tempbuffer to partner
			iResult = sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
			if (iResult == SOCKET_ERROR)
			{
				//output error
				cout << "Send Failed ";
				ReportError(__LINE__, "Send Failed ", WSAGetLastError());
			}
			if (iResult > 0)
			{
				//makes thread wait for 3 seconds 
				//switches sending and recieveing function
				serverSide = !serverSide;
				//increments count by 1 
				count++;
				//checks if count is equvilant to 0 
				if (count % 4 == 0)
				{
					// R - 001 - 12:12:122
					//Sleep(1000);
					seqNum++;
				}
				//checks if count is equvilant to 4
				if (count % 4 == 1)
				{
					// ACK - R - 001 - 12:12:122
					//Sleep(1000);
					//calc roundtrip
				}
				//checks if count is equvilant to 4
				if (count % 4 == 2)
				{
					// ACK - R - 001 - recived at: 12:12:122
				}
				//checks if count is equvilant to 4
				if (count % 4 == 3)
				{
					// ACK - 001 - 12:12:122 - Round Trip Delay: XX seconds
					Sleep(3000);
					//calc roundtrip
				}
			}
			break;
		}

		if (recvBuffer[0] == 'E' && user == "Client")
		{
			count = 1;
			//seqNum++;

			//recives message, prints message to screen, adds message to queue, waits 3 seconds
			recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
			cout << "MESSAGE: " << recvBuffer << endl;
			textQueue.push(recvBuffer);
			Sleep(1000);

			//generates End message, sends to partner, prints message to screen, adds message to queue, waits 3 seconds 
			tempBuffer = MakeMessage(seqNum, count, true);
			sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
			//cout << "MESSAGE: " << recvBuffer << endl;
			textQueue.push(recvBuffer);
			Sleep(1000);

			//recives message, prints message to screen, adds message to queue, waits 3 seconds
			recvfrom(listenSocket, recvBuffer, 100, 0, (struct sockaddr*)&otherSock, &sockLen);
			cout << "MESSAGE: " << recvBuffer << endl;
			textQueue.push(recvBuffer);
			Sleep(1000);

			////generates End message, sends to partner, prints message to screen, adds message to queue, waits 3 seconds 
			//tempBuffer = MakeMessage(seqNum, count);
			//sendto(listenSocket, tempBuffer, 100, 0, (struct sockaddr*)&otherSock, sockLen);
			////cout << "MESSAGE: " << recvBuffer << endl;
			//textQueue.push(recvBuffer);
			//Sleep(1000);

			connected == false;
			break;
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


	configFile.seekg(ip.length()+12);
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
			messageLog << user << "-" << textQueue.front() << endl;
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
