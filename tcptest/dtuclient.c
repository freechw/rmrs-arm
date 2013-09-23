#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void RecvHead(int sd, char headerBuf[]);
void RecvData(int sd, char dataBuf[], int length);
void AnalyzeData(int sd, char dataBuf[], int length);
void SendMeterData(int sd, char commandBuf[], int length);
int ConnectServer(char host[], char port[]);
void ArrayCopy(unsigned char source[], unsigned char target[], unsigned char distIdx, unsigned char num);


char RECONNECT = 0;

int histFlow = 13;
int histPower = 20;
int cutFlow = 30;
int flowTemp = 37;
int retnTemp = 30;
int errorStatus = 0;
char meterIdByte[] = {0x46, 0x88, 0x47, 0x60};
int getTime = 1234;


int main(int argc, char * argv[])
{
    int sd;

    if(3 != argc)
    {
        printf("Too many/less paramters\n");
        return 0;
    }

    sd = ConnectServer(argv[1], argv[2]);

    while(1)
    {
        char headerBuf[13];

        RecvHead(sd, headerBuf);
        if(RECONNECT == 1)
        {
            exit(0);
            close(sd);
            sd = ConnectServer(argv[1], argv[2]);
            RECONNECT = 0;
        }
        else
        {
            printf("main: recv head\n");

            if(0x00 == headerBuf[8])//data header
            {
                int length = *(int *)(&headerBuf[9]);
                char dataBuf[length];
                RecvData(sd, dataBuf, length);
                AnalyzeData(sd, dataBuf,length);
            }
            else if(0x80 == headerBuf[8])//ACK header
            {
                ;
            }
        }
    }

    close(sd);

    return 0;
}

void RecvHead(int sd, char * headerBuf)
{
    char mainLoop = 1;
    while(1 == mainLoop)
    {

        int recvNum = 0;
        recvNum = recv(sd, headerBuf, 13, 0);
        if(recvNum < 0)
        {
            printf("Recv Error\n");
            continue;
        }
        else if (recvNum < 13)
        {
            printf("recv header, but not full!\n");
            printf("recv length is %d\n", recvNum);
            int tmpint = *(int *)(&headerBuf[0]);
            printf("recv data trans to int is %d\n", tmpint);
            headerBuf[12] = '\0';
            printf("%s\n", headerBuf);
            char recvFlag = 0;
            int newRecvNum = 0;
            while(0 == recvFlag)
            {
                printf("recv until full!\n");
                newRecvNum += recv(sd, &headerBuf[recvNum + newRecvNum], (13-recvNum-newRecvNum), 0);
                if(13 == newRecvNum + recvNum)
                {
                    printf("recv full!\n");
                    recvFlag = 1;
                }
            }
        }

        printf("RecvHead: recv full 2 !\n");

        int headerId = *(int *)(&headerBuf[0]);
        printf("RecvHead: head id is %d\n", headerId);
        
        char localIdByte[] = {0x01, 0x02, 0x03, 0x04};
        int localId = *(int *)localIdByte;
        if(headerId == localId )
        {
            printf("RecvHead: recv header!!\n");
            mainLoop = 0;
        }
        else
        {
            RECONNECT = 1;
            printf("RecvHead:recv wrong dtu identifer, reconnect!");
            mainLoop = 0;
        }
    }
}

void RecvData(int sd, char dataBuf[], int length)
{
    printf("RecvData: wait for data!\n");
    while(1)
    {
        int recvNum = 0;
        if((recvNum = recv(sd, dataBuf, length, 0)) < 0)
        {
            printf("Data Recv Error\n");
            continue;
        }
        else if (recvNum < length)
        {
            char recvFlag = 0;
            int newRecvNum = 0;
            while(!recvFlag)
            {
                newRecvNum += recv(sd, &dataBuf[recvNum + newRecvNum], (length-recvNum-newRecvNum), 0);
                if(length == newRecvNum + recvNum)
                {
                    recvFlag = 1;
                }
            }
        }

        break;
    }
}

