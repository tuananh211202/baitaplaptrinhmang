#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

void send_response(int client_socket, char *content_type, char *content, int content_length) {
    char response[1024];

    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", content_type, content_length);
    write(client_socket, response, strlen(response));
    write(client_socket, content, content_length);
}

void send_error(int client_socket, int status_code, char *status_message) {
    char response[1024];

    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: 0\r\n\r\n", status_code, status_message);
    write(client_socket, response, strlen(response));
}

void send_directory_listing(int client_socket, char *directory_path) {
    DIR *directory;
    struct dirent *entry;
    struct stat file_stat;
    char response[1024];
    char content[4096] = "";
    int content_length = 0;

    directory = opendir(directory_path);
    if (directory == NULL) {
        send_error(client_socket, 404, "Not Found");
        return;
    }

    while ((entry = readdir(directory)) != NULL) {
        char *name = entry->d_name;
        char path[1024];
        sprintf(path, "%s/%s", directory_path, name);
        stat(path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)) {
            strcat(content, "<p><b><a href=\"");
            strcat(content, name);
            strcat(content, "/\">");
            strcat(content, name);
            strcat(content, "/</a></b></p>\n");
        } else if (S_ISREG(file_stat.st_mode)) {
            char *extension = strrchr(name, '.');
            if (extension == NULL) {
                continue;
            }
            extension++;

            if (strcmp(extension, "txt") == 0 || strcmp(extension, "c") == 0 || strcmp(extension, "cpp") == 0) {
                strcat(content, "<p><i><a href=\"");
                strcat(content, name);
                strcat(content, "\">");
                strcat(content, name);
                strcat(content, "</a></i></p>\n");
            } else if (strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0) {
                int fd = open(path, O_RDONLY);
                if (fd == -1) {
                    continue;
                }
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                while ((bytes_read = read(fd, buffer, MAX_BUFFER_SIZE)) > 0) {
                    write(client_socket, buffer, bytes_read);
                }
                close(fd);
                return;
            } else if (strcmp(extension, "mp3") == 0) {
                sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\n\r\n");
                write(client_socket, response, strlen(response));
                int fd = open(path, O_RDONLY);
                if (fd == -1) {
                    continue;
                }
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                while ((bytes_read = read(fd, buffer, MAX_BUFFER_SIZE)) > 0) {
                    write(client_socket, buffer, bytes_read);
                }
                close(fd);
                return;
            }
        }
    }

    sprintf(response, "<html><head><title>Directory Listing</title></head><body><h1>Directory Listing</h1>%s</body></html>", content);
    content_length = strlen(response);

    send_response(client_socket, "text/html", response, content_length);

    closedir(directory);
}

void handle_request(int client_socket, char *request) {
    char method[16];
    char path[1024];
    char protocol[16];
    char *token;

    token = strtok(request, " ");
    if (token != NULL) {
        strcpy(method, token);
    } else {
        send_error(client_socket, 400, "Bad Request");
        return;
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(path, token);
    } else {
        send_error(client_socket, 400, "Bad Request");
        return;
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(protocol, token);
    } else {
        send_error(client_socket, 400, "Bad Request");
        return;
    }

    if (strcmp(method, "GET") != 0) {
        send_error(client_socket, 405, "Method Not Allowed");
        return;
    }

    if (strcmp(path, "/") == 0) {
        send_directory_listing(client_socket, ".");
    } else if (access(path + 1, F_OK) != -1) {
        struct stat file_stat;
        stat(path + 1, &file_stat);

        if (S_ISDIR(file_stat.st_mode)) {
            send_directory_listing(client_socket, path + 1);
        } else if (S_ISREG(file_stat.st_mode)) {
            char *extension = strrchr(path, '.');
            if (extension == NULL) {
                send_error(client_socket, 404, "Not Found");
                return;
            }
            extension++;

            if (strcmp(extension, "txt") == 0 || strcmp(extension, "c") == 0 || strcmp(extension, "cpp") == 0) {
                int fd = open(path + 1, O_RDONLY);
                if (fd == -1) {
                    send_error(client_socket, 404, "Not Found");
                    return;
                }
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                char content[MAX_BUFFER_SIZE] = "";
                int content_length = 0;
                while ((bytes_read = read(fd, buffer, MAX_BUFFER_SIZE)) > 0) {
                    strncat(content, buffer, bytes_read);
                    content_length += bytes_read;
                }
                close(fd);
                send_response(client_socket, "text/plain", content, content_length);
            } else if (strcmp(extension, "jpg") == 0 || strcmp(extension, "png") == 0) {
                int fd = open(path + 1, O_RDONLY);
                if (fd == -1) {
                    send_error(client_socket, 404, "Not Found");
                    return;
                }
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                while ((bytes_read = read(fd, buffer, MAX_BUFFER_SIZE)) > 0) {
                    write(client_socket, buffer, bytes_read);
                }
                close(fd);
            } else if (strcmp(extension, "mp3") == 0) {
                sprintf(extension, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\n\r\n");
                write(client_socket, extension, strlen(extension));
                int fd = open(path + 1, O_RDONLY);
                if (fd == -1) {
                    send_error(client_socket, 404, "Not Found");
                    return;
                }
                char buffer[MAX_BUFFER_SIZE];
                int bytes_read;
                while ((bytes_read = read(fd, buffer, MAX_BUFFER_SIZE)) > 0) {
                    write(client_socket, buffer, bytes_read);
                }
                close(fd);
            } else {
                send_error(client_socket, 404, "Not Found");
            }
        } else {
            send_error(client_socket, 404, "Not Found");
        }
    } else {
        send_error(client_socket, 404, "Not Found");
    }
}

int main() {
    int server_socket, client_socket, address_length;
    struct sockaddr_in server_address, client_address;
    char request_buffer[MAX_BUFFER_SIZE];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("HTTP server is running on port %d\n", PORT);

    while (1) {
        address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&address_length);

        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }

        int bytes_read = read(client_socket, request_buffer, MAX_BUFFER_SIZE);
        if (bytes_read <= 0) {
            close(client_socket);
            continue;
        }

        request_buffer[bytes_read] = '\0';

        handle_request(client_socket, request_buffer);

        close(client_socket);
    }

    close(server_socket);

    return 0;
}