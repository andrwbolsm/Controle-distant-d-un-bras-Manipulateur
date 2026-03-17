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
  long long time;
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
    struct mesg message, message_feedback;
    struct sockaddr_in sockAddr, sock;
    struct timespec time_step, time_recv;

    float w= 2*M_PI / 2.5;
    float dt = 0.01;
    float t = 0;

    int systeme, longaddr , results, resultr;

    long long latency = 0;

    systeme=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    sock.sin_family = PF_INET;
    sock.sin_port = htons(2000); 
    sock.sin_addr.s_addr = inet_addr("127.0.0.1");
    longaddr=sizeof(sock);

    for (int i=0; i < 6;i++) message.q[i] = 0.0;
    for (int i=0; i < 6;i++) message_feedback.q[i] = 0.0;

    message.time = 0;
    message_feedback.time = 0;

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
        //--- Keyboard Logic ---
        // if (kbhit()) {
        //     char c = getchar();
        //     if (c == 'q') message.q[0] += 0.1; // Increase
        //     if (c == 'a') message.q[0] -= 0.1; // Decrease
        //     if (c == 'w') message.q[1] += 0.1; // Increase
        //     if (c == 's') message.q[1] -= 0.1; // Decrease
        //     if (c == 'e') message.q[2] += 0.1; // Increase
        //     if (c == 'd') message.q[2] -= 0.1; // Decrease
        //     if (c == 'r') message.q[3] += 0.1; // Increase
        //     if (c == 'f') message.q[3] -= 0.1; // Decrease
        //     if (c == 't') message.q[4] += 0.1; // Increase
        //     if (c == 'g') message.q[4] -= 0.1; // Decrease
        //     if (c == 'y') message.q[5] += 0.1; // Increase
        //     if (c == 'h') message.q[5] -= 0.1; // Decrease
        //     if (c == 'z') break;               // Quit
        // }

        //message.q[0] = 0.5*sin(t * w);

        message.q[0] = -4;
        
        //for (int i=0; i < 6;i++) message.q[i] = -1.0;

        for (int i=0; i < 6;i++) printf("q[%d] = %f; ", i, message.q[i]);
        printf("\n");
        for (int i=0; i < 6;i++) printf("q[%d] = %f; ", i, message_feedback.q[i]);
        printf("\n");

        clock_gettime(CLOCK_MONOTONIC, &time_step);
        message.time = time_in_us(&time_step);

        results = sendto(systeme, &message, sizeof(message), 0,
                        (struct sockaddr*)&sock, sizeof(sock));

        resultr = recvfrom(systeme, &message_feedback, sizeof(message_feedback), 0,
                        (struct sockaddr*)&sock, &longaddr);

        clock_gettime(CLOCK_MONOTONIC, &time_recv);
        long long time_recv_us = time_in_us(&time_recv);

        latency = time_recv_us - message_feedback.time;

        if (message_feedback.time) {
            fprintf(logfile, "%lld,%lld,%lld,%f,%f\n",
                    message.time,
                    time_recv_us,
                    latency,
                    message.q[0],     // valor enviado
                    message_feedback.q[0]);   // valor recebido
            
            fflush(logfile);
        }

        t += dt;
        usleep(dt*1000*1000);
    }

    close(systeme);

    return 0;

}