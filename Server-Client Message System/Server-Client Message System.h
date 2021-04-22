// Server-Client Message System.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <fstream>


// TODO: Make sure defs work

#ifdef _WIN32 || _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef __unix__
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> 
#include <arpa/inet.h>
#endif