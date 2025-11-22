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

#define SAFE_DO(cond) \
    do { \
        if (cond == SIZE_MAX ) { \
            goto CLOSE ;  \
        } \
    } while (0)




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


    printf("-------------------------------\n%s\n", buffer);
    char response [BUFFER_SIZE] ;


    char* method = strlib_strtok(buffer , " ") ;
    if (strlib_strcmp(method , "GET") != STRCMP_EQUAL) 
    {
        SAFE_DO( strlib_strcpy(response , BUFFER_SIZE , 
            "HTTP/1.1 501 Not Implemented\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 15\r\n"
            "\r\n"
            "Not Implemented" ));

        ssize_t sent = send(client_fd, response, strlen(response), 0);
        // یادم باشه برسی اینکه کامل ارسال بشه
        goto CLOSE ;
    }
    char* request_path = strlib_strtok(NULL , " ") ;
    SAFE_DO(strlib_strcpy(response , BUFFER_SIZE , 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: "));

    char* bodyLen = strlib_NumToStr(strlib_strlen(request_path)) ;
    SAFE_DO( strlib_strncat(response , BUFFER_SIZE , bodyLen , strlib_strlen(bodyLen))) ; free(bodyLen);
    SAFE_DO( strlib_strncat(response , BUFFER_SIZE , "\r\n\r\n" , 4 )) ;
    SAFE_DO( strlib_strncat(response , BUFFER_SIZE , request_path , strlib_strlen(request_path))) ;

    ssize_t sent = send(client_fd, response, strlen(response), 0);
    // یادم باشه برسی اینکه کامل ارسال بشه
    
    CLOSE:
        close(client_fd); 
}
close(server_fd);
return 0;
}
