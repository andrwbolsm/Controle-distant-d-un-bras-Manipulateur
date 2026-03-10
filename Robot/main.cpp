//
//   C++ - VREP API for manipulator Robotis H
//
//
//
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

using namespace std;

extern "C" {
    #include "extApi.h"
}
#define ERROR (-1)

struct mesg 
{
  float q[6];
  float qr[6];
};

int handles[6],all_ok=1;
simxInt handle, error;

void GetHandles(int clientID)
{
	simxChar objectName[100];
	char str[10];
    for (int i=0; i < 6; i++) {
        strcpy(objectName, "joint");
        sprintf(str, "%d", i+1);
        strcat(objectName,str);
        error=simxGetObjectHandle(clientID, objectName, &handle, simx_opmode_oneshot_wait);
        if (error == simx_return_ok)
            handles[i]=handle;
        else {
            printf("Error in Object Handle - joint number %d\n", i);
            all_ok=0;
        }
    }
}
/////////////////////////////////////////////////////////
// Set the join position
//
// Inputs:
//  clientID
//  q : array of the joint values
// Return: 0 if an error occurs in object handling, 1 otherwise
/////////////////////////////////////////////////////////
int SetJointPos(int clientID,  float *q)
{
    //simxChar objectName[100];
    //char str[10];
    //simxInt handle, error;
    //int all_ok=1;

    // Get the table of handles
    /*
    for (int i=0; i < 6; i++) {
        strcpy(objectName, "joint");
        sprintf(str, "%d", i+1);
        strcat(objectName,str);
        error=simxGetObjectHandle(clientID, objectName, &handle, simx_opmode_oneshot_wait);
        if (error == simx_return_ok)
            handles[i]=handle;
        else {
            printf("Error in Object Handle - joint number %d\n", i);
            all_ok=0;
        }
    }
    */
    if (all_ok) {
        //Pause the communication thread
        //simxPauseCommunication(clientID, 1);
        // Send the joint target positions
        for (int i=0; i < 6; i++)
            simxSetJointTargetPosition(clientID, handles[i], q[i], simx_opmode_oneshot);
        // Resume the communication thread to update all values at the same time
        //simxPauseCommunication(clientID, 0);
        return 1;
    }
    else
        return 0;
}

void GetJointPos(int clientID,  float *q){
    for (int i=0; i < 6; i++){
        simxGetJointPosition(clientID, handles[i], &q[i], simx_opmode_oneshot);
    }
}


int main(int argc,char* argv[])
{
    struct mesg message;
    struct mesg message_temp;
    struct sockaddr_in sock;
    socklen_t longaddr;

    long int Te = 10 * 1000;

    int portNb=5555;            // the port number where to connect
    int timeOutInMs=5000;       // connection time-out in milliseconds (for the first connection)
    int commThreadCycleInMs=5;  // indicate how often data packets are sent back and forth - a default value of 5 is recommended
    int command, err;
    int results, resultr;
    int new_data = -1;

    command=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
        sock.sin_family=PF_INET;
        sock.sin_port=htons(2001); 
        sock.sin_addr.s_addr=0;
    longaddr=sizeof(sock);

    err=bind(command,(struct sockaddr*)&sock,longaddr);
    if(err==ERROR) {
        printf("\n erreur de bind du retard UDP!! \n");
    }

    results=ERROR;
    resultr=ERROR;

    fcntl(command,F_SETFL,fcntl(command,F_GETFL) | O_NONBLOCK); 

    // Connection to the server
    int clientID=simxStart((simxChar*)"172.23.96.1",portNb,true,true,timeOutInMs,commThreadCycleInMs);

    GetHandles(clientID);

    for (int i=0; i < 6;i++)message.q[i] = 0.0;

    if (clientID != -1)
    {
       simxSynchronous(clientID,true); // Enable the synchronous mode (Blocking function call)
       simxStartSimulation(clientID, simx_opmode_oneshot);
       
       // int offsetTime=simxGetLastCmdTime(clientID)/1000;

       while (1) 
       {
            while (recvfrom(command, &message_temp, sizeof(message_temp), 0, (struct sockaddr*)&sock, &longaddr) > 0) 
            {
                message = message_temp;
                new_data = 1;
            }

            if(new_data == 1)
            {
                SetJointPos(clientID, message.q);

                // Faire avancer la simulation d'un pas
                simxSynchronousTrigger(clientID);

                // Attendre que le pas soit calculé par Coppelia pour avoir des données fraîches
                simxGetPingTime(clientID, &error);

                GetJointPos(clientID, message.qr);

                results=sendto(command,&message,sizeof(message),0,(struct sockaddr*)&sock,sizeof(sock));

                printf("q : %f\nqr : %f\n", message.q[0], message.qr[0]);
            }
        }

        //simxStopSimulation(clientID, simx_opmode_oneshot);

        // Close the connection to the server
        //simxFinish(clientID);
    }
    else
    {
        printf("Connection to the server not possible\n");
    }

    close(command);

    return(0);
}
