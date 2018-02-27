#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <dirent.h>
#include <fstream>

#define BUFLEN 256
#define MAX_CLIENTS 25

typedef struct {
	std::string surname, name, password;
	double sold;
	int pin, no_card;
    int nrWrong;//folosita pentru card blocat
} userStruct;

typedef struct {
	std::string name_logged;
	int no_card;
	int position;//pozitia in vectorul de useri
	int socket_no;
} logStruct;


std::vector<logStruct> vector_log;
std::vector<userStruct> userlist;

//seteaza pentru fiecare user vaiabila nrWrong cu 0
void set_zeros() {

	unsigned int i;
	for (i = 0 ; i < userlist.size() ; i++) {
		userlist[i].nrWrong = 0;
	}
}

/*Primeste ca parametru numarul cardului si il cauta
in vectorul de logati si returneaza socketul corespunzator acelui int.
In caz negativ, -2 (valoarea e folosita de logout exclusiv)*/
int find_log(int no_card) {

    unsigned int i;
    for (i = 0 ; i < vector_log.size() ; i++) {
        if (vector_log[i].no_card == no_card)
            return vector_log[i].socket_no;
    }
    return -2;
}

/*Primeste ca parametru un socket number si returneaza
pozitia corespunzatoare din vectorul logatilor. In caz negativ, -1.*/
int find_socket(int socket) {

    unsigned int i;
    for (i = 0 ; i < vector_log.size() ; i++) {
        if (vector_log[i].socket_no == socket)
            return i;
    }
    return -1;
}

/*Primeste ca parametru un numar de card si returneaza
pozitia din vectorul userilor. In caz negativ, -1.*/
int find_noCard(int no_card, int N) {

	int i;
	for (i = 0 ; i < N ; i++) {
		if (userlist[i].no_card == no_card)
			return i; //returnez pozitia din struct
	}
	return -1;
}

/*Primeste ca parametru un numar PIN si verifica daca exista
in vectorul userilor.*/
int find_pin(int pin_card, int poz, int N) {

	if (userlist[poz].pin == pin_card)
			return 1;
	return -1;
}

/*Primeste ca parametru un socket si verifica daca
exista in vectorul logatilor. In caz pozitiv, intoarce nr cardului.*/
int find_noCard_socket(int socket) {

    unsigned int i;
    for (i = 0 ; i < vector_log.size() ; i++) {
        if (vector_log[i].socket_no == socket)
            return vector_log[i].no_card;
    }
    return -1;
}


