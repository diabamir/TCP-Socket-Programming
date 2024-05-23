#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <Windows.h>

const int TIME_PORT = 27015;

bool checkForAnError(int bytesResult, char* ErrorAt, SOCKET socket)
{
    if (SOCKET_ERROR == bytesResult)
    {
        printf("Time Client: Error at %s(): ",ErrorAt);
        printf("%d", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return true;
    }
    return false;
}


void main()
{

    LARGE_INTEGER frequency, start, end;
    double interval, rtt_sum =0, rtt_avg =0;



    // Initialize Winsock (Windows Sockets).

    WSADATA wsaData;
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 0), & wsaData))
    {
        printf("Time Client: Error at WSAStartup()\n");
        return;
    }

    // Client side:
    // Create a socket and connect to an internet address.

    SOCKET connSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == connSocket)
    {
        printf("Time Client: Error at socket(): ");
        printf("%d", WSAGetLastError());
        WSACleanup();
        return;
    }

    // For a client to communicate on a network, it must connect to a server.
    // Need to assemble the required data for connection in sockaddr structure.
    // Create a sockaddr_in object called server.
    // insert the server IP and port

    struct sockaddr_in server;
    memset( & server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(TIME_PORT);

    // Connect to server.

    // The connect function establishes a connection to a specified network
    // address. The function uses the socket handler, the sockaddr structure
    // (which defines properties of the desired connection) and the length of
    // the sockaddr structure (in bytes).
    if (SOCKET_ERROR == connect(connSocket, (SOCKADDR * ) & server, sizeof(server)))
    {
        printf("Time Client: Error at connect(): ");
        printf("%d", WSAGetLastError());
        closesocket(connSocket);
        WSACleanup();
        return;
    }


    printf("connection established successfully.");


    // Send and receive data.

    int bytesSent = 0, flag=0;
    int bytesRecv = 0;
    char * sendBuff;
    char recvBuff[10000];
    char option;
    double rtt_data[20];

    for (int i = 0; i < 20; i++)
    {
        rtt_data[i] = 0.0;
    }

    int j=0;

    while (option != '4')
    {

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);

        printf("\nPlease insert an option :\n");
        printf("\n 1 : get the anything file");
        printf("\n 2 : get Jason file");
        printf("\n 3 : get RTT");
        printf("\n 4 : Exit.");
        printf("\n Your option : ");
        scanf("%c", & option);


        switch(option)
        {
        case '1':
            sendBuff = "the anything file";
            break;
        case '2':
            sendBuff = "Jason file";
            break;
        case '3':
            sendBuff = "RTT";
            flag =1;
            break;
        case '4':
            sendBuff = "Exit";
            break;
        default:
            printf("\n *-*-* Please enter a valid option only. *-*-*\n");
            fflush(stdin);
            continue;
        }

        bytesSent = send(connSocket, sendBuff, (int) strlen(sendBuff), 0);
        if (checkForAnError(bytesSent,"send",connSocket))
            return;

        // if a request for exit was made, there is no need to recv a response from server, only send.
        if(option == '4')
        {
            printf("Closing connection.");
            break;
        }


        bytesRecv = recv(connSocket, recvBuff, 10000, 0);
        if (checkForAnError(bytesRecv,"recv",connSocket))
            return;


        QueryPerformanceCounter(&end);
        interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;

        rtt_data[j] = interval;


        if(flag ==0)
        {
            printf("\nRecieved from server: %s \n",recvBuff);
        }
        else if(flag == 1)
        {
            for(int x =0; x<j; x++)
            {
                rtt_sum+= rtt_data[x];
                printf("\nRequst number: %d  RTT:%lf\n", x+1, rtt_data[x]);
            }

            printf("\nThe RTT Average of all requests: %f\n", (rtt_sum/j));

            flag=0;
        }
        fflush(stdin);
        sendBuff="";
        memset(recvBuff, 0, 10000);

        j++;
    }
}
