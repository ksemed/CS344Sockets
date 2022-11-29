#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define MAX 150000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char* msg) {
    perror(msg);
    exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
    int portNumber,
    char* hostname) {

    // Clear out the address struct
    memset((char*)address, '\0', sizeof(*address));
    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == NULL) {
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*)&address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char* argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[MAX];
    char buffer2[MAX];
    // Check usage & args
    if (argc < 3) {
        fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
        exit(0);
    }


    char* texttitle = argv[1];
    char* keytitle = argv[2];

    FILE* text = fopen(texttitle, "r");
    FILE* key = fopen(keytitle, "r");

    char textkey[MAX];
    char textkey2[MAX];

    // Input line to make sure we're dealing with enc
    strcat(textkey, "dec");
    strcat(textkey, "\n");

    strcat(textkey2, "dec");
    strcat(textkey2, "\n");

    // Record the text

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Get input from the user, trunc to buffer - 1 chars, leaving \0
    fgets(buffer, sizeof(buffer) - 1, text);
    // Remove the trailing \n that fgets adds
    buffer[strcspn(buffer, "\n")] = '\0';

    for (int i = 0; i < strlen(buffer); i++)
    {
        if ((buffer[i] < 'A' || buffer[i] > 'Z') && buffer[i] != ' ' && buffer[i] != '\n')
        {
            fprintf(stderr, "Error, plaintext has an invalid char\n");
            exit(1);
        }
    }

    // We need to split in order to accomodate for plaintext4
    char split1[MAX];
    char split2[MAX];

    for (int i = 0; i < strlen(buffer) / 2; i++)
    {
        split1[i] = buffer[i];
    }

    int j = 0;
    for (int i = strlen(buffer) / 2; i < strlen(buffer); i++)
    {
        split2[j] = buffer[i];
        j++;
    }

    int textlen = strlen(buffer);

    strcat(textkey, split1);
    strcat(textkey, "\n");

    strcat(textkey2, split2);
    strcat(textkey2, "\n");

    // Record the key

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    // Get input from the user, trunc to buffer - 1 chars, leaving \0
    fgets(buffer, sizeof(buffer) - 1, key);
    // Remove the trailing \n that fgets adds
    buffer[strcspn(buffer, "\n")] = '\0';

    for (int i = 0; i < strlen(buffer); i++)
    {
        if ((buffer[i] < 'A' || buffer[i] > 'Z') && buffer[i] != ' ' && buffer[i] != '\n')
        {
            fprintf(stderr, "Error, key has an invalid char\n");
            exit(1);
        }
    }

    int keylen = strlen(buffer);

    strcat(textkey, buffer);
    strcat(textkey, "\n");

    strcat(textkey2, buffer);
    strcat(textkey2, "\n");

    if (keylen < textlen)
    {
        fprintf(stderr, "Key length is too short\n");
        exit(1);
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    // Send plaintext and key to server
    // Write to the server
    charsWritten = send(socketFD, textkey, strlen(textkey), 0);
    if (charsWritten < 0) {
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer)) {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }


    if (strcmp(buffer, "") == 0)
    {
        error("Error\n");
    }

    charsWritten = send(socketFD, textkey2, strlen(textkey2), 0);
    if (charsWritten < 0) {
        error("CLIENT: ERROR writing to socket");
    }
    if (charsWritten < strlen(buffer)) {
        printf("CLIENT: WARNING: Not all data written to socket!\n");
    }

    charsRead = recv(socketFD, buffer2, sizeof(buffer2) - 1, 0);
    if (charsRead < 0) {
        error("CLIENT: ERROR reading from socket");
    }

    strcat(buffer, buffer2);
    //strcat(buffer, "\n");

    printf("%s\n", buffer);

    //printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

    // Close the socket
    close(socketFD);
    return 0;
}