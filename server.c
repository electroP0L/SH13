#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>

struct _client
{
        char ipAddress[40];
        int port;
        char name[40];
} tcpClients[4];
int nbClients;
int fsmServer;
int deck[13]={0,1,2,3,4,5,6,7,8,9,10,11,12};
int tableCartes[4][8];
char *nomcartes[]=
{"Sebastian Moran", "irene Adler", "inspector Lestrade",
  "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
  "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
  "Mrs. Hudson", "Mary Morstan", "James Moriarty"};
int joueurCourant;
int joueurselimines[4] = {-1,-1,-1,-1};

int endgame(int IdJoueur){
	char buffer[256];

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 8; j++){
			sprintf(buffer, "V %d %d %d", i, j, tableCartes[i][j]);
			broadcastMessage(buffer);

			if(IdJoueur != -1){
				if (i == IdJoueur){
					sprintf(buffer, "M %d", 5);
					sendMessageToClient(tcpClients[i].ipAddress, tcpClients[i].port, buffer);
				}
				else{
					sprintf(buffer, "M %d", 4);
					sendMessageToClient(tcpClients[i].ipAddress, tcpClients[i].port, buffer);
				}
			}
		}
	}
	return -1;
}

int joueursuivant(int joueurCourant, int* joueurselimines){		//Une fonction pour passer au joueur suivant, en sautant les joueurs éliminés
	char buffer[256];
	//Si tous les joueurs ont perdu, on indique que le jeu est terminé et on dévoile le tableau tablecarte à tous les joueurs
	if(joueurselimines[0]==1 && joueurselimines[1]==1 && joueurselimines[2]==1 && joueurselimines[3]==1){
		endgame(-1);
		joueurCourant=4;
	}

	else{
		joueurCourant++;
		if(joueurCourant>3){
			joueurCourant=0;
		}
		
		while(joueurselimines[joueurCourant]==1){
			joueurCourant++;
			if(joueurCourant>3){
				joueurCourant=0;
			}
		}

		sprintf(buffer, "M %d", joueurCourant);
		broadcastMessage(buffer);
	}
	
	return joueurCourant;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void melangerDeck()
{
		srand (time(NULL));
        int i;
        int index1,index2,tmp;

        for (i=0;i<1000;i++)
        {
                index1=rand()%13;
                index2=rand()%13;

                tmp=deck[index1];
                deck[index1]=deck[index2];
                deck[index2]=tmp;
        }
}

void createTable()
{
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5 
	// Le joueur 2 possede les cartes d'indice 6,7,8 
	// Le joueur 3 possede les cartes d'indice 9,10,11
	// Le coupable est la carte d'indice 12
	int i,j,c;

	for (i=0;i<4;i++)
		for (j=0;j<8;j++)
			tableCartes[i][j]=0; //On réinitialise le tableau

	for (i=0;i<4;i++) //Pour chacune des carte, on rempli le tableau des cartes associé au joueur
	{
		for (j=0;j<3;j++)
		{
			c=deck[i*3+j];
			switch (c)
			{
				case 0: // Sebastian Moran
					tableCartes[i][7]++;
					tableCartes[i][2]++;
					break;
				case 1: // Irene Adler
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					tableCartes[i][5]++;
					break;
				case 2: // Inspector Lestrade
					tableCartes[i][3]++;
					tableCartes[i][6]++;
					tableCartes[i][4]++;
					break;
				case 3: // Inspector Gregson 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					tableCartes[i][4]++;
					break;
				case 4: // Inspector Baynes 
					tableCartes[i][3]++;
					tableCartes[i][1]++;
					break;
				case 5: // Inspector Bradstreet 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					break;
				case 6: // Inspector Hopkins 
					tableCartes[i][3]++;
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					break;
				case 7: // Sherlock Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][2]++;
					break;
				case 8: // John Watson 
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					tableCartes[i][2]++;
					break;
				case 9: // Mycroft Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][4]++;
					break;
				case 10: // Mrs. Hudson 
					tableCartes[i][0]++;
					tableCartes[i][5]++;
					break;
				case 11: // Mary Morstan 
					tableCartes[i][4]++;
					tableCartes[i][5]++;
					break;
				case 12: // James Moriarty 
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					break;
			}
		}
	}
} 

void printDeck() //Affiche le deck
{
        int i,j;

        for (i=0;i<13;i++)
                printf("%d %s\n",deck[i],nomcartes[deck[i]]);

	for (i=0;i<4;i++)
	{
		for (j=0;j<8;j++)
			printf("%2.2d ",tableCartes[i][j]);
		puts("");
	}

	//printf("Le coupable est %d : %s\n",deck[12],nomcartes[deck[12]]);
}

void printClients() //Affiche la liste des clients tcp
{
        int i;

        for (i=0;i<nbClients;i++)
                printf("%d: %s %5.5d %s\n",i,tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        tcpClients[i].name);
}

int findClientByName(char *name) //Retourne l'indice du client dans la liste des clients tcp
{
        int i;

        for (i=0;i<nbClients;i++)
                if (strcmp(tcpClients[i].name,name)==0)
                        return i;
        return -1;
}

void sendMessageToClient(char *clientip,int clientport,char *mess)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(clientip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(clientport);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
                printf("ERROR connecting\n");
                exit(1);
        }

        sprintf(buffer,"%s\n",mess);
        n = write(sockfd,buffer,strlen(buffer));

    close(sockfd);
}

