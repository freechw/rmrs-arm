#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[])
{
    struct sockaddr_in pin;
    struct hostent *nlp_host;
    int sd;
    char message[] = "hello\n";
    int port;

    if(3 != argc)
    {
        printf("Too many/less paramters\n");
        return 0;
    }

    //init host and port.the hostname can be ip address.
    port = atoi(argv[2]);

    while((nlp_host = gethostbyname(argv[1])) == 0)
    {
        printf("Resolve Error!\n");
    }

    bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = htonl(INADDR_ANY);
    pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
    pin.sin_port = htons(port);

    sd = socket(AF_INET, SOCK_STREAM, 0);

    while(connect(sd, (struct sockaddr *)&pin, sizeof(pin)) == -1)
    {
        printf("Connect Error!\n");
    }

    while(sizeof(message) != send(sd, message, sizeof(message), 0))
    {
        printf("Send Error!\n");
    }

    while(1)
    {
        int recvNum;
        char recvBuf[32];

        if((recvNum = recv(sd, recvBuf, 32, 0)) < 0)
        {
            printf("Recv Error!\n");
        }
        for(int i = 0; i < recvNum; i++)
        {
            putchar(recvBuf[i]);
            //printf("  %d  ", i);
        }
        printf("\n");
    }

    close(sd);

    return 0;
}
