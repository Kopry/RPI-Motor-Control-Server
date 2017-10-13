
/*
    Serveur
*/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <cstdlib> // pour cls
#define maxmess 192

using namespace std;

/* -- Prototype declaration

*/

void analyse_trame(char *);
void controle_moteur(int *);

/* -- main

corp du programme, il s'agit d'un serveur recevant une trame depuis un client distant pour le controle des moteurs du robot via
une connexion TCP

Voir la fonction analyse_trame pour plus d'information au sujet de la trame.

*/

int main(int argc , char *argv[])
{

    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[maxmess];

    /* verifie si wiringpi est initialisé

    if (wiringPiSetup () == -1)
    exit (1);

    */

    // Creation d'un socket
    // DGRAM pour UDP et STREAM pour TCP
    // EN l'occurence nous utilisons une connexion TCP
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Impossible de creer le socket");
    }
    // affichage statut de connexion du serveur
    cout << " ________________________\033[36mStatut serveur\033[0m___ " << endl;
    cout << "|                                         |" << endl;
    cout << "| Socket :                     \033[32mOK\033[0m         |" << endl;

    // Preparation sock_adress comme structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    // Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        // Affichage d'une erreur
        cout << "| Connexion :                \033[31mERORR\033[0m        |" << endl;
        cout << "| Erreur de liaison (bind error)          |" << endl;
        cout << "|_________________________________________|" << endl;
        return 1;
    }
    // Boucle infinie, le serveur ne s'arette jamais
    while(1)
    {
        // affichage statut liaison
        cout << "| Liaison                      \033[32mOK\033[0m         |" << endl;
        // Ecoute sur le port definit dans la variable server.sin_port
        listen(socket_desc , 3);

        // Accepte les connexions entrante et met a jour le statut
        cout << "| En attente d'une connexion...           |" << endl;
        c = sizeof(struct sockaddr_in);

        // Accepte les connexions d'un client entrant
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0)
        {
            perror("Impossible d'accepter");
            return 1;
        }
        // affiche statut de la connexion
        cout << "| Connexion :                  \033[32mOK\033[0m         |" << endl;
        cout << "| Attente d'un message du client          |" << endl;
        cout << "|_________________________________________|" << endl;

        // Reçois un message d'un client

        while( (read_size = recv(client_sock , client_message , maxmess , 0)) > 0 )
        {

            system("clear");
            // Affiche le message sur la console du serveur
            cout << " ________________________\033[36mStatut analyse\033[0m___ " << endl;
            cout << "|                                         |" << endl;
            //cout << "| Trame reçue du client taille            |" << endl;
            //cout << "| " << sizeof(client_message) << endl;
            //cout << "| " << client_message << endl;
            // Renvois le message au client
            // write(client_sock , client_message , strlen(client_message));

            // Appel de la fonction d'analyse
            cout << "| Analyse des informations reçue          |" << endl;
            cout << "|                                         |" << endl;
            analyse_trame(client_message);

        }

        if(read_size == 0)
        {
            cout << "| Deconnexion :                \033[32mOK\033[0m         |" << endl;
            cout << "|_________________________________________|" << endl;
            //system("clear");
            fflush(stdout);
        }
        else if(read_size == -1)
        {
            perror("recv failed");
        }
    }
    return 0;
}

/* -- analyse_trame(char *mesg)

Variable reçue : la chaine de caractere mesg envoyé par le client distant.

Fonction de découpage de trame, cette fonction recupere le message envoyé par le client pour ensuite isoler chaques entiers.
celle-ci se refere aux virgules qui servent de caractere de separation entre chaque bloc de donnée pour savoir lorsqu'une
nouvelle variable démarre.

Il y'a en tout 4 moteurs controlés par chaques lot de variable
L'exemple qui suit represente la chaine de caractere stocké dans la trame :

* IDM  : Identifiant moteur
* SENS : Sens de rotation du moteur
* BRAKE: Moteur en activité ou non
* PWM  : MLI - modulation de largeur d'impulsion (permet de faire varier la vitesse des moteurs a courant continu

[ Indice   ][  0  |   1  |   2   |  3  ][  4  |   5  |   6   |  7  ][  8  |  9   |   10  |  11 ][  12 |   13 |  14   |  15 ][ 16 ]
[ Nom      ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ][Etat]
[ Valeur   ][  1  |   0  |   0   | 500 ][  2  |   1  |   0   | 500 ][  3  |   0  |   0   |  0  ][  4  |   0  |   0   |  0  ][  1 ]

*/

