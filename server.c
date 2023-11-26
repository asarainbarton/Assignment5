#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void listenAndRespond(int client_socket, char* csv1, char* csv2);
char* processRequest(char* client_command, char* csv1, char* csv2);
char** split(char* inputStr);

int main(int argc, char** argv)
{
    // We can let argv[1] = "MSFT.csv" and argv[2] = "TSLA.csv", and argv[3] = 30000
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    {
        perror("Error: Must provide arguments for MSFT.csv, TSLA.csv, and the port number that the server will listen to.");
        exit(1);
    }

    // Creates a socket represented as the server's file descriptor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) 
    {
        perror("Error: Unable to open socket.");
        exit(1);
    }

    // Allows the server to re-establish connection if the server's process ended and then immediately got reinitiated.
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0)
        perror("Error: Unable for server to establish connection.");

    // Bind the socket to a port
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[3]));
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) 
    {
        perror("Error: Unable to bind.");
        exit(1);
    }

    // Allows socket to now accept incoming connections from the client
    listen(server_fd, 5);

    while(1)
    {
        // Accept a connection
        struct sockaddr_in client_address;
        int client_address_len = sizeof(client_address);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_address, (socklen_t*)&client_address_len);
        if (client_socket < 0) 
        {
            perror("Error: Unable to accept");
            exit(1);
        }

       // Handle the client request in a new thread/process
       listenAndRespond(client_socket, argv[1], argv[2]);
    }
}

void listenAndRespond(int client_socket, char* csv1, char* csv2)
{
    char buffer[256];
    int n;

    // Read the client's request
    n = read(client_socket, buffer, 255);
    if (n < 0) 
    {
        perror("Error: Unable to read request from client");
        exit(1);
    }

    buffer[n] = '\0';

    // Process the request and prepare a response
    char* response = processRequest(buffer, csv1, csv2);

    // Send the response back to the client
    n = write(client_socket, response, strlen(response));
    if (n < 0) 
    {
        perror("Error: Unable to write response back to client");
        exit(1);
    }

    // Close the connection
    close(client_socket);
}

char* processRequest(char* client_command, char* csv1, char* csv2)
{
    printf("%s\n", client_command);
    
    char** args = split(client_command);

    if (args == NULL || *args == NULL)
    {
        free(args);
        char* temp = malloc(24 * sizeof(char));
        strcpy(temp, "Error: Invalid Request");

        return temp;
    }

    if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "List") == 0)
    {
            
    }
    else if (strcmp(args[0], "Prices") == 0 && args[1] != NULL && args[2] != NULL)
    {
            
    }
    else if (strcmp(args[0], "MaxProfit") == 0 && args[1] != NULL && args[2] != NULL && args[3] != NULL)
    {
            
    }
    else 
    {

    }
        

    // Free all memory from splitVals
    for (int i = 0; args[i] != NULL; i++)
        free(args[i]);
    free(args);


    // Temporary response for now
    char* response = malloc(10 * sizeof(char));
    strcpy(response, "received\n");

    return response;
}

char** split(char* inputStr)
{
    char** splitVals = NULL;
    char* str = malloc(sizeof(char));
    str[0] = '\0';

    int count = 0, bigCount = 0;
    
    for (int i = 0; i < strlen(inputStr); i++)
    {
        if (inputStr[i] != ' ' && inputStr[i] != 9)
        {
            count++;
            str = realloc(str, (count + 1) * sizeof(char));
            str[count - 1] = inputStr[i];
            str[count] = '\0';

            if (i == strlen(inputStr) - 1)
            {
                bigCount++;
                splitVals = realloc(splitVals, bigCount * sizeof(char*));
                splitVals[bigCount - 1] = str;
            }

            continue;
        }

        // We don't want empty strings to be part of our array of strings
        if (strlen(str) == 0)
            continue;
        
        bigCount++;
        splitVals = realloc(splitVals, bigCount * sizeof(char*));
        splitVals[bigCount - 1] = str;

        str = malloc(sizeof(char));
        str[0] = '\0';
        
        count = 0;
    }

    // Add a terminating NULL value to the end of the data structure to signify that the end has been reached
    splitVals = realloc(splitVals, (bigCount + 1) * sizeof(char*));
    splitVals[bigCount] = NULL;

    return splitVals;
}
