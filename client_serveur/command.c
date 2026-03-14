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
#include <math.h>
#include <time.h>

#include <termios.h>

struct mesg 
{
  float q[6];
  float qr[6];
};

#define ERROR (-1)

long long time_in_us(struct timespec *ts) 
{
    return (long long)ts->tv_sec * 1000000LL + ts->tv_nsec / 1000;
}

// Helper function to check for keypress without blocking
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

int main (int nba, char *arg[]) 
{
    struct mesg message;
    struct sockaddr_in sockAddr, sock;
    struct timespec t_send, t_recv;

    float w= 2*M_PI / 2.5;
    float dt = 0.01;
    float t = 0;

    int systeme, longaddr , results, resultr;

    long long send_us = 0;
    long long recv_us = 0;
    long long latency = 0;

    systeme=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    sock.sin_family = PF_INET;
    sock.sin_port = htons(2000); 
    sock.sin_addr.s_addr = inet_addr("127.0.0.1");
    longaddr=sizeof(sock);

    for (int i=0; i < 6;i++) message.q[i] = 0.0;

    fcntl(systeme,F_SETFL,fcntl(systeme,F_GETFL) | O_NONBLOCK);
    
    FILE *logfile = fopen("latence.csv", "w");
    if (!logfile) 
    {
        perror("Erreur");
        exit(EXIT_FAILURE);
    }
    fprintf(logfile, "t_in,t_rcv,latence_us,q0_in,q0_rcv\n");

    while (1) 
    {   
        // --- Keyboard Logic ---
        // if (kbhit()) {
        //     char c = getchar();
        //     if (c == 'u') message.q[0] += 0.1; // Increase
        //     if (c == 'd') message.q[0] -= 0.1; // Decrease
        //     if (c == 'q') break;               // Quit
        // }

        message.q[0] = 0.5*sin(t * w);
        printf("q[0] = %f\n", message.q[0]);

        results = sendto(systeme, &message, sizeof(message), 0,
                        (struct sockaddr*)&sock, sizeof(sock));

        clock_gettime(CLOCK_MONOTONIC, &t_send);

        resultr = recvfrom(systeme, &message, sizeof(message), 0,
                        (struct sockaddr*)&sock, &longaddr);
              
        clock_gettime(CLOCK_MONOTONIC, &t_recv);

        if(resultr > 0)
        {
            send_us = time_in_us(&t_send);
            recv_us = time_in_us(&t_recv);
            latency = recv_us - send_us;

            fprintf(logfile, "%lld,%lld,%lld,%f,%f\n",
                    send_us,
                    recv_us,
                    latency,
                    message.q[0],     // valor enviado
                    message.qr[0]);   // valor recebido
        }
        
        //fflush(logfile);

        t += dt;
        usleep(dt*1000*1000);
    }

    close(systeme);

    return 0;

}