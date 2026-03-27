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

// ========================================================
// STRUCTURES
// ========================================================

// Message UDP contenant 6 valeurs + timestamp
struct mesg 
{
    float q[6];
    long long time;
};

// Entrée de la table de lookup
typedef struct {
    double Trc;
    double Kc[6];
} LUT_entry;

// ========================================================
// VARIABLES GLOBALES LUT
// ========================================================

LUT_entry LUT[200];
int LUT_size = 0;

// ========================================================
// OUTILS GENERAUX
// ========================================================

// Convertit timespec → microsecondes
long long time_in_us(struct timespec *ts) 
{
    return (long long)ts->tv_sec * 1000000LL + ts->tv_nsec / 1000;
}

// Détection de touche dans le terminal (non bloquant)
int kbhit(void) 
{
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

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// ========================================================
// LUT : CHARGEMENT DU CSV
// ========================================================

void load_LUT(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Erreur chargement LUT"); exit(1); }

    char line[256];
    fgets(line, sizeof(line), f); // ignorer header

    while (fgets(line, sizeof(line), f))
    {
        LUT_entry e;

        int read = sscanf(line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                          &e.Trc,
                          &e.Kc[0], &e.Kc[1], &e.Kc[2],
                          &e.Kc[3], &e.Kc[4], &e.Kc[5]);

        if (read == 7)
            LUT[LUT_size++] = e;
    }

    fclose(f);
    //printf("[LUT] %d lignes chargées.\n", LUT_size);
}

// ========================================================
// SELECTION DU GAIN Kc(selon latence) – Nearest Neighbor
// ========================================================

void get_Kc_vector(double latence_s, double Kc_dyn[6])
{
    double best_diff = 1e9;
    int best_idx = 0;

    // Cherche la ligne LUT la plus proche
    for (int i = 0; i < LUT_size; i++)
    {
        double diff = fabs(latence_s - LUT[i].Trc);
        if (diff < best_diff) {
            best_diff = diff;
            best_idx = i;
        }
    }

    // Copie les 6 gains
    for (int j = 0; j < 6; j++){
        Kc_dyn[j] = LUT[best_idx].Kc[j];

        if(Kc_dyn[j] > 10) Kc_dyn[j] = 10;
    }
}

// ========================================================
// PROGRAMME PRINCIPAL
// ========================================================

int main(int nba, char *arg[]) 
{
    // Messages UDP
    struct mesg message, message_feedback;

    // Socket UDP
    struct sockaddr_in sock;
    int systeme;
    int longaddr;

    // Horodatage
    struct timespec time_step, time_recv;

    // Contrôle
    double Kc[6];
    float consigne[6];
    float dt = 0.01;
    float t = 0;
    int joint = 0;

    long long latency = 0;

    // Chargement de la Lookup Table
    load_LUT("Kc_LUT.csv");

    // ============================================
    // CONFIGURATION SOCKET UDP
    // ============================================

    systeme = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sock.sin_family = PF_INET;
    sock.sin_port = htons(2000);
    sock.sin_addr.s_addr = inet_addr("127.0.0.1");
    longaddr = sizeof(sock);

    // Mode non bloquant
    fcntl(systeme, F_SETFL, fcntl(systeme, F_GETFL) | O_NONBLOCK);

    // ============================================
    // INITIALISATION DES VALEURS
    // ============================================

    for (int i = 0; i < 6; i++) 
    {
        message.q[i] = 0.0f;
        message_feedback.q[i] = 0.0f;
        consigne[i] = 0.0f;
    }

    // ============================================
    // FICHIER CSV LOGGING
    // ============================================

    FILE *logfile = fopen("latence.csv", "w");
    if (!logfile) { perror("Erreur ouverture fichier log"); exit(1); }

    fprintf(logfile,
            "t_in,t_rcv,latence_us,"
            "q0_in,q1_in,q2_in,q3_in,q4_in,q5_in,"
            "q0_rcv,q1_rcv,q2_rcv,q3_rcv,q4_rcv,q5_rcv\n");

    // ============================================
    // BOUCLE PRINCIPALE
    // ============================================

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

        // Consigne identique pour tous
        for (int i = 0; i < 6; i++) consigne[i] = 0.2;
        //consigne[joint] = 0.5f;

        // Sélection du gain en fonction de la latence
        if (message_feedback.time) 
            get_Kc_vector(latency / 1e6, Kc);
        else
            for(int i = 0; i < 6; i++) Kc[i] = 0.0;

        // Contrôleur = Kc * (ref - mesure)
        for (int i = 0; i < 6; i++) message.q[i] = (float)(Kc[i] * (consigne[i] - message_feedback.q[i]));
        //message.q[joint] = (float)(Kc[joint] * (consigne[joint] - message_feedback.q[joint]));
        //message.q[joint] = consigne[joint];

        // Envoi
        clock_gettime(CLOCK_MONOTONIC, &time_step);
        message.time = time_in_us(&time_step);

        sendto(systeme, &message, sizeof(message), 0,
               (struct sockaddr*)&sock, sizeof(sock));

        // Réception
        recvfrom(systeme, &message_feedback, sizeof(message_feedback), 0,
                 (struct sockaddr*)&sock, &longaddr);

        // Calcul latence
        clock_gettime(CLOCK_MONOTONIC, &time_recv);
        long long time_recv_us = time_in_us(&time_recv);
        latency = time_recv_us - message_feedback.time;

        // Logging
        if (message_feedback.time) 
        {
            fprintf(logfile,
                    "%lld,%lld,%lld,"
                    "%f,%f,%f,%f,%f,%f,"
                    "%f,%f,%f,%f,%f,%f\n",
                    message.time,
                    time_recv_us,
                    latency,

                    consigne[0], consigne[1], consigne[2],
                    consigne[3], consigne[4], consigne[5],

                    message_feedback.q[0], message_feedback.q[1], message_feedback.q[2],
                    message_feedback.q[3], message_feedback.q[4], message_feedback.q[5]
            );
            fflush(logfile);
        }

        printf("Latence : %.6f s\n", latency / 1e6);

        printf("Consigne  : ");
        for(int i = 0; i < 6; i++)
            printf("% .3f  ", consigne[i]);
        printf("\n");

        printf("Kc dyn    : ");
        for(int i = 0; i < 6; i++)
            printf("% .3f  ", Kc[i]);
        printf("\n");

        t += dt;
        usleep(dt * 1000 * 1000);
    }

    close(systeme);
    return 0;
}