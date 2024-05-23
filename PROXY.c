#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <Windows.h>
const int TIME_PORT = 27015;
const char* MAIN_SERVER_HOST = "httpbin.org";
const int MAIN_SERVER_PORT = 80;
int flag=0;
int flag1=0;
// the checkForAnError function checks if there was an error and handles it accordingly
// @ params
// bytesResult = send/receive function result
// ErrorAt = string stating if it was receive or send function
// socket_1 & socket_2 = the sockets to close in case there was an error.



bool checkForAnError(int bytesResult, char* ErrorAt, SOCKET socket_1, SOCKET socket_2)
{
    if (SOCKET_ERROR == bytesResult)
    {
        printf("Time Server: Error at %s(): ",ErrorAt);
        printf("%d", WSAGetLastError());
        closesocket(socket_1);
        closesocket(socket_2);
        WSACleanup();
        return true;
    }
    return false;
}
bool checkForAnError2(int bytesResult, char* ErrorAt, SOCKET socket)
{
    if (SOCKET_ERROR == bytesResult)
    {
        printf("proxy while connecting to MAIN server: Error at %s(): ",ErrorAt);
        printf("%d", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return true;
    }
    return false;
}


void main()
{
    // Create a WSADATA object called wsaData.
    // The WSADATA structure contains information about the Windows
    // Sockets implementation.
    WSADATA wsaData;
    SOCKET listenSocket;
    struct sockaddr_in serverService;
    double interval,interval_rtt ;  // Variable to store RTT
    LARGE_INTEGER frequency, start_RTT, end_RTT, start_proc, end_proc;
    // Call WSAStartup and return its value as an integer and check for errors.
    // The WSAStartup function initiates the use of WS2_32.DLL by a process.
    // First parameter is the version number 2.2.
    // The WSACleanup function destructs the use of WS2_32.DLL by a process.
    if (NO_ERROR != WSAStartup(MAKEWORD(2, 0), & wsaData))
    {
        printf("Time Server: Error at WSAStartup()\n");
        return;
    }
    // Create the listen socket
    listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (INVALID_SOCKET == listenSocket)
    {
        printf("Time Server: Error at socket(): ");
        printf("%d", WSAGetLastError());
        WSACleanup();
        return;
    }

    memset( & serverService, 0, sizeof(serverService));
    // Address family (must be AF_INET - Internet address family).
    serverService.sin_family = AF_INET;
    // IP address. The sin_addr is a union (s_addr is a unsigned long
    // (4 bytes) data type).
    // inet_addr (Iternet address) is used to convert a string (char *)
    // into unsigned long.
    // The IP address is INADDR_ANY to accept connections on all interfaces.
    serverService.sin_addr.s_addr = htonl(INADDR_ANY);
    // IP Port. The htons (host to network - short) function converts an
    // unsigned short from host to TCP/IP network byte order
    // (which is big-endian).
    serverService.sin_port = htons(TIME_PORT);

    // Bind the socket for client's requests.

    // The bind function establishes a connection to a specified socket.
    // The function uses the socket handler, the sockaddr structure (which
    // defines properties of the desired connection) and the length of the
    // sockaddr structure (in bytes).
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR * ) & serverService, sizeof(serverService)))
    {
        printf("Time Server: Error at bind(): ");
        printf("%d", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Listen on the Socket for incoming connections.
    // This socket accepts only one connection (no pending connections
    // from other clients). This sets the backlog parameter.

    //defines the maximum length to which the
    //  queue of pending connections for listenSocket may grow
    if (SOCKET_ERROR == listen(listenSocket, 5))
    {
        printf("Time Server: Error at listen(): ");
        printf("%d", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    while(1)
    {
        // Accept connections and handles them one by one.
        struct sockaddr_in from; // Address of sending partner
        int fromLen = sizeof(from);

        printf("Time Server: Wait for clients' requests.\n");

        // The accept function permits an incoming connection
        // attempt on another socket (msgSocket).
        // The first argument is a bounded-listening socket that
        // receives connections.
        // The second argument is an optional pointer to buffer
        // that receives the internet address of the connecting enrity.
        // the third one is a pointer to the length of the network address
        // (given in the second argument).
        SOCKET msgSocket = accept(listenSocket, (struct sockaddr * ) & from, & fromLen);
        if (INVALID_SOCKET == msgSocket)
        {
            printf("Time Server: Error at accept(): ");
            printf("%d", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return;
        }

        printf("Time Server: Client is connected.\n");
        while (1)
        {
            QueryPerformanceFrequency(&frequency);


            // Send and receive data.
            int bytesSent = 0;
            int bytesRecv = 0;
            char * sendBuff;
            char recvBuff[10000];
            // LARGE_INTEGER frequency;
            // LARGE_INTEGER start,end;
            // Get client's requests and answer them.
            // The recv function receives data from a connected or bound socket.
            // The buffer for data to be received and its available size are
            // returned by recv. The last argument is an idicator specifying the way
            // in which the call is made (0 for default).
            //   QueryPerformanceFrequency(&frequency);
            // QueryPerformanceCounter(&start);

            bytesRecv = recv(msgSocket, recvBuff, 10000, 0);

            QueryPerformanceCounter(&start_proc);


            if (checkForAnError(bytesRecv,"recv",listenSocket,msgSocket))
                return;

            // start processing the client requests.
            if (strncmp(recvBuff, "the anything file", 17) == 0)
            {

                const char *filename = "anything.txt";
                FILE *file = fopen(filename, "rt");
                if (file!=NULL)
                {
                    fseek(file, 0, SEEK_END);
                    long length = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char *buffer = (char *)malloc(length + 1);
                    buffer[length] = '\0';
                    fread(buffer, 1, length, file);
                    fclose(file);

                    QueryPerformanceCounter(&end_proc);

                    printf("\nSending: %s\n",buffer);

                    interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                    printf("\nProcessing time for this request (get anything file): %f \n",interval);

                    bytesSent = send(msgSocket, buffer, (int)strlen(buffer), 0);
                    free(buffer);  // Free the allocated memory when done

                }
                else
                {
                    printf("file is not available in proxy server! checking if it's available in the main server\n");

                    if (flag == 0)
                    {
                        // Connect to the main server
                        SOCKET connSocket;
                        struct sockaddr_in server;
                        connSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                        if (INVALID_SOCKET == connSocket)
                        {
                            printf("Proxy: Error at socket(): %d\n", WSAGetLastError());
                            closesocket(listenSocket);
                            WSACleanup();
                            return;
                        }

                        memset(&server, 0, sizeof(server));
                        server.sin_family = AF_INET;
                        // Resolve the domain name to an IP address
                        struct hostent* host = gethostbyname(MAIN_SERVER_HOST);
                        if (host == NULL)
                        {
                            printf("Proxy: Error resolving hostname: %s\n", MAIN_SERVER_HOST);
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

// Copy the resolved IP address to the server structure
                        memcpy(&server.sin_addr.s_addr, host->h_addr, host->h_length);

                        server.sin_port = htons(MAIN_SERVER_PORT);

                        // Connect to the main server
                        if (SOCKET_ERROR == connect(connSocket, (SOCKADDR *)&server, sizeof(server)))
                        {
                            printf("Proxy: Error at connect(): %d\n", WSAGetLastError());
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }


                        printf("Proxy: Connection with the main server established successfully\n");

                        // Request the file from the main server
                        const char *mainServerRequest = "GET /anything HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n";
                        int requestLength = (int)strlen(mainServerRequest);


                        QueryPerformanceCounter(&start_RTT);

                        int bytesSent = send(connSocket, mainServerRequest, requestLength, 0);
                        if (checkForAnError2(bytesSent, "send", connSocket))
                        {
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

                        // Receive the response from the main server
                        char mainServerResponse[10000];
                        int bytesReceived = recv(connSocket, mainServerResponse, sizeof(mainServerResponse), 0);
                        /////// END OF RTT
                        QueryPerformanceCounter(&end_RTT);
                        interval_rtt = (double)(end_RTT.QuadPart - start_RTT.QuadPart) / frequency.QuadPart;
                        printf("\nRTT between proxy server and main server: %f \n",interval_rtt);

                        if (checkForAnError2(bytesReceived, "recv", connSocket))
                        {
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

                        // Find the position of the empty line that separates headers and body
                        char *bodyStart = strstr(mainServerResponse, "\r\n\r\n");
                        if (bodyStart != NULL)
                        {
                            // Move the pointer to the beginning of the body
                            bodyStart += 4;

                            // Save a copy of the body in the proxy
                            FILE *proxyFile = fopen("anything.txt", "wt");
                            if (proxyFile != NULL)
                            {
                                fwrite(bodyStart, 1, bytesReceived - (bodyStart - mainServerResponse), proxyFile);
                                fclose(proxyFile);
                                printf("Proxy: Copy of the file body saved in the proxy\n");

                                // Send the body back to the client

                                // Procrssing ended
                                QueryPerformanceCounter(&end_proc);
                                interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                                printf("\nProcessing time for request (get anything file): %f \n",interval);

                                bytesSent = send(msgSocket, bodyStart, bytesReceived - (bodyStart - mainServerResponse), 0);
                                if (checkForAnError(bytesSent, "send", listenSocket, msgSocket))
                                {
                                    closesocket(connSocket);
                                    WSACleanup();
                                    return;
                                }
                            }else
                            {
                                QueryPerformanceCounter(&end_proc);
                                interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                                printf("\nProcessing time for request (get anything file): %f \n",interval);

                                printf("Proxy: Error opening proxy file\n");

                            }
                        }
                        else
                        {
                            QueryPerformanceCounter(&end_proc);
                            interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;

                            printf("Proxy: Error parsing main server response - empty line not found\n");

                            printf("\nProcessing time for request (get anything file): %f \n",interval);
                        }

                        closesocket(connSocket);
                        flag = 1; // Set the flag to indicate that the main server is connected
                    }
                    else
                    {
                        sendBuff = "Error connecting to the main server";

                        QueryPerformanceCounter(&end_proc);
                        interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                        printf("\nProcessing time for request (get anything file): %f \n",interval);

                        bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
                    }
                }
            }

            else if (strncmp(recvBuff, "Jason file", 10) == 0)
            {
                const char *filename = "json.txt";
                FILE *file = fopen(filename, "rt");
                if (file)
                {
                    fseek(file, 0, SEEK_END);
                    long length = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    char *buffer = (char *)malloc(length + 1);
                    buffer[length] = '\0';
                    fread(buffer, 1, length, file);
                    fclose(file);

                    printf("Sending: %s\n",buffer);

                    QueryPerformanceCounter(&end_proc);
                    interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                    printf("\nProcessing time for request (get json file): %f \n",interval);

                    bytesSent = send(msgSocket, buffer, (int)strlen(buffer), 0);
                    free(buffer);  // Free the allocated memory when done
                }
                else
                {
                    printf("file is not available in proxy server! checking if it's available in the main server\n");

                    if (flag1 == 0)
                    {
                        // Connect to the main server
                        SOCKET connSocket;
                        struct sockaddr_in server;
                        connSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                        if (INVALID_SOCKET == connSocket)
                        {
                            printf("Proxy: Error at socket(): %d\n", WSAGetLastError());
                            closesocket(listenSocket);
                            WSACleanup();
                            return;
                        }

                        memset(&server, 0, sizeof(server));
                        server.sin_family = AF_INET;
                        // Resolve the domain name to an IP address
                        struct hostent* host = gethostbyname(MAIN_SERVER_HOST);
                        if (host == NULL)
                        {
                            printf("Proxy: Error resolving hostname: %s\n", MAIN_SERVER_HOST);
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

// Copy the resolved IP address to the server structure
                        memcpy(&server.sin_addr.s_addr, host->h_addr, host->h_length);

                        server.sin_port = htons(MAIN_SERVER_PORT);


                        // Connect to the main server
                        if (SOCKET_ERROR == connect(connSocket, (SOCKADDR *)&server, sizeof(server)))
                        {
                            printf("Proxy: Error at connect(): %d\n", WSAGetLastError());
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }


                        printf("Proxy: Connection with the main server established successfully\n");

                        // Request the file from the main server
                        const char *mainServerRequest = "GET /json HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n";
                        int requestLength = (int)strlen(mainServerRequest);


                        QueryPerformanceCounter(&start_RTT);

                        int bytesSent = send(connSocket, mainServerRequest, requestLength, 0);
                        if (checkForAnError2(bytesSent, "send", connSocket))
                        {
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

                        // Receive the response from the main server
                        char mainServerResponse[10000];
                        int bytesReceived = recv(connSocket, mainServerResponse, sizeof(mainServerResponse), 0);

                        QueryPerformanceCounter(&end_RTT);

                        interval_rtt = (double)(end_RTT.QuadPart - start_RTT.QuadPart) / frequency.QuadPart;
                        printf("\nRTT between proxy server and main server (get json file): %f \n",interval_rtt);

                        if (checkForAnError2(bytesReceived, "recv", connSocket))
                        {
                            closesocket(connSocket);
                            WSACleanup();
                            return;
                        }

                        // Find the position of the empty line that separates headers and body
                        char *bodyStart = strstr(mainServerResponse, "\r\n\r\n");
                        if (bodyStart != NULL)
                        {
                            // Move the pointer to the beginning of the body
                            bodyStart += 4;

                            // Save a copy of the body in the proxy
                            FILE *proxyFile = fopen("json.txt", "wt");
                            if (proxyFile != NULL)
                            {
                                fwrite(bodyStart, 1, bytesReceived - (bodyStart - mainServerResponse), proxyFile);
                                fclose(proxyFile);
                                printf("Proxy: Copy of the file body saved in the proxy\n");

                                // Send the body back to the client
                                QueryPerformanceCounter(&end_proc);
                                interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                                printf("\nProcessing time for request (get json file): %f \n",interval);

                                bytesSent = send(msgSocket, bodyStart, bytesReceived - (bodyStart - mainServerResponse), 0);
                                if (checkForAnError(bytesSent, "send", listenSocket, msgSocket))
                                {
                                    closesocket(connSocket);
                                    WSACleanup();
                                    return;
                                }
                            }
                            else
                            {
                                QueryPerformanceCounter(&end_proc);
                                interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;

                                printf("Proxy: Error opening proxy file\n");
                                printf("\nProcessing time for request (get json file): %f \n",interval);
                            }
                        }
                        else
                        {
                            printf("Proxy: Error parsing main server response - empty line not found\n");

                            QueryPerformanceCounter(&end_proc);
                            interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                            printf("\nProcessing time for request: %f \n",interval);
                        }

                        closesocket(connSocket);
                        flag1 = 1; // Set the flag to indicate that the main server is connected
                    }
                    else
                    {
                        sendBuff = "Error connecting to the main server";
                        QueryPerformanceCounter(&end_proc);
                        interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;

                        bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
                        printf("\nProcessing time for request (get json file): %f \n",interval);
                    }
                }

            }
            else if (strncmp(recvBuff, "RTT", 3) == 0)
            {

                // Initialize sendBuff and format the RTT response
                //char rttResponse[100];
                 //sprintf(rttResponse, "RTT response seconds");
                sendBuff = "o";
                printf("Sending: %s\n",sendBuff);

                QueryPerformanceCounter(&end_proc);
                interval = (double)(end_proc.QuadPart - start_proc.QuadPart) / frequency.QuadPart;
                printf("\nProcessing time for request (RTT between client and proxy): %f \n",interval);

                bytesSent = send(msgSocket, sendBuff, (int) strlen(sendBuff), 0);
                if (checkForAnError(bytesRecv,"send",listenSocket,msgSocket))
                    return;
            }
            else
            {
                printf("Time Server: Closing Connection.\n");
                closesocket(msgSocket);
                break;
            }

            fflush(stdin);
            sendBuff="";
            interval_rtt = 0;
        }
    }
    closesocket(listenSocket);
    WSACleanup();
    return;
}
