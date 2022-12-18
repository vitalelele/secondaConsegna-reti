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

// librerie standard per gli OS
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h> 

#define PORTA_SERVER 9069  // definiamo la porta server da utilizzare
#define MAX_CONNESSIONI 5 // definiamo un limite massimo di connessioni
#define BUFFERSIZE 512   // definiamo la grandezza del buffer come costante (BUFFERSIZE)
#define ECHOMAX 255	    // definiamo la lunghezza MAX di una stringa che si può mandare (MAXSTR) abbiamo scelto 255 perché è
					   // più o meno la metà del buffersize (512)


void ClearWinSock(){
    #ifdef WIN32
        WSACleanup;
    #endif
}

void ErrorHandler(char *errorMessage) {
	printf(errorMessage);
}


int main(int argc,char *argv[]){
	#if defined WIN32
	WSADATA wsaData;  // inizializziamo un elemento WSADATA per essere sicuri che le socket windows siano supportate dal sistema
    // MAKEWORD(2,2) specifica il numero di winsock sul sistema
	int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
	if (iResult != 0) {
		printf ("Errore nell'inizializzazione di WSAStartup()\n");
		return EXIT_FAILURE;
	}
	#endif

	int sock;
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	unsigned int cliAddrLen;
	// creazione socket UDP
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		printf("Creazione della socket fallita.\n");
	}

	// costruzione indirizzo server
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORTA_SERVER);
	// poichè il server non contatta nessuno questo indirizzo sarà sempre localhost (127.0.0.1)
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

     	
	// assegnamo la porta e l'ip alla socket e verifichiamo se non ci sono errori
	// int bind (int socket, struct sockaddr* localaddress, int addressLength)
	// bind ritorna 0 in caso di successo e -1 altrimenti
	if((bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) < 0){
		printf("Il metodo bind() ha fallito.\n");
	}
	char buffer[ECHOMAX]; // creiamo il nostro buffer
	
	// Questo while ci permette di rimanere in ascolto anche se il client cade.
	while(1){
		int clientAddrLen = sizeof(clientAddr);

        // void * memset( void *buffer, int c, size_t count )
		// copia il valore di "c" nell'area di memoria (puntata da una variabile di tipo Puntatore ) 
		// per una quantità stabilita nell'argomento count
		memset(&buffer,0,sizeof(buffer));
		recvfrom(sock, buffer, ECHOMAX, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
		printf("%s", buffer);

		// Traduzione da IP a hostname del client attraverso il DNS (metodo gethostbyaddr())
		struct hostent *host;
		host = gethostbyaddr((char *) &clientAddr.sin_addr, 4, AF_INET);

        printf(" Messaggio ricevuto dal client %s\n\n", host->h_name);

        // Invio risposta al messaggio di benvenuto del client
		char *rispostaBenvenuto = "OK";
		if( sendto(sock, rispostaBenvenuto, strlen(rispostaBenvenuto), 0 ,(struct sockaddr*)&clientAddr, clientAddrLen) != strlen(rispostaBenvenuto) ){
			printf("Errore nell'invio rispostaBenvenuto.\n'");
		}

        // Ricezione e invio delle vocali
		char vocaleRicevuta = ' '; 
		struct sockaddr_in fromAddr;
		int fromSize = sizeof(fromAddr);
		while(1){
            recvfrom(sock, &vocaleRicevuta,sizeof(char),0,(struct sockaddr*)&fromAddr, &fromSize);
           // controlliamo che la recvfrom() vada a buon fine, eventualmente avvisiamo l'utente del problema
		    if(clientAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
                printf("Vocale ricevuta da un'altro client, IP: %s",inet_ntoa(fromAddr.sin_addr));
                vocaleRicevuta = ' ';
            }
            // Controlliamo che la vocale ricevuta non sia l'ultima (carattere '$' definito in client come flag d'uscita)
            if(vocaleRicevuta != '$'){
               vocaleRicevuta = toupper(vocaleRicevuta);
                if(sendto(sock, &vocaleRicevuta, sizeof(char), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) != sizeof(char)){
                    printf("Errore nell'invio del carattere.\n");
                }
            }else{
                break;
            }
		}
	}
	closesocket(sock); // chiudiamo la socket e terminiamo il programma
} // end main()
