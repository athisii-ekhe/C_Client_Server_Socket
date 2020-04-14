#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "helper.h"

#define MAX(a, b) ((a > b) ? a : b)
#define TRUE 1

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	char buffer[1024], id[1024], password[1024];
	struct sockaddr_in serv_addr;

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("Error opening socket.");
	}

	portno = atoi(argv[2]);
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
	// bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		error("Connection Failed.");
	}

start:
	//Clears buffers
	bzero(buffer, sizeof(buffer));
	bzero(id, sizeof(id));
	bzero(password, sizeof(password));

	//Reads server message and selects the options
	Recv(sockfd, buffer, sizeof(buffer));
	fputs(buffer, stdout);
	bzero(buffer, sizeof(buffer));
	fgets(buffer, sizeof(buffer), stdin);
	Send(sockfd, buffer, strlen(buffer));
	if (strncmp(buffer, "1\n", 2) == 0) //New Registration
	{

		//Gets id and password from stdin
		fputs("\tNew Registration\nEnter your id: ", stdout);
		fgets(id, sizeof(id), stdin);
		Send(sockfd, id, strlen(id));
		fputs("Enter your password: ", stdout);
		fgets(password, sizeof(password), stdin);
		Send(sockfd, password, strlen(password));
		fputs("\n", stdout);
		bzero(buffer, sizeof(buffer));
		Recv(sockfd, buffer, sizeof(buffer));
		fputs(buffer, stdout);
	}
	else if (strncmp(buffer, "2\n", 2) == 0) //Else Login
	{

		//Gets id and password from stdin
		fputs("Enter your id: ", stdout);
		fgets(id, sizeof(id), stdin);
		Send(sockfd, id, strlen(id));
		fputs("Enter your password: ", stdout);
		fgets(password, sizeof(password), stdin);
		Send(sockfd, password, strlen(password));
		bzero(buffer, sizeof(buffer));
		read(sockfd, buffer, sizeof(buffer));
		if (strncmp(buffer, "SORRY,", 6) == 0)
		{
			fputs(buffer, stdout);
			close(sockfd);
			exit(1);
		}
		else
			fputs(buffer, stdout);
	}
	else //Finally Prompt again for id and password.
		goto start;

	while (TRUE)
	{
		printf("CLIENT: ");
		bzero(buffer, sizeof(buffer));
		fgets(buffer, 1025, stdin);
		upper_string(buffer); //Converting to uppercase.
		Send(sockfd, buffer, strlen(buffer));

		//Check if input is SOUND
		if (strcmp(buffer, "SOUND\n") == 0)
		{
			bzero(buffer, sizeof(buffer));
			Recv(sockfd, buffer, 1024);
			printf("%s\n", buffer);

			//input from stdin
			bzero(buffer, sizeof(buffer));
			fgets(buffer, 1025, stdin);
			upper_string(buffer);
			Send(sockfd, buffer, strlen(buffer));

			bzero(buffer, sizeof(buffer));
			Recv(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
		}

		//Check if input is STORE
		else if (strncmp(buffer, "STORE\n", 6) == 0)
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
			Recv(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
		}

		//Check if input is QUERY
		else if (strcmp(buffer, "QUERY\n") == 0)
		{
			bzero(buffer, sizeof(buffer));
			Recv(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
		}

		//Check if input is BYE OR END
		else if ((strcmp(buffer, "BYE\n")) == 0 || (strcmp(buffer, "END\n")) == 0)
		{
			bzero(buffer, sizeof(buffer));
			Recv(sockfd, buffer, 1024);
			printf("%s\n\n", buffer);
			close(sockfd);
			break;
		}

		//DISPLAYS HELP
		else
		{
			bzero(buffer, sizeof(buffer));
			Recv(sockfd, buffer, 1024);
			printf("%s\n", buffer);
		}
	}

	return 0;
}