void analyse_trame(char *mesg)
{
    int trame[maxmess],n = 0; // Tableau servant de recepteur a la trame convertie en entier, n est l'index de ce tableau
    // pointeur vers le tableau trame
    char* next = mesg;
    while (next)
    {
        int digit;
        if (sscanf(next, "%d", &digit) == 0)
            break;

        // Stockage de chaque entier de la trame dans un tableau d'entier
        trame[n]=digit;
        // incrementation de l'index du tableau
        n++;

        next = strchr(next, ',');
        if (next)
            next++;
    }

    /* lecture du tableau d'entier pour tester son fonctionnement
    n = 0;
    while (n<4)
    {
    cout << "Valeur a l'indice " << n << " : " << trame[n] << endl;
    n++;
    }
    */

    // appel de la fonction pour mettre a jour les valeurs du GPIO
    controle_moteur(trame);

}

/* -- controle_moteur(int *trame)

[ Indice   ][  0  |   1  |   2   |  3  ][  4  |   5  |   6   |  7  ][  8  |  9   |   10  |  11 ][  12 |   13 |  14   |  15 ]
[ Nom      ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ][ IDM | SENS | BRAKE | PWM ]
[ Valeur   ][  1  |   0  |   0   | 500 ][  2  |   1  |   0   | 500 ][  3  |   0  |   0   |  0  ][  4  |   0  |   0   |  0  ]
[ Pin      ][  ?  |   ?  |   ?   |  ?  ]

*/

void controle_moteur(int *trame)
{
    cout << " ________________________\033[36mStatut controle\033[0m__ " << endl;
    cout << "|                                         |" << endl;
    cout << "| controle des moteur actif               |" << endl;
    cout << "| mise à jour des informations GPIO       |" << endl;
    cout << "|_________________________________________|" << endl;
    cout << "|                                         |" << endl;
    // Status des moteur 1 & 2
    cout << "| Moteur ID     " << trame[0] << "    | Moteur ID     " << trame[4] << "    |" << endl;
    cout << "| Sens moteur   " << trame[1] << "    | Sens moteur   " << trame[5] << "    |" << endl;
    // Mise à jour des variable concernant le sens de rotation des moteurs
    // digitalWrite (7, trame[1]);
    cout << "| Actif O/N     " << trame[2] << "    | Actif O/N     " << trame[6] << "    |" << endl;
    // mise à jour des variables concernant l'état des moteurs
    // digitalWrite (8, trame[2]):
        // Status des moteur 3 & 4
    cout << "| Moteur ID     " << trame[8] << "    | Moteur ID     " << trame[12] << "    |" << endl;
    cout << "| Sens moteur   " << trame[9] << "    | Sens moteur   " << trame[13] << "    |" << endl;
    // Mise à jour des variable concernant le sens de rotation des moteurs
    // digitalWrite (7, trame[1]);
    cout << "| Actif O/N     " << trame[10] << "    | Actif O/N     " << trame[14] << "    |" << endl;
    // mise à jour des variables concernant l'état des moteurs
    // digitalWrite (8, trame[2]):
    cout << " ________________________\033[36mStatut PWM\033[0m_______ " << endl;
    cout << "|                                         |" << endl;
    cout << "| PWM moteur "<< trame [0] << " : " << trame[3] << endl;
    cout << "| PWM moteur "<< trame[4]  << " : " << trame[7] << endl;
    cout << "| PWM moteur "<< trame [8] << " : " << trame[11] << endl;
    cout << "| PWM moteur "<< trame[12]  << " : " << trame[15] << endl;
    // mise à jour concernant l'état de la PWM attribué aux moteurs
    // pinMode(1, PWM_OUTPUT);
    // pwmWrite (1, trame[3];
    // pwmWrite (1, trame[7];

}
