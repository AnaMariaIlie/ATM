
	
	
	
	
					README 
					
					
					
		Pentru realizarea temei 2(Sistem monetar de tip Bancomat) am folosit
limbajul C++, in special pentru a beneficia de facilitati in lucrul cu structurile.
De asemenea, am folosit scheletul de cod si notiunile dobandite in laboratorul 8
(Socketi TCP. Multiplexarea I/O) pentru a putea functiona cu mai multi clienti
(asa cum se cere), cu ajutorul functiei select(in server) pentru controlarea mai
multor socketi in acelasi timp.
	Am definit structurile:
		"userStruct" - care contine credentialele din fisierul users_data_file.txt
(nume,prenume,parola,sold,pin,numar card).
		"logStruct" - structura folosita pentru a tine evidenta userilor logati.
Tot in server am creat un vector de useri(pentru a parcurge/cauta informatii despre
useri) si un vector de logati(pentru toti userii activi).
	Dupa ce am stabilit conexiunea dintre server si client incep sa primesc comenzi:
	Pentru fiecare cerere impart comanda in cuvinte pentru a le manipula usor.
	- Comanda "login" trimisa de client (prin "send()"):
		In server, la primirea acestei comenzi verifica daca numarul cardului exista.
In caz ca nu exista trimit inapoi clientului respectiv codul de eroare "−4 :
Numar card inexistent". Daca exista ii verific pinul. Daca pinul este gresit trimit
"-3 : Pin gresit" sau daca nu este gresit trimit catre client "Welcome" si il adaug
in vectorul de useri logati. Daca userul e deja logat pe un alt socket atunci intorc
codul "-2 : Sesiune deja deschisa". De asemenea, pentru un card existent, daca se
incearca logarea de 3 ori(de pe oricare sesiune) cardul se va bloca("-5 : Card blocat").
	In client, daca clientul este deja logat nu se mai trimite la server comanda
de login si se afiseaza mesajul "-2 : Sesiune deja deschisa".
	-Comanda "logout" trimisa de client (prin "send()")
		In client, daca nu sunt autentificat si dau comanda de logout nu trimit
nimic catre server si afisez mesajul "−1 : Clientul nu este autentificat". Daca
sunt autentificat trimit comanda la server.
		In server, caut in vectorul de logati socketul i si obtin pozitia celui
care a dat comanda, astfel actualizez vectorul de logati cu valoarea -2 pentru a-l
putea deosebi de userii care sunt inca logati.
	-Comenzile "listsold","getmoney" si "putmoney" functioneaza toate pe acelasi
principiu:
	Obtin numarul cardului corespunzator socketului si cu el obtin pozitia din
vectorul de useri. In functie de ce se cere actualizez si soldul(retragere sau depunere).
	-Comanda "quit" pe server se primeste de la stdin si inchide programul.
	Pentru comanda "quit" pe client trimit instiintare la server si inchid conexiunea,
fisierul si ies din program.

	Mentiuni: pentru comenzi valide in client cand userul nu e autentificat nu trimit
comanda la server, dar afisez "−1 : Clientul nu este autentificat".
		Pentru oricare alta comanda afisez "-6 : Operatie esuata".
		Nu am implementat comanda "unlock".
		In client scriu fiecare comanda si rezultatul ei in fisierul corespunzator.
		Am pastrat o parte din cod si de asemenea si comentariile(date in schelet)
din laboratorul 8(TCP multiplexare).
		
	
