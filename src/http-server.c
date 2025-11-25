#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../lib/strlib.h"
#include "../src/config.h"
#include <errno.h>




int create_server_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on http://localhost:%d\n", PORT);
    return server_fd;
}

size_t send_http_header( int client_fd , const char* status , const char* content_type , size_t body_size )
{
    char header[BUFFER_SIZE];
    if ( strlib_strcpy(header , BUFFER_SIZE , "HTTP/1.1 ") == SIZE_MAX ) return SIZE_MAX ;
    if ( strlib_strncat(header , BUFFER_SIZE , status , strlib_strlen(status) ) == SIZE_MAX ) return SIZE_MAX ;
    if ( strlib_strncat(header , BUFFER_SIZE , "\r\nContent-Type: " , 16 ) == SIZE_MAX ) return SIZE_MAX ;
    if ( strlib_strncat(header , BUFFER_SIZE , content_type , strlib_strlen(content_type) ) == SIZE_MAX ) return SIZE_MAX ;
    if ( strlib_strncat(header , BUFFER_SIZE , "\r\nContent-Length: " , 18 ) == SIZE_MAX ) return SIZE_MAX ;
    char* bodySize = strlib_NumToStr(body_size) ;
    if ( strlib_strncat(header , BUFFER_SIZE , bodySize , strlib_strlen(bodySize)) == SIZE_MAX ) {free(bodySize); return SIZE_MAX ;} 
    free(bodySize) ;
    if ( strlib_strncat(header , BUFFER_SIZE , "\r\n\r\n" , 4 ) == SIZE_MAX ) return SIZE_MAX ;
    size_t header_len = strlib_strlen(header) ;
    size_t sent = 0 ;
    while (sent < header_len)   
    {
        ssize_t s = send(client_fd, &header[sent], header_len - sent, 0);
        if ( s < 1 ) return SIZE_MAX ;
        sent += s ;
    }
    return 0 ;
}

size_t send_http_body(int client_fd, const char* body , size_t size )
{
    size_t sent = 0 ;
    while (sent < size)
    {
        ssize_t s = send(client_fd, &body[sent], size- sent , 0);
        if ( s < 1 ) return SIZE_MAX ;
        sent += s ;
    }
    return 0 ;
}

FILE* http_open_requested_file( int client_fd , const char* request_path)
 {
        size_t request_pathLen = strlib_strlen(request_path) ;
        size_t path_size = request_pathLen + strlib_strlen(PUBLIC_FILES_PATH ) + 1 ;
        char* path = malloc(( path_size ) *sizeof(char)) ;
        if (path == NULL ) goto Internal_Server_Error ;
        if (strlib_strcpy(path , path_size, PUBLIC_FILES_PATH ) == SIZE_MAX ) { free(path) ;  goto Internal_Server_Error ; }
        if (strlib_strncat(path , path_size , request_path , strlib_strlen(request_path) ) == SIZE_MAX ) { free(path) ; goto Internal_Server_Error ; }
        if (path[strlib_strlen(path)-1 ] == '/')
        {
            free(path) ;
            path_size =  request_pathLen + strlib_strlen(INDEX_FILE) + 1 ;
            path = malloc((path_size) *sizeof(char)) ;
            if (path == NULL ) goto Internal_Server_Error ;
            if (strlib_strcpy(path , path_size  , request_path) == SIZE_MAX ) {free (path) ; goto Internal_Server_Error ;} 
            if (strlib_strncat(path , path_size , INDEX_FILE , 10 ) == SIZE_MAX ) {free (path) ; goto Internal_Server_Error ;}
            FILE * fp =  http_open_requested_file(client_fd , path) ;
            free (path) ;
            return fp ;
        }
        FILE *fp = fopen(path, "r");
        free(path) ;
        if (fp == NULL) 
        {
            if (errno == ENOENT) 
            {
                send_http_header(client_fd , "404 Not Found" , "text/plain" , strlib_strlen(ERROR_404_MESSAGE) );
                send_http_body(client_fd , ERROR_404_MESSAGE , strlib_strlen(ERROR_404_MESSAGE)) ;
                return NULL ;
            } 
            else if (errno == EACCES) 
            {
                send_http_header(client_fd , "403 Forbidden" , "text/plain" , strlib_strlen(ERROR_403_MESSAGE) );
                send_http_body(client_fd , ERROR_403_MESSAGE , strlib_strlen(ERROR_403_MESSAGE)) ;
                return NULL ;
            } 
            else 
            {
                Internal_Server_Error :
                send_http_header(client_fd , "500 Internal Server Error" , "text/plain" , strlib_strlen(ERROR_500_MESSAGE) );
                send_http_body(client_fd , ERROR_500_MESSAGE , strlib_strlen(ERROR_500_MESSAGE)) ;
                return NULL ;
            }
        }
        return fp ;
 }

size_t send_http_file (int client_fd , FILE* fp , const char * fileType )
{
    if (fp == NULL ) return SIZE_MAX ;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char data[CHUNK_SIZE] ;
    send_http_header(client_fd, "200 OK" , fileType , size ) ;
    while (1) 
    {
        size_t read_bytes = fread(data, 1, CHUNK_SIZE, fp);
        if (read_bytes > 0) 
        {
            send_http_body(client_fd , data , read_bytes) ;
        }
        if (read_bytes < CHUNK_SIZE) break;
    }
    fclose(fp);
    return 0 ;
}


int main() 
{
    int server_fd = create_server_socket() ;
    char buffer[BUFFER_SIZE] = {0};
    while (1) 
    {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
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
        
        //__________________________________________________

        char* method = strlib_strtok(buffer , " ") ;
        if (strlib_strcmp(method , "GET") != STRCMP_EQUAL) 
        {
            send_http_header(client_fd , "501 Not Implemented" , "text/plain" , strlib_strlen(ERROR_501_MESSAGE)) ;
            send_http_body(client_fd , ERROR_501_MESSAGE , strlib_strlen(ERROR_501_MESSAGE)) ;
            close(client_fd);
            continue;
        }

        char* request_path = strlib_strtok(NULL , " ") ;
        FILE *fp = http_open_requested_file(client_fd , request_path) ;
        if (fp == NULL) { close(client_fd);  continue; }
        send_http_file(client_fd , fp , "text/html") ;
        
        close(client_fd); 
    }
    close(server_fd);
    return 0;
}


