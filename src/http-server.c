#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../lib/strlib.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define WWW_DIR "./www/"
int main() {
int server_fd, client_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);
char buffer[BUFFER_SIZE] = {0};
// Create socket
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
perror("socket failed");
exit(EXIT_FAILURE);
}
// Enable address reuse
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
// Bind and listen
address.sin_family = AF_INET;

address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);
bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 3);
printf("Server listening on http://localhost:%d\n", PORT);
while (1) 
{
    client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client_fd < 0) 
    {
        perror("accept");
        continue;
    }
    // Read request
    int valread = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (valread <= 0) 
    {
        close(client_fd);
        continue;
    }
    buffer[valread] = '\0';
    printf("REQUEST:\n%s\n", buffer);
    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";

    ssize_t sent = send(client_fd, response, strlen(response), 0);

    // Parse and respond (Steps 2-4)
    // ...
    close(client_fd);
}
close(server_fd);
return 0;
}