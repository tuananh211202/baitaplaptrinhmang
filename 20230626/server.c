#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{
    int sockfd, newsockfd, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *response;
    char *http_response =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html\n"
        "\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<title>Simple HTTP Server</title>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Hello, World!</h1>\n"
        "</body>\n"
        "</html>\n";

    // Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(sockfd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char *response;
        // Accept a new connection
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Read the request from the client
        valread = read(newsockfd, buffer, 1024);
        if (valread < 0)
        {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        // Parse the request
        char *method = strtok(buffer, " ");
        char *path = strtok(NULL, " ");
        strtok(NULL, " "); // Skip the HTTP version
        if (strcmp(method, "GET") == 0)
        {
            // Serve the requested file
            char *filename = path + 1; // Skip the leading '/'
            if (*filename == '\0')
            {
                filename = "index.html";
            }

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                response = "HTTP/1.1 404 Not Found\nContent-Length: 0\n\n";
            }
            else
            {
                fseek(file, 0, SEEK_END);
                int file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                response = malloc(file_size + strlen(http_response));
                sprintf(response, "%sContent-Length: %d\n\n", http_response, file_size);

                fread(response + strlen(response), file_size, 1, file);
                fclose(file);
            }
        }
        else
        {
            response = "HTTP/1.1 501 Not Implemented\nContent-Length: 0\n\n";
        }

        // Send the response to the client
        send(newsockfd, response, strlen(response), 0);

        // Cleanup
        // free(response);
        close(newsockfd);
    }

    return 0;
}