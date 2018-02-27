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


int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;

    char buffer[BUFLEN], str[BUFLEN], buf[100];

    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }

	//creare fisier log
    int pid = getpid(); //pid-ul procesului
    sprintf(buf, "client-%d.log", pid);
    FILE *fo = fopen(buf, "a");

    if (fo == NULL)
        printf("-10 : Eroare la apel fopen\n");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);

	if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
        printf("-10 : Eroare la apel connect\n");

    int ok = 0;

	while(1){

		// se primeste de la stdin

    	memset(buffer, 0 , BUFLEN);
    	fgets(buffer, BUFLEN - 1, stdin);

        char comm[BUFLEN], second_word[BUFLEN];
        sscanf(buffer, "%s", comm); //primul cuvant din comanda

        fprintf(fo, "%s", buffer); //scriu comanda in log

    	//trimit mesaj la server
        if (strcmp(comm, "login") == 0) {

        	if (ok == 0) {

	        	send(sockfd, buffer, strlen(buffer), 0);
	        	recv(sockfd, str, sizeof(str), 0);
	        	if (strcmp(str, "Welcome") != 0) {
	        		printf("ATM> %s\n", str);
                    fprintf(fo, "ATM> %s\n\n", str);
                }

	        	if (strcmp(str, "Welcome") == 0) { //daca ma autentifica imi stochez userul

	        		ok = 1;
	        		strcpy(second_word, buffer + strlen("login") + 1); //imi stochez user-ul logat, adica nr card
	        		recv(sockfd, str, sizeof(str), 0); //fac receive sa primesc user-ul complet
	        		printf("ATM> Welcome %s\n", str);
                    fprintf(fo, "ATM> Welcome %s\n\n", str);
        		}
        	}
        	else if (ok == 1) {
                //sunt deja autentificat
        		printf("-2 : Sesiune deja deschisa\n");
                fprintf(fo, "-2 : Sesiune deja deschisa\n");
        	}
        }
        else if (strcmp(comm, "logout") == 0 && ok == 0) {
                //nu sunt autentificat si dau logout
                if (strlen(buffer) == 7) {
                    printf("-1 : Clientul nu e autentificat\n");
                    fprintf(fo, "-1 : Clientul nu e autentificat\n\n");
                }
        }
        else if (strcmp(comm, "logout") == 0 && ok == 1) {
                //sunt autentificat si dau logout
                if (strlen(buffer) == 7) {
                    send(sockfd, buffer, strlen(buffer), 0);
                    printf("ATM> Deconectare de la bancomat\n");
                    fprintf(fo, "ATM> Deconectare de la bancomat\n\n");
                    ok = 0;
                }
        }
        else if (strcmp(comm, "listsold") == 0 && ok == 1) {
       		send(sockfd, buffer, strlen(buffer), 0);
       		recv(sockfd, str, sizeof(str), 0);
       		printf("ATM> %s\n", str);
            fprintf(fo, "ATM> %s\n\n", str);
        }
        else if (strcmp(comm, "listsold") == 0 && ok == 0) {
            printf("-1 : Clientul nu e autentificat\n");
            fprintf(fo, "-1 : Clientul nu e autentificat\n\n");
        }
        else if (strcmp(comm, "quit") == 0 && ok == 1) {
            send(sockfd, buffer, strlen(buffer), 0);
            fclose(fo);
            close(sockfd);
            return 0;
        }
        else if (strcmp(comm, "quit") == 0 && ok == 0) {
            printf("-1 : Clientul nu e autentificat\n");
            fprintf(fo, "-1 : Clientul nu e autentificat\n\n");
        }
        else if (strcmp(comm, "getmoney") == 0 && ok == 1) {
            send(sockfd, buffer, strlen(buffer), 0);
            recv(sockfd, str, sizeof(str), 0);
            printf("ATM> %s\n", str);
            fprintf(fo, "ATM> %s\n\n", str);
        }
        else if (strcmp(comm, "getmoney") == 0 && ok == 0) {
            printf("-1 : Clientul nu e autentificat\n");
            fprintf(fo, "-1 : Clientul nu e autentificat\n\n");
        }
        else if (strcmp(comm, "putmoney") == 0 && ok == 1) {
            send(sockfd, buffer, strlen(buffer), 0);
            recv(sockfd, str, sizeof(str), 0);
            printf("ATM> %s\n", str);
        }
        else if (strcmp(comm, "putmoney") == 0 && ok == 0) {
            printf("-1 : Clientul nu e autentificat\n");
            fprintf(fo, "-1 : Clientul nu e autentificat\n\n");
        }
        else {

            printf("-6 : Operatie esuata\n");
            fprintf(fo, "-6 : Operatie esuata\n\n"); //pentru alte comenzi invalide
        }

	}

    close(sockfd);
    fclose(fo);

}
