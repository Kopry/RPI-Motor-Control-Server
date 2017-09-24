#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <wiringPi.h> // for GPIO

/*
    Serveur
*/



int main(int argc , char *argv[])
{
    wiringPiSetup () ;
    pinMode (15, OUTPUT) ;
    pinMode (16, OUTPUT) ;
    pinMode (1, PWM_OUTPUT) ;

    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];

    // Creation d'un socket
    // DGRAM pour UDP et STREAM pour TCP
    // EN l'occurence nous utilisons une connexion TCP
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    // Preparation sock_adress comme structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8887 );

    // Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        // Affichage d'une erreur
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Ecoute sur le port definit dans la variable server.sin_port
    listen(socket_desc , 3);

    // Accepte les connexions entrante
    puts("En attente d'une connexion...");
    c = sizeof(struct sockaddr_in);

    // Accepte les connexions d'un client entrant
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    // Reçois un message d'un client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        // Renvois le message au client
        write(client_sock , client_message , strlen(client_message));
        // Affiche le message sur la console du serveur
        puts("Client message : ");
        puts(client_message);
        
        analyse_trame(client_message)


    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        digitalWrite (16,  LOW) ;
        digitalWrite (15,  LOW) ;
        pwmWrite (1, 0) ;
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

int analyse_trame()
{
    char mesg[] = "3,1,0,500";
    char* next = mesg;
    while (next)
    {
        int digit;
        if (sscanf(next, "%d", &digit) == 0)
            break;

        cout << digit << endl;

        next = strchr(next, ',');
        if (next)
            next++;
    }

    return 0;
}
