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

struct mesg {
  int in;
  float fl;
};

#define ERROR (-1)

int main (int nba, char *arg[]) {

    struct mesg message;
    int result;
    struct sockaddr_in sock;
    int command, err, longaddr;
    int results, resultr;

    command=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        sock.sin_family=PF_INET;
        sock.sin_port=htons(2001); 
        sock.sin_addr.s_addr=0;
    longaddr=sizeof(sock);

    err=bind(command,(struct sockaddr*)&sock,longaddr);
    if(err==ERROR) {
        printf("\n erreur de bind du retard UDP!! \n");
    }

    message.in = 0;
    message.fl = 0.0;

    long int Te = 2000000;
    int i = 0;

    results=ERROR;
    resultr=ERROR;

    fcntl(command,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK); 

    while (1) {
    usleep(Te);

    resultr=recvfrom(command,&message,sizeof(message), 0,(struct sockaddr*)&sock,&longaddr);

    if(resultr == -1){
        i++;
        printf("%d paquets perdus\n", i);
    } else{
        printf("server : int=%d float=%.2f\n", message.in, message.fl );
        results=sendto(command,&message,sizeof(message),0,(struct sockaddr*)&sock,sizeof(sock));
    }
    };

    close(command);

    return 0;

}

