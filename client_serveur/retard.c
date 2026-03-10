#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

struct mesg 
{
  float q[6];
  float qr[6];
};

#define ERROR (-1)

int main (int nba, char *arg[]) 
{
    struct mesg message;
    struct mesg message_temp;
    struct sockaddr_in sockAddr, sock;

    int command, systeme, err, longaddr, addr;
    int results, resultr, last_result;

    long int  Tr;  

    command=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    sock.sin_family=PF_INET;
    sock.sin_port=htons(2000); 
    sock.sin_addr.s_addr=0;
 	longaddr=sizeof(sock);

	err=bind(command,(struct sockaddr*)&sock,longaddr);
 	if(err==ERROR) 
    {
        printf("\n erreur de bind du serveur UDP!! \n");
	}

    systeme=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    sockAddr.sin_family=PF_INET;
    sockAddr.sin_port=htons(2001); 
    sockAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
 	addr=sizeof(sockAddr);

    for (int i=0; i < 6;i++)message.q[i]=0.0;

    results = ERROR;
    resultr = ERROR;

    last_result = -1;

    Tr = 100 * 1000;

    fcntl(command,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK); 
    fcntl(systeme,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK); 

    while (1) 
    {
        while ((resultr = recvfrom(command, &message_temp, sizeof(message_temp), 0, (struct sockaddr*)&sock, &longaddr)) > 0) 
        {
            memcpy(&message, &message_temp, sizeof(message)); // On ne garde que le dernier reçu
            last_result = resultr;
        }

        if (last_result > 0) 
        {
            usleep(Tr/2);

            if(rand() % 5) // Simulation de pertes
            { 
                printf("q[0] = %f\n", message.q[0]);

                sendto(systeme, &message, sizeof(message), 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
            }

            last_result = -1; // Reset pour le prochain cycle
        }

        while ((resultr = recvfrom(systeme, &message_temp, sizeof(message_temp), 0, (struct sockaddr*)&sockAddr, &addr)) > 0) 
        {
            memcpy(&message, &message_temp, sizeof(message));
            last_result = resultr;
        }

        if (last_result > 0) 
        {
            usleep(Tr/2);

            if(rand() % 5) 
            {
                printf("qr[0] = %f\n", message.qr[0]);
                sendto(command, &message, sizeof(message), 0, (struct sockaddr*)&sock, sizeof(sock));
            }

            last_result = -1;
        }
    }

    close(command);
    close(systeme);

    return 0;

}

