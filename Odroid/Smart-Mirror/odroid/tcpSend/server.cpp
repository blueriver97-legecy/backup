#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSIZE 4096

#define SERVER_IP "114.70.21.95"
#define SERVER_PORT 8888

int recvn(int s, char* buf, int len, int flags);

int main(){

    int sum = 0, totalSize = 0;
    int ret = 0;
    int server_socket = 0;
    int client_socket = 0;
    char newFileName[128] = {'\0'};
    char newFileAddr[128] = {'\0'};
    char buf[BUFSIZE];
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = 0;
    FILE *fp = NULL;

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // socket bind
    ret = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0){
        printf("bind error\n");
    }
    
    // socket listen
    ret = listen(server_socket, 5);
    if(ret < 0){
        printf("listen error\n");
    }

    client_addr_size = sizeof(client_addr);

    while(true){
       totalSize = 0;
       sum = 0;
       
       memset(newFileName, 0, sizeof(newFileName));
       memset(newFileAddr, 0, sizeof(newFileAddr));

       // socket accept
       client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
       if(client_socket < 0){
          printf("accept error\n");
          continue;
       }

       ret = recvn(client_socket, newFileName, 256, 0);
       if(ret < 0){
           printf("newFileName receive error\n");      
           continue;
       }

       ret = recvn(client_socket, (char *)&totalSize, sizeof(totalSize), 0);
       if(ret < 0){
           printf("totalSize receive error\n");
           continue; 
       }

       sprintf(newFileAddr, "/opt/lampp/htdocs/temp/%s", newFileName);
       printf("File name : %s\n", newFileName);
       printf("File path : %s\n", newFileAddr);
       printf("File size : %d\n", totalSize);

       // file write
       fp = fopen(newFileAddr, "wb");
       if(fp==NULL){
           printf("file open error\n");
       }

       while(true){
           ret = recvn(client_socket, buf, BUFSIZE, 0);
           usleep(500);
           if(ret < 0){
               printf("recv error\n");
               break;           
           } else if(ret == 0){
               break; 
           } else{
               fwrite(buf, 1, ret, fp);
               sum += ret;           
           }
       }
       ret = fclose(fp);
       usleep(1000000);
       if(ret < 0){
           printf("fclose error\n");
       }

       // close client_socket
       close(client_socket);
    }
    // close server_socket
    close(server_socket);
}

int recvn(int s, char* buf, int len, int flags){
    int received;
    char* ptr = buf;
    int left = len;

    while(left > 0){
        received = recv(s, ptr, left, flags);
        if(received == -1){
            return -1; //error
        }
        else if(received == 0){
            break;
        }
        left -= received;
        ptr += received;
    }//while
    return (len - left);
}
