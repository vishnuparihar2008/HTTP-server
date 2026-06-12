#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 2048

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Hardcoded simple HTTP response (Status line + Headers + Body)
    const char *http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello from C Server!</h1></body></html>";

    // 1. Create socket file descriptor (IPv4, TCP, Protocol 0)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow immediate reuse of port to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Define the server network configuration
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    address.sin_port = htons(PORT);       // Convert port to network byte order

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen for connections (backlog queue set to 10)
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on http://localhost:%d\n", PORT);

    // 4. The Infinite Accept Loop
    while (1) {
        printf("\nWaiting for a new connection...\n");
        
        // Block until a client attempts to connect
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue; 
        }

        // Read the incoming HTTP request raw text
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate string
            printf("--- Received Request ---\n%s\n------------------------\n", buffer);
        }

        // Send the hardcoded HTTP protocol response back to the client
        write(client_fd, http_response, strlen(http_response));
        printf("Response sent to client.\n");

        // Close the client connection (HTTP/1.1 close model)
        close(client_fd);
    }

    // Unreachable code in this loop, but good practice
    close(server_fd);
    return 0;
}

