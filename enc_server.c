#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define MAX 150000

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
    int connectionSocket, charsRead;
    char buffer[MAX];
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0) {
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while (1) {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
            (struct sockaddr*)&clientAddress,
            &sizeOfClientInfo);
        if (connectionSocket < 0) {
            error("ERROR on accept");
        }

        // Fork here
        pid_t id = fork();
        int status = 0;

        if (id == 0)
        {

            // Get the message from the client and display it
            memset(buffer, '\0', MAX);
            // Read the client's message from the socket
            charsRead = recv(connectionSocket, buffer, sizeof(buffer)-1, 0);
            if (charsRead < 0) {
                error("ERROR reading from socket");
            }

            char check[MAX];
            char text[MAX];
            char key[MAX];
        
            int j = 0;
            int k = 0;

            // Check if client is enc
            for (int i = 0; i < strlen(buffer); i++)
            {
                
                if (buffer[i] == '\n')
                {
                    break;
                }
                check[i] = buffer[i];

            }

            // If client is not enc, then stop
            if (strcmp(check, "enc") != 0)
            {
                fprintf(stderr, "Error Client is not enc\n");
                //break;
            }

            // Split Buffer back to message and key
            for (int i = strlen(check)+1; i < strlen(buffer); i++)
            {

                if (buffer[i] == '\n')
                {                 
                    break;
                }
                text[k] = buffer[i];
                k++;
            }
            
            for (int i = strlen(text)+strlen(check)+2; i < strlen(buffer); i++)
            {

                if (buffer[i] == '\n')
                {
                    break;
                }
                key[j] = buffer[i];
                j++;

            }

            char encrypt[MAX];
            int encryptnum = 0;
            // Encrypt String
            for (int i = 0; i < strlen(text); i++)
            {
                if (text[i] == ' ')
                {
                    encrypt[i] = ' ';
                    continue;
                }

                int textnum = text[i];
                int keynum = key[i];
                textnum = textnum - 65;
                keynum = keynum - 65;
                encryptnum = textnum + keynum;
                if (encryptnum >= 26)
                {
                    encryptnum = encryptnum - 26;
                }
                encryptnum = encryptnum + 65;
                encrypt[i] = encryptnum;               
            }

            // Send Encrypted Message 1
            charsRead = send(connectionSocket, encrypt, strlen(encrypt), 0);
            if (charsRead < 0) {
                error("ERROR writing to socket");
            }

            // Repeat Proccess ----------------------------------------------------------------------------------

            // Recieve
            memset(buffer, '\0', MAX);
            charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
            if (charsRead < 0) {
                error("ERROR reading from socket");
            }

            memset(check, '\0', MAX);
            memset(text, '\0', MAX);
            memset(key, '\0', MAX);

            j = 0;
            k = 0;

            // Check if client is enc
            for (int i = 0; i < strlen(buffer); i++)
            {

                if (buffer[i] == '\n')
                {
                    break;
                }
                check[i] = buffer[i];

            }

            // If client is not enc, then stop
            if (strcmp(check, "enc") != 0)
            {
                fprintf(stderr, "Error Client is not enc\n");
                //break;
            }

            // Split Buffer back to message and key
            for (int i = strlen(check) + 1; i < strlen(buffer); i++)
            {

                if (buffer[i] == '\n')
                {
                    break;
                }
                text[k] = buffer[i];
                k++;
            }

            for (int i = strlen(text) + strlen(check) + 2; i < strlen(buffer); i++)
            {

                if (buffer[i] == '\n')
                {
                    break;
                }
                key[j] = buffer[i];
                j++;

            }

            memset(encrypt, '\0', MAX);
            encryptnum = 0;
            // Encrypt String
            for (int i = 0; i < strlen(text); i++)
            {
                if (text[i] == ' ')
                {
                    encrypt[i] = ' ';
                    continue;
                }

                int textnum = text[i];
                int keynum = key[i];
                textnum = textnum - 65;
                keynum = keynum - 65;
                encryptnum = textnum + keynum;
                if (encryptnum >= 26)
                {
                    encryptnum = encryptnum - 26;
                }
                encryptnum = encryptnum + 65;
                encrypt[i] = encryptnum;
            }

            // Send second half of encrypted message
            charsRead = send(connectionSocket, encrypt, strlen(encrypt), 0);
            if (charsRead < 0) {
                error("ERROR writing to socket");
            }

            // Close the connection socket for this client
            close(connectionSocket);
            exit(0);
        }
        else
        {
            waitpid(id, &status, 1);
            //break;
        }

    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}