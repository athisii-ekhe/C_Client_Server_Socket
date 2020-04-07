#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/select.h>

#define MAX( a, b ) ( ( a > b) ? a : b )
#define TRUE 1

void error(char *msg)
 {
 	perror(msg);
 	exit(1);
 }

void upper_string(char s[]) {
   int c = 0;
   while( s[c] != '\0') 
   {
      if (s[c] >= 'a' && s[c] <= 'z') {
         s[c] = s[c] - 32;
      }
      c++;
   }
 }

int Read(int sockfd, char *buffer, int size)
 {
	 int n;
	 n = read(sockfd, buffer, size);
	 if(n<=0)
	  {
		fprintf(stderr, "Server terminated prematurely.\n");
		exit(EXIT_FAILURE);
	  }
	 else
	  {
		 return n;
	 }
	 
 }

int Send(int sockfd, char *buffer, int size)
 {
	 int n;
	 n = send(sockfd, buffer, size,0);
	 if(n<=0)
	  {
		fprintf(stderr, "Server terminated prematurely.\n");
		exit(EXIT_FAILURE);
	  }
	 else
	  {
		 return n;
	 }
	 
 }


int main(int argc, char *argv[])
 {
 	int sockfd, portno, n;
 	char buffer[1024]={}, password[1024]={}, id[1024]={};
 	struct sockaddr_in serv_addr;

 	if(argc <3)
 	 {
 	 	fprintf(stderr, "usage %s hostname port\n", argv[0]);
 	 	exit(1);
 	 }

 	sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if(sockfd < 0)
 	 {
 	 	error("Error opening socket.");
 	 }
	
 	portno = atoi(argv[2]);
 	bzero((char *)&serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
 	// bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 	serv_addr.sin_port = htons(portno);
	
 	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
 	 {
 	 	error("Connection Failed.");
 	 }

	// printf("Server Login Info\nid: client \npassword: 1234\n");
	read(sockfd, buffer, 1024);
	printf("%s\n", buffer);

	//Prompt for id input
	bzero(buffer, sizeof(buffer));
	read(sockfd, buffer, sizeof(buffer));
	printf("%s\n",buffer);
	fgets(id, sizeof(id), stdin);
	Send(sockfd, id, strlen(id));

	//Prompt for password input
	bzero(buffer, sizeof(buffer));
	read(sockfd, buffer, sizeof(buffer));
	printf("%s\n", buffer);
	fgets(password, sizeof(password), stdin);
	Send(sockfd, password, strlen(password));
	
	bzero(buffer, sizeof(buffer));
	Read(sockfd, buffer, sizeof(buffer));

	if(strncmp(buffer, "SORRY,", 6) == 0)
	 {
		 printf("%s", buffer);
		 exit(1);
	 }

	printf("%s\n", buffer);

	while(TRUE)
	 {
		printf("CLIENT: ");
		bzero(buffer, sizeof(buffer));
		fgets(buffer, 1025, stdin);
		upper_string(buffer); //Converting to uppercase.
		Send(sockfd, buffer, strlen(buffer));


		
		//Check if input is SOUND
		if(strcmp(buffer, "SOUND\n") == 0)
		 {
			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n", buffer);

			//input from stdin
			bzero(buffer, sizeof(buffer));
			fgets(buffer, 1025, stdin);
			upper_string(buffer);
			Send(sockfd, buffer, strlen(buffer));


			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
		
		 }
		
		//Check if input is STORE
		else if(strncmp(buffer, "STORE\n", 6) == 0)
		  {
			
			//input from stdin 
			bzero(buffer, sizeof(buffer));
			fgets(buffer, 1025, stdin);
			upper_string(buffer);
			Send(sockfd, buffer, strlen(buffer));

			//input from stdin
			bzero(buffer, sizeof(buffer));
			fgets(buffer, 1025, stdin);
			upper_string(buffer);
			Send(sockfd, buffer, strlen(buffer));


			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n\n",buffer);


		  }

		//Check if input is QUERY
		else if(strcmp(buffer, "QUERY\n") == 0)
		 {
			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n\n",buffer);

		 }
		
		//Check if input is BYE OR END
		else if((strcmp(buffer,"BYE\n")) ==0 || (strcmp(buffer, "END\n"))==0)
		 {
			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
			close(sockfd);
			break;
		 }

		//DISPLAYS HELP
		else
		 {
			bzero(buffer, sizeof(buffer));
			Read(sockfd, buffer, 1024);
			printf("%s\n",buffer);
		 }
	 }

 	return 0;
 }