void AnalyzeData(int sd, char dataBuf[], int length)
{
    char dataint = *(char *)(&dataBuf[1]);
    if(257 == dataint)
    {
        ;//it is a hello data
    }
    else if (1 == dataint)
    {
        //it is a read command
        printf("receive command\n");
        SendMeterData(sd, dataBuf, length);
    }
}

void SendMeterData(int sd, char commandBuf[], int length)
{


    printf("SendMeterData: command length is %d\n", length);

    if(8 == length)
    {
        unsigned char command = commandBuf[7];
        unsigned char data[16];
        int dataLength;
        int meterId = *(int *)(meterIdByte);

        printf("SendMeterData: command is %d\n", command);

        switch (command)
        {
            case 0x01:
                dataLength = 4;
                ArrayCopy((char *)(&histFlow), data, 0, 4);
                histFlow++;
                break;
            case 0x02:
                dataLength = 4;
                ArrayCopy((char *)(&histPower), data, 0, 4);
                histPower++;
                break;
            case 0x04:
                dataLength = 4;
                ArrayCopy((char *)(&cutFlow), data, 0, 4);
                cutFlow++;
                break;
            case 0x05:
                dataLength = 4;
                ArrayCopy((char *)(&flowTemp), data, 0, 4);
                flowTemp++;
                break;
            case 0x06:
                dataLength = 4;
                ArrayCopy((char *)(&retnTemp), data, 0, 4);
                retnTemp++;
                break;
            case 0x07:
                dataLength = 4;
                ArrayCopy((char *)(&errorStatus), data, 0, 4);
                break;
            case 0x08:
                dataLength = 4;
                ArrayCopy((char *)(&meterId), data, 0, 4);
                break;
            case 0x09:
                dataLength = 4;
                ArrayCopy((char *)(&getTime), data, 0, 4);
                getTime++;
                break;
            case 0x0d:
                dataLength = 16;
                ArrayCopy((char *)(&histFlow), data, 0, 4);
                ArrayCopy((char *)(&histPower), &data[4], 0, 4);
                ArrayCopy((char *)(&meterId), &data[8], 0, 4);
                ArrayCopy((char *)(&getTime), &data[12], 0, 4);
                histFlow++;
                histPower++;
                getTime++;
                break;
            default:
                break;
        }

        char retnBuf[10+dataLength];
        commandBuf[1] = 0x04;
        ArrayCopy(commandBuf, retnBuf, 0, 8);
        ArrayCopy(data, &retnBuf[10], 0, dataLength);
        retnBuf[7] = 0x01;
        retnBuf[8] = 0x00;
        retnBuf[9] = command;

        char headBuf[13] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
        unsigned int retnBufLength = sizeof(retnBuf);
        ArrayCopy((unsigned char *)(&retnBufLength), headBuf, 9, 4);

        unsigned char wholeBuf[sizeof(headBuf) + sizeof(retnBuf)];
        ArrayCopy(headBuf, wholeBuf, 0, sizeof(headBuf));
        ArrayCopy(retnBuf, wholeBuf, sizeof(headBuf), sizeof(retnBuf));

        printf("data is \n");
        for(int i = 0; i < dataLength; i++)
        {
            printf("%d ", data[i]);
        }
        printf("\n");

        printf("SendMeterData: retnLength is %d", sizeof(retnBuf));

        if(sizeof(wholeBuf) != send(sd, wholeBuf, sizeof(wholeBuf), 0))
        {
            printf("SendMeterData: Send Error!\n");
        }

    }
}

int ConnectServer(char host[], char Port[])
{
    struct sockaddr_in pin;
    struct hostent *nlp_host;
    int sd;
    char message[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x00, 0x00};
    int port;

    //init host and port.the hostname can be ip address.
    port = atoi(Port);

    while((nlp_host = gethostbyname(host)) == 0)
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

    return sd;
}

void ArrayCopy(unsigned char source[], unsigned char target[], unsigned char distIdx, unsigned char num)
{
    unsigned char i;
    for (i = 0; i < num; i++)
    {
        target[distIdx+i] = source[i];
    }
}

