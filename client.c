#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void loop(char* server_address, int server_listening_port);
char* readString();
char* getEmptyString();
void checkValidPtr(char* ptr);
char** split(char* inputStr);
char* send_to_server(char* server_address, int server_listening_port, char* text);


int main(int argc, char** argv)
{
    // We can let argv[1] = "localhost" and argv[2] = 30000
    if (argv[1] == NULL || argv[2] == NULL)
    {
        perror("Error: Must provide argument for server domain name and the port number that the server is listening to.\n");
        exit(1);
    }

    loop(argv[1], atoi(argv[2]));
}

void loop(char* server_address, int server_listening_port)
{
    char* input;
    char* server_response;

    do
    {
        printf("> ");
        input = readString();

        if (input == NULL)
        {
            printf("Error: Invalid Input\n");
            continue;
        }
        else if (strcmp(input, "quit") == 0)
        {
            server_response = send_to_server(server_address, server_listening_port, input);
            
            exit(0);
        }
        else
        {
            server_response = send_to_server(server_address, server_listening_port, input);
            printf("Received from server: %s\n", server_response);
        }

        free(input);

    } while (1);
}

char* readString() 
{
    char* str = getEmptyString();

    int ch, str_len = 0;
    while ((ch = getchar()) != EOF) 
    {
        if (ch == '\n')
            break;
        
        str_len++;
        str = realloc(str, (str_len + 1) * sizeof(char));
        checkValidPtr(str);
        
        str[str_len - 1] = ch;
        str[str_len] = '\0';
    }

    return str;
}

char* getEmptyString()
{
    char* emptyStr = malloc(sizeof(char));
    checkValidPtr(emptyStr);

    *emptyStr = '\0';

    return emptyStr;
}

void checkValidPtr(char* ptr)
{
    if (ptr == NULL)
    {
        printf("Error: Unable to process user input");
        exit(1);
    }
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

char* send_to_server(char* server_address, int server_listening_port, char* text) 
{
    // Creates the client socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) 
    {
        perror("Error: Unable to create client socket.\n");
        exit(1);
    }

    // Sets up the server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_listening_port);

    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr) <= 0) 
    {
        perror("Error: Invalid IP address\n");
        exit(1);
    }

    // Connects to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("Error: Connection Failed.\n");
        exit(1);
    }

    // Sends the message
    if (send(sock, text, strlen(text), 0) < 0) 
    {
        perror("Error: Send Failed");
        exit(1);
    }

    // Receives the response from the server
    char* response = malloc(1024 * sizeof(char));
    int len = recv(sock, response, sizeof(response) - 1, 0);
    if (len < 0) 
    {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
    response[len] = '\0';

    close(sock);

    return response;
}