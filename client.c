/* Sviluppato con cura da Antonio Vitale [754740], Angelo Sciarra[758256], Antonio Troncellito[754736] */

#if defined WIN32 // se l'OS è win32 includiamo la libreria <winsock.h>
    #include <winsock.h>
#else // se l'OS fa parte della famiglia UNIX
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif // questo if defined controlla che l'OS sia win32 piuttosto che UNIX

// librerie standard per ambo gli OS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  
#define ECHOMAX 255

void ClearWinSock(){
	#if defined WIN32
	WSACleanup();
	#endif
}

// Funzione che ritorna 1 se il carattere passato è una vocale, 0 altrimenti
int isVocale(char car){
	char vocali[] = {'a', 'e', 'i', 'o', 'u'};
	int i;
	for(i=0; i<5; i++){
		if(car == vocali[i]){
			return 1;
		}
	}
	return 0;
}

int main(){
    #if defined WIN32
        WSADATA wsaData; // inizializziamo un elemento WSADATA per essere sicuri che le socket windows siano supportate dal sistema
        // MAKEWORD(2,2) specifica il numero di winsock sul sistema
        int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
        if (iResult != 0) {
            printf ("error at WSASturtup\n");
            return EXIT_FAILURE;
        }
    #endif

	char nomeServer[30];
	int numeroPorta;

	// Inserimento nome server da input
	printf("Inserisci il nome del server: ");
	scanf("%19s", nomeServer);

	// Inserimento numero porta da input 
	printf("Inserisci il numero della porta: ");
	scanf("%d", &numeroPorta);

	// Traduzione del nome server in indirizzo internet (metodo gethostbyname()) attraverso DNS
	struct hostent *serverAddr;
	struct in_addr addr;

	serverAddr = gethostbyname(nomeServer);

	if(serverAddr == NULL){
        printf("Impossibile tradurre il nome. Verra automaticamente inserito l'hostname: localhost\n");
        char *nomeServerDefault = "localhost";
        serverAddr = gethostbyname(nomeServerDefault);
	}
    int i = 0;
    if(serverAddr -> h_addrtype == AF_INET){
        while(serverAddr -> h_addr_list[i] != 0){
            addr.s_addr = *(u_long *) serverAddr -> h_addr_list[i++];
        }
    }

    // Creazione della socket
    // int socket (int famigliaProtocolli, int tipoDiSocket, int portocolloDaUsare)
	// nel nostro caso PF_INET = Internet Protocol Family, SOCK_DGRAM poichè è una socket UDP, IPPROTO_UDP)	
    int sock;
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        printf("Creazione della socket fallita.");
        return -1;
    }

    // Creazione dell'indirizzo del server
    struct sockaddr_in serverUDPaddr;
    struct sockaddr_in fromAddr;
    memset(&serverUDPaddr, 0, sizeof(serverUDPaddr));
    serverUDPaddr.sin_family = PF_INET;
  	// chiediamo all'utente di inserire la porta del server da contattare
	// se ne inserisce una errata utilizziamo una di default
    if(numeroPorta != 9069){
        printf("La porta inserita non e' corretta, verra' automaticamente inserita la porta 9069...\n");
        numeroPorta = 9069;
    }
    serverUDPaddr.sin_port = htons(numeroPorta);
    serverUDPaddr.sin_addr.s_addr = addr.s_addr;

    // invio della stringa di benvenuto al server
    char *stringaBenvenuto = "Ciao, io sono il client!";
    int lunghezzaStringa = strlen(stringaBenvenuto);
    if(sendto(sock, stringaBenvenuto, lunghezzaStringa, 0, (struct sockaddr*)&serverUDPaddr, sizeof(serverUDPaddr)) != lunghezzaStringa){
        printf("La sendto() ha fallito.\n");
        ClearWinSock();
        return -1;
    }

    // Ricevo stringa di conferma dal server
    int fromSize;
    char buffer[ECHOMAX];
    fromSize = sizeof(fromAddr);
    memset(buffer,0,ECHOMAX);
    int bufferSize = recvfrom(sock, buffer, ECHOMAX, 0, (struct sockaddr*)&fromAddr, &fromSize);
    if(serverUDPaddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
        printf("Errore! Il pacchetto ricevuto proviene da un'altra macchina (IP: %s).\n", inet_ntoa(fromAddr.sin_addr));
        return -1;
    }
    printf("%s\n", buffer);

    // Chiedo all'utente la stringa da tradurre
    char strClient[60];
    printf("Inserisci la stringa da inviare al server: ");
    while(getchar() != '\n');
    fgets(strClient,59,stdin);

    // Ricezione (recv()) e invio (send()) delle vocali
    printf("Caratteri ricevuti: ");
    for(i=0; i<strlen(strClient); i++){
        char carattere;
        carattere = tolower(strClient[i]);
        if(isVocale(carattere) == 1){
            if(sendto(sock, &carattere, sizeof(char), 0, (struct sockaddr*)&serverUDPaddr, sizeof(serverUDPaddr)) != sizeof(char)){
                printf("Errore nell'invio del carattere.\n");
            }
            recvfrom(sock, &carattere, sizeof(char), 0, (struct sockaddr*)&fromAddr, &fromSize);
            if(serverUDPaddr.sin_addr.s_addr == fromAddr.sin_addr.s_addr){
                printf("%c ", carattere); // stampa del carattere
            }
        }
    }
    printf("\n");
    // Invio di un carattere '$' che ci permette di interrompere la connessione con il server (viene controllato server side)
    char carattereFine = '$';
    if(sendto(sock, &carattereFine, sizeof(char), 0, (struct sockaddr*)&serverUDPaddr, sizeof(serverUDPaddr)) != sizeof(char)){
        printf("Errore nell'invio del carattere.\n");
    }

    system("pause");
    closesocket(sock);

    return 0;
    
} // end main()
