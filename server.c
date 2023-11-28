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
#include <netdb.h>

typedef struct 
{
    char* name;
    char** dates;
    char** prices;
} Stock;

typedef struct 
{
    Stock** stocks;
    int size;
} StockList;

void listenAndRespond(int client_socket, StockList* stocks);
char* processRequest(char* client_command, StockList* stocks);
char** split(char* inputStr);
char* addLengthByteToString(char* str);
Stock* read_stock_data(char* filename);
int endsWith(const char *str, const char *suffix);
StockList* init_stock_list();
void append_stock(StockList* stock_list, Stock* stock);
char* get_csv_stock_name(const char *filename);
char* getStockName(Stock* stock);

int main(int argc, char** argv)
{
    // We can let argv[1] = "MSFT.csv" and argv[2] = "TSLA.csv", and argv[3] = 30000
    // if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL)
    // {
    //     perror("Error: Must provide arguments for MSFT.csv, TSLA.csv, and the port number that the server will listen to.");
    //     exit(1);
    // }

    StockList* stocks = init_stock_list();

    char ch[5] = ".csv";
    int index = 0;
    bool csvExists = false;

    // Computer reads stock data from csv files
    while (argv[index] != NULL)
    {
        // True if the argument refers to a csv file
        if (endsWith(argv[index], ch))
        {
            append_stock(stocks, read_stock_data(argv[index]));
            csvExists = true;
        }
            
        index++;
    }

    // Must provide valid command with proper arguments when starting the server
    if (index <= 2 || !csvExists)
    {
        perror("Error: Must provide arguments for at least one csv file, and the port number that the server will listen to.");
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
    server_address.sin_port = htons(atoi(argv[index - 1]));
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) 
    {
        perror("Error: Unable to bind.");
        exit(1);
    }

    // Allows socket to now accept incoming connections from the client
    listen(server_fd, 5);

    printf("server started\n");

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
       listenAndRespond(client_socket, stocks);
    }
}

void listenAndRespond(int client_socket, StockList* stocks)
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
    char* response = processRequest(buffer, stocks);

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

char* processRequest(char* client_command, StockList* stocks)
{
    char num_letters = client_command[0];
    client_command = client_command + 1;

    char* response = malloc(256 * sizeof(char));

    // All data sent must be received in its entirety
    if (num_letters != strlen(client_command))
    {
        strcpy(response, "Error: Corrupted request from client.");

        response = addLengthByteToString(response);
        return response;
    }

    printf("%s\n", client_command);
    
    char** args = split(client_command);

    if (args == NULL || *args == NULL)
    {
        char* temp = malloc(24 * sizeof(char));
        strcpy(temp, "Invalid syntax");

        temp = addLengthByteToString(temp);
        return temp;
    }
    
    if (strcmp(args[0], "quit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "List") == 0)
    {
        response[0] = '\0'; 
        char* temp_name;

        for (int i = 0; stocks -> stocks[i] != NULL; i++)
        {
            temp_name = getStockName(stocks -> stocks[i]);
            strcat(response, temp_name);

            if (stocks -> stocks[i + 1] != NULL) 
                strcat(response, " | ");
        }
    }
    else if (strcmp(args[0], "Prices") == 0 && args[1] != NULL && args[2] != NULL)
    {
        
    }
    else if (strcmp(args[0], "MaxProfit") == 0 && args[1] != NULL && args[2] != NULL && args[3] != NULL)
    {
        
    }
    else // Client should be responsible for making sure queries are valid before being sent but this is here just in case
    {
        char* temp = malloc(24 * sizeof(char));
        strcpy(temp, "Invalid syntax");

        temp = addLengthByteToString(temp);
        return temp;
    }

    response = addLengthByteToString(response);

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

char* addLengthByteToString(char* str) 
{
    int len = strlen(str);

    memmove(str + 1, str, len + 1);
    str[0] = len;

    return str;
}

Stock* read_stock_data(char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    char line[1024];
    char* dates[1000];
    char* prices[1000];
    int count = 0;

    while (fgets(line, 1024, file)) 
    {
        char* tmp = strdup(line);
        char* tok;
        int i = 0;
        for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n")) 
        {
            if (i == 0) 
                dates[count] = strdup(tok);
            else if (i == 4) 
                prices[count] = strdup(tok);
            i++;
        }
        count++;
        free(tmp);
    }

    Stock* stock = malloc(sizeof(Stock));
    stock->dates = dates;
    stock->prices = prices;
    stock->name = get_csv_stock_name(filename);

    fclose(file);
    return stock;
}

int endsWith(const char *str, const char *suffix) 
{
    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr)
        return 0;

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

StockList* init_stock_list() 
{
    StockList* stock_list = malloc(sizeof(StockList));
    stock_list->stocks = malloc(sizeof(Stock*));
    stock_list->size = 0;
    return stock_list;
}

void append_stock(StockList* stock_list, Stock* stock) 
{
    stock_list->stocks = realloc(stock_list->stocks, (stock_list->size + 2) * sizeof(Stock*));
    stock_list->stocks[stock_list->size] = stock;
    stock_list->stocks[stock_list->size + 1] = NULL;
    stock_list->size++;
}

char* get_csv_stock_name(const char *filename) 
{
    const char *dot = strrchr(filename, '.');

    if (!dot || dot == filename) 
        return "";

    return strndup(filename, dot - filename);
}

char* getStockName(Stock* stock)
{
    return stock -> name;
}