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
  long long time;
};

#define ERROR (-1)
#define SIZE_BUFFER 500

int main (int nba, char *arg[]) 
{
    struct mesg message, message_feedback;
    struct mesg* buffer_comm[SIZE_BUFFER];
    struct mesg* buffer_sys[SIZE_BUFFER];
    struct sockaddr_in sockAddr, sock;

    int command, systeme, err, longaddr, addr;
    int results, resultr, last_result;

    int cnt_comm_write = 0;
    int cnt_sys_write = 0;
    int cnt_comm_read = 0;
    int cnt_sys_read = 0;
    int cnt_time = 0;

    int max_msg = 25;
    int dt = 0.01;

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

    results = ERROR;
    resultr = ERROR;

    last_result = -1;

    Tr = 100 * 1000;

    for (int i=0; i < 6;i++) message.q[i] = 0.0;
    for (int i=0; i < 6;i++) message_feedback.q[i] = 0.0;
    message.time = 0;
    message_feedback.time = 0;

    fcntl(command,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK); 
    fcntl(systeme,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK);

    while (1) 
    {
        resultr = recvfrom(command, &message, sizeof(struct mesg), 0, (struct sockaddr*)&sock, &longaddr);

        if(resultr > 0)
        {
            //if(rand() % 5) {
                struct mesg* temp_msg = malloc(sizeof(struct mesg));
                *temp_msg = message;
                buffer_comm[cnt_comm_write] = temp_msg; 
            // }
            // else {
            //     buffer_comm[cnt_comm_write] = NULL;
            // }
            
            cnt_comm_write++;
        }

        int diff_comm = cnt_comm_write - cnt_comm_read;

        if(diff_comm >= max_msg || diff_comm == -1) {
            if (buffer_comm[cnt_comm_read]) {
                //printf("q[0] = %f\n", message.q[0]);

                sendto(systeme, buffer_comm[cnt_comm_read], sizeof(struct mesg), 0, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
                
                free(buffer_comm[cnt_comm_read]);

                resultr = recvfrom(systeme, &message, sizeof(message), 0, (struct sockaddr*)&sockAddr, &addr);
                if(resultr > 0)
                {
                    //if(rand() % 5) {
                        //printf("qr: %f\n", message.q[0]);
                        struct mesg* temp_msg = malloc(sizeof(struct mesg));
                        *temp_msg = message;
                        buffer_sys[cnt_sys_write] = temp_msg; 
                    //}
                    // else {
                    //     buffer_sys[cnt_sys_write] = NULL;
                    // }
                    
                    cnt_sys_write++;
                }
                cnt_time = 0;
            }

            int diff_sys = cnt_sys_write - cnt_sys_read;

            //printf("diff_sys : %d\n", diff_sys);

            if (diff_sys >= max_msg || (diff_sys <= 0 && cnt_sys_write != 0)) {
                if (buffer_sys[cnt_sys_read]) {
                    //printf("qr[0] = %f\n", message.q[0]);
                    sendto(command, buffer_sys[cnt_sys_read], sizeof(struct mesg), 0, (struct sockaddr*)&sock, sizeof(sock));
                    
                    free(buffer_sys[cnt_sys_read]);
                }

                cnt_sys_read++;
                cnt_sys_read %= max_msg;
            }

            cnt_comm_read++;
            cnt_comm_read %= max_msg;
        }

        cnt_comm_write %= max_msg;
        cnt_sys_write %= max_msg;

        cnt_time++;
        usleep(dt * 1000 * 1000);
    }

    close(command);
    close(systeme);

    return 0;

}