void broadcastMessage(char *mess) //Envoie un message à tous les clients tcp
{
        int i;

        for (i=0;i<nbClients;i++)
                sendMessageToClient(tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        mess);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
	int i;

        char com;
        char clientIpAddress[256], clientName[256];
        int clientPort;
        int id;
        char reply[256];


     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);


					//Etapes de création de la partie
	//printDeck();
	melangerDeck();
	createTable();
	printDeck();
	joueurCourant=0;

					//La partie peut commencer
	
	//Initialisation de la liste des clients tcp
	for (i=0;i<4;i++)
	{
        	strcpy(tcpClients[i].ipAddress,"localhost");
        	tcpClients[i].port=-1;
        	strcpy(tcpClients[i].name,"-");
	}

     while (1)
     {    
     	newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     	if (newsockfd < 0) 
          	error("ERROR on accept");

     	bzero(buffer,256);
     	n = read(newsockfd,buffer,255);
     	if (n < 0) 
		error("ERROR reading from socket");

        printf("Received packet from %s:%d\nData: [%s]\n\n",
                inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

        if (fsmServer==0) //On attend des connections d'autres joueurs
        {
        	switch (buffer[0]) //On regarde la commande
        	{
                	case 'C':
                        	sscanf(buffer,"%c %s %d %s", &com, clientIpAddress, &clientPort, clientName);
                        	printf("COM=%c ipAddress=%s port=%d name=%s\n",com, clientIpAddress, clientPort, clientName); //A DEGAGER QUAND ON AURA FINI LE CODE

                        	// fsmServer==0 alors j'attends les connexions de tous les joueurs
                                strcpy(tcpClients[nbClients].ipAddress,clientIpAddress);	//On remplit la structure du client avec les infos de l'adresse ip
                                tcpClients[nbClients].port=clientPort;						//On remplit la structure du client avec les infos du port
                                strcpy(tcpClients[nbClients].name,clientName);				//On remplit la structure du client avec les infos du nom
                                nbClients++;

                                printClients();							//J'affiche la table des clients

				// rechercher l'id du joueur qui vient de se connecter

                                id=findClientByName(clientName);		//On récupère un id pour le client
                                printf("id=%d\n",id);

				// lui envoyer un message personnel pour lui communiquer son id

                                sprintf(reply,"I %d",id);
                                sendMessageToClient(tcpClients[id].ipAddress,
                                       tcpClients[id].port,
                                       reply);

				// Envoyer un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement connectés

                                sprintf(reply,"L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name);
                                broadcastMessage(reply);

				// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu

                                if (nbClients==4) 			//Si les joueurs sont enfin au complet
				{
					// On envoie ses cartes au joueur 0, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d", deck[0*3], deck[0*3+1], deck[0*3+2]);
					sendMessageToClient(tcpClients[0].ipAddress, tcpClients[0].port, reply);

					for(i=0; i<8; i++){
						sprintf(reply, "V %d %d %d", 0, i, tableCartes[0][i]);
						sendMessageToClient(tcpClients[0].ipAddress, tcpClients[0].port, reply);
					}

					// On envoie ses cartes au joueur 1, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d", deck[1*3], deck[1*3+1], deck[1*3+2]);
					sendMessageToClient(tcpClients[1].ipAddress, tcpClients[1].port, reply);

					for(i=0; i<8; i++){
						sprintf(reply, "V %d %d %d", 1, i, tableCartes[1][i]);
						sendMessageToClient(tcpClients[1].ipAddress, tcpClients[1].port, reply);
					}

					// On envoie ses cartes au joueur 2, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d", deck[2*3], deck[2*3+1], deck[2*3+2]);
					sendMessageToClient(tcpClients[2].ipAddress, tcpClients[2].port, reply);

					for(i=0; i<8; i++){
						sprintf(reply, "V %d %d %d", 2, i, tableCartes[2][i]);
						sendMessageToClient(tcpClients[2].ipAddress, tcpClients[2].port, reply);
					}

					// On envoie ses cartes au joueur 3, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d", deck[3*3], deck[3*3+1], deck[3*3+2]);
					sendMessageToClient(tcpClients[3].ipAddress, tcpClients[3].port, reply);

					for(i=0; i<8; i++){
						sprintf(reply, "V %d %d %d", 3, i, tableCartes[3][i]);
						sendMessageToClient(tcpClients[3].ipAddress, tcpClients[3].port, reply);
					}

					// On envoie enfin un message a tout le monde pour definir qui est le joueur courant = 0
					sprintf(reply, "M %d", joueurCourant);
					broadcastMessage(reply);
					
                    fsmServer=1;
				}
				break;
                }
	}
	else if (fsmServer==1) //Si tout le monde est connecté, phase de JEU
	{
		switch (buffer[0])
		{
                	case 'G':						//Déclarer un Guilty (considérer que N° ... est le méchant)
					int GuiltId, askGuilty;
					sscanf(buffer,"G %d %d", &GuiltId, &askGuilty);

					//Si le joueur qui a envoyé le message a accusé la dernière carte du deck, il a gagné
					if (askGuilty == deck[12]){
						endgame(GuiltId);
						sprintf(reply, ":D Le joueur %d a bien deviné, il a gagné :D \n", GuiltId);
						broadcastMessage(reply);
						return(0);
					}

					//Si le joueur qui a envoyé le message n'a pas accusé la dernière carte du deck, il est éliminé
					if(askGuilty != deck[12]){
						joueurselimines[GuiltId] = 1;
						sprintf(reply, "M %d", 4);
						sendMessageToClient(tcpClients[GuiltId].ipAddress, tcpClients[GuiltId].port, reply);
						sprintf(reply, ":( Le joueur %d a mal deviné, il a perdu :( \n", GuiltId);
						broadcastMessage(reply);
						joueurCourant = joueursuivant(joueurCourant, &joueurselimines);
						if (joueurCourant == 4){	//On sort du jeu
							sprintf(reply, ":(( Tous les joueurs ont été éliminés, vous avez perdu :((");
							broadcastMessage(reply);
							return(0);
						}
					}

					

				break;
                	
					case 'O': 						//Qui a tel symbole, on demande au serveur global ?
					int askId, askSymbole;
					sscanf(buffer,"O %d %d", &askId, &askSymbole);
					for(int i = 0; i < 4; i++)
					{	
						if (i == askId)
						{
							//On passe au joueur suivant
							continue;
						}
						
						else{
							for(int j = 0; j < 3; j++){
								if(tableCartes[i][askSymbole] != 0){
									//Le joueur i a le symbole de la colonne askSymbole - On l'envoie à tous les joueurs
									sprintf(reply, "V %d %d %d", i, askSymbole, 100);
									broadcastMessage(reply);
								}
								else{
									//Le joueur i n'a pas le symbole de la colonne askSymbole - On l'envoie à tous les joueurs
									sprintf(reply, "V %d %d %d", i, askSymbole, 0);
									broadcastMessage(reply);
								}
							}
						}
					}
					joueurCourant = joueursuivant(joueurCourant, &joueurselimines);

				break;
			case 'S': 								//Toi, joueur n° tant, combien as-tu de collonnes associées à tel symbole ? 
				int askingId, askedId, askSymboleS;
				sscanf(buffer,"S %d %d %d", &askingId, &askedId, &askSymboleS);
				if(tableCartes[askedId][askSymboleS] !=0){

					//Le joueur askedId a le symbole askSymbole - On l'envoie au joueur qui a demandé (askId)
					sprintf(reply, "V %d %d %d", askedId, askSymboleS, tableCartes[askedId][askSymboleS]);
					sendMessageToClient(tcpClients[askingId].ipAddress, tcpClients[askingId].port, reply);
				}
				else{
					//Le joueur askedId n'a pas le symbole askSymbole - On l'envoie au joueur qui a demandé (askId)
					sprintf(reply, "V %d %d %d", askedId, askSymboleS, 0);
					sendMessageToClient(tcpClients[askingId].ipAddress, tcpClients[askingId].port, reply);
					continue;
				}
				joueurCourant = joueursuivant(joueurCourant, &joueurselimines);
				break;
                	default:
                        	break;
		}
        }
     	close(newsockfd);
     }
     close(sockfd);
     return 0; 
}