int main(int argc, char** argv) {

	if (argc != 3) {
		// <port_server> <users_data_file>
		printf("Numar de argumente incorect\n");
		return 0;
	}

	int i, n, contor = 0, ok = 0, N, var;
	int sockfd, newsockfd, portno;
    char buffer[BUFLEN], line[1000], file_name[50];
    struct sockaddr_in serv_addr, cli_addr;
	//citire din fisier
	double varSold;
	sprintf(file_name, "%s", argv[2]);
	FILE *users;
	users = fopen(file_name, "r");
	if (users == NULL) {

		printf("-10 : Eroare la apel fopen\n");
	}

	userStruct u;
	fscanf(users, "%d", &N);

	for (i = 0 ; i < N; i++) {

		fscanf(users, "%s", line);//citesc prenume
		u.surname = line;
		fscanf(users, "%s", line); //citesc nume
		u.name = line;
		fscanf(users, "%d", &var); //numar card
		u.no_card = var;
		fscanf(users, "%d", &var); // pin
		u.pin = var;
		fscanf(users, "%s", line); //parola
		u.password = line;
		fscanf(users, "%lf", &varSold); //sold
		u.sold = varSold;
		userlist.push_back(u);
	}

	set_zeros(); //seteaza 0 campul nrWrong din userStruct

	// ================== Conexiunea TCP ====================

    fd_set read_fds; //multimea de citire folosita in select
    fd_set temp_fds; //multime temp
    int fdmax; ////valoare maxima file descriptor din multimea read_fds

    ////golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);

    //deschid socket-ul
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        printf("Error opening socket\n");

    //initializari, port
    portno = atoi(argv[1]);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
     serv_addr.sin_port = htons(portno);


     //bind
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
            printf("-10 : Eroare la apel bind\n");


     //listen
     listen(sockfd, MAX_CLIENTS);

     // //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
     FD_SET(sockfd, &read_fds);
     FD_SET(0, &read_fds);
     fdmax = sockfd;

	while(1) {

     	temp_fds = read_fds;
     	if (select(fdmax + 1, &temp_fds, NULL, NULL, NULL) == -1)
     		printf("-10 : Eroare la apel select\n");

     	for (i = 0 ; i <= fdmax; i++) {
     		if (!FD_ISSET(i, &temp_fds)) continue;

     		if (i == 0) {
        		// se primeste de la stdin
				memset(buffer, 0, BUFLEN);
				fgets(buffer, BUFLEN - 1, stdin);
				sscanf(buffer, "%s", buffer);
				if (!strcmp(buffer, "quit")) {
					close(sockfd);
					return 0;
				}
     		}

     		else if (i == sockfd) { //a venit ceva pe socket inactiv = noua conexiune

     				socklen_t clilen = sizeof(cli_addr);
     				newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
     				if (newsockfd == -1) {
     					printf("-10 : Eroare la apel accept\n");
     				}
     				else {

     					//adaug noul socket intors de accept() la mult descriptorilor de citire
     					FD_SET(newsockfd, &read_fds);
     					if (newsockfd > fdmax)
     						fdmax = newsockfd;

     				}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
     				memset(buffer, 0, BUFLEN);
     				n = recv(i, buffer, sizeof(buffer), 0);
     				if (n <= 0) {
	     				close(i);
	     				FD_CLR(i, &read_fds); //scot socket-ul
     				}

     				if (n < 0)
     					printf("-10 : Eroare la apel receive\n");

     				else {

     					int posInStruct = 0, pos = 0, arg1;
     					char cod[50], first_word[BUFLEN], second_word[BUFLEN];
     					sscanf(buffer, "%s", first_word); //primul cuvant din comanda
     					logStruct l;

     					//Impart comanda in cuvinte
     					char *words[100];
     					char *aux = strtok(buffer, " ");

     					while (aux) {
     						words[pos] = aux;
     						pos++;
     						aux = strtok(NULL, " \n");
     					}

     					if (strcmp(first_word, "login") == 0) {

     						posInStruct = find_noCard(atoi(words[1]), N); //poz din struct bazata pe no_card

     						if (posInStruct == -1) { //nu exista nr card
     							sprintf(cod, "-4 : Numar card inexistent");
     							send(i, cod, BUFLEN, 0);

     						}
     						else { //nr card exista, check PIN la aceeasi position.
                                if(words[2] == NULL) { //se da doar login <something>
                                    sprintf(cod, "-3 : Pin gresit");
                                    send(i, cod, BUFLEN, 0);
                                }
                                else {

     							    int pinInStruct = find_pin(atoi(words[2]), posInStruct, N); //verific PIN-ul la aceeasi pozitie.

         							if (pinInStruct == 1) {
                                        //verific daca e logat pe alta sesiune

                                        if(find_log(atoi(words[1])) != -2) { //intoarce socket-ul
                                            sprintf(cod, "-2 : Sesiune deja deschisa");
                                            send(i, cod, BUFLEN, 0);
                                        }
                                        else {

	                                        strcpy(second_word, buffer + strlen("login") +1);
	                                        std::string name_logged;
	                                        std::string surname_logged;
	                                        std::string complet_name;

	                                        name_logged = userlist[posInStruct].surname;
	                                        surname_logged = userlist[posInStruct].name;
	                                        complet_name = name_logged + ' ' + surname_logged;

	                                        arg1 = atoi(words[1]); //numar card

	                                        l.name_logged = complet_name;
	                                        l.position = posInStruct;
	                                        l.no_card = atoi(words[1]);
	                                        l.socket_no = i;
	                                        vector_log.push_back(l); //l-am logat cu numar card + prenume + pozitie.

	                                        sprintf(cod, "Welcome");
	                                        send(i, cod, BUFLEN, 0);
	                                        userlist[posInStruct].nrWrong = 0;
	                                        sprintf(cod, "%s", complet_name.c_str());
	                                        send(i, cod, BUFLEN, 0); //trimit prenume user
	                                    }
                                    }

                                    else if (pinInStruct == -1) {//daca numar card exista si PIN e gresit sau nu exista
                                        userlist[posInStruct].nrWrong++;
                                        if (userlist[posInStruct].nrWrong == 3) {
                                            //blochez cardul
                                            sprintf(cod, "-5 : Card blocat");
                                            send(i, cod, BUFLEN, 0);
                                        }

                                        sprintf(cod, "-3 : Pin gresit");
                                        send(i, cod, BUFLEN, 0);
                                        }
                                    }
         						}
         					}
     					else if (strcmp(first_word, "logout") == 0) {
     						ok = 0;
     						contor = 0;
                            int poz = find_socket(i);
                            vector_log[poz].position = -2;
                            vector_log[poz].no_card = -2;
                            vector_log[poz].socket_no = -2;
     					}

     					else if (strcmp(first_word, "listsold") == 0) {
     						int poz = find_noCard(find_noCard_socket(i), N); //pozitia userului din strctura cu nr card, bazata pe socket-ul activ
     						double sold = userlist[poz].sold;
     						sprintf(cod, "%.2f", sold);
     						send(i, cod, BUFLEN, 0);
     					}

                        else if (strcmp(first_word, "getmoney") == 0) {
                            int poz = find_noCard(find_noCard_socket(i), N);
                            double sold = userlist[poz].sold;
                            int askSum = atoi(words[1]);

                            if (askSum > sold) {
                                sprintf(cod, "-8 : Fonduri insuficiente");
                            } else if (askSum % 10 != 0 ) {
                                sprintf(cod, "-9 : Suma nu este multiplu de 10");
                            } else {
                                userlist[poz].sold -= (double)askSum;//scad suma dorita din soldul curent
                                sprintf(cod, "Suma %d retrasa cu succes", askSum);
                            }
                            send(i, cod, BUFLEN, 0);
                        }

                        else if (strcmp(first_word, "putmoney") == 0) {
                            int poz = find_noCard(find_noCard_socket(i), N);
                            double putSum = atof(words[1]);

                            userlist[poz].sold += putSum;//adaug suma dorita la soldul curent
                            sprintf(cod, "Suma %.2f depusa cu succes",putSum);
                            send(i, cod, BUFLEN, 0);
                        }
     				}
				}
		}
	}
	return 0;

}
