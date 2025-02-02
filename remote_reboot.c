#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8765
#define BUFFER_SIZE 1024
#define FLAG_FILE "./reset_status.flag"

// 读取标志文件内容
char* read_flag_file() {
    FILE *file = fopen(FLAG_FILE, "r");
    if (file == NULL) {
        // 如果文件不存在，创建文件并写入默认值 "0"
        file = fopen(FLAG_FILE, "w");
        if (file != NULL) {
            fprintf(file, "0");
            fclose(file);
        }
        return strdup("0");
    }

    char *content = malloc(BUFFER_SIZE);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }

    fgets(content, BUFFER_SIZE, file);
    fclose(file);
    return content;
}

// 写入标志文件
void write_flag_file(const char* content) {
    FILE *file = fopen(FLAG_FILE, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    }
}

// 发送 HTTP 响应
void send_response(int client_socket, const char* status, const char* content_type, const char* body) {
    char response[BUFFER_SIZE];
    sprintf(response, 
            "HTTP/1.1 %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            status,
            content_type,
            strlen(body),
            body);
    
    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // 设置 socket 选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // 绑定 socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 监听连接
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\n", PORT);
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        // 读取请求
        read(client_socket, buffer, BUFFER_SIZE);
        printf("Received request:\n%s\n", buffer);
        
        // 解析请求路径
        char* path = strtok(buffer, " ");
        path = strtok(NULL, " "); // 获取 URL 路径
        
        if (path == NULL) {
            send_response(client_socket, "400 Bad Request", "text/plain", "Bad Request");
        }
        else if (strcmp(path, "/read") == 0) {
            char* content = read_flag_file();
            if (content != NULL) {
                send_response(client_socket, "200 OK", "text/plain", content);
                free(content);
            } else {
                send_response(client_socket, "500 Internal Server Error", "text/plain", "Error reading file");
            }
        }
        else if (strcmp(path, "/set_reset") == 0) {
            write_flag_file("reset=1");
            send_response(client_socket, "200 OK", "text/plain", "Reset status set to 1");
        }
        else if (strcmp(path, "/clear_reset") == 0) {
            write_flag_file("reset=0");
            send_response(client_socket, "200 OK", "text/plain", "Reset status set to 0");
        }
        else {
            send_response(client_socket, "404 Not Found", "text/plain", "Not Found");
        }
        
        close(client_socket);
    }
    
    return 0;
}