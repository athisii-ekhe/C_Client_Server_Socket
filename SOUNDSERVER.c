#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include "helper.h" //my functons and data structures



int main(int argc, char *argv[])
{

   int sockfd, confd, portno, n;
   struct sockaddr_in address;
   int opt = 1;
   socklen_t addrlen;
   char id[1024], password[1024], buffer[1024], temp[1024],

       *info = "SERVER: WELCOME TO SOUNDSERVER\n1. Register \n2. Login\nEnter your choice: ",
       *id_prompt = "SERVER: ENTER YOUR ID",
       *password_prompt = "SERVER: ENTER YOUR PASSWORD",
       *login_err_msg = "SORRY, ID AND PASSWORD IS NOT CORRECT.\n",
       *not_found = "SERVER: I DON'T KNOW ",
       *help = "SERVER: SUPPORTED COMMANDS\n 1. SOUND   - RETURNS SOUND OF THE ENTERED ANIMAL.\n"
               " 2. STORE     - STORES ANIMAL WITH ITS SOUND.\n"
               " 3. QUERY     - DISPLAYS ALL STORED ANIMALS.\n"
               " 4. BYE       - TERMINATES THE CURRENT SESSION.\n"
               " 5. END       - SHUTDOWN THE SERVER.\n";

   int nready, valread, i, max_sd, maxi, max_clients = 30,
                                         clifds[max_clients], sd, tot_cli = 0;

   fd_set readfds;

   if (argc < 2)
   {
      fprintf(stderr, "Error, port number not provided.\n");
      exit(EXIT_FAILURE);
   }

   for (i = 0; i < max_clients; i++)
   {
      clifds[i] = 0;
   }

   struct database clientData[30];
   bzero((char *)clientData, sizeof(clientData));
   struct animal list[15];
   bzero((char *)list, sizeof(list));

   char names[5][15] = {"CAT", "DOG", "HORSE", "SNAKE", "COW"};
   char sounds[5][15] = {"MEOW", "WOOF", "NEIGH", "HISS", "BOO"};

   for (int i = 0; i < 5; i++)
   {
      strcpy(list[i].name, names[i]);
      strcpy(list[i].sound, sounds[i]);
   }

   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
   {
      error("Error creating socket");
   }
   //set master socket to allow multiple connections ,
   //this is just a good habit, it will work without this
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
   {
      error("Error on sockopt");
   }

   portno = atoi(argv[1]);

   bzero((char *)&address, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_port = htons(portno);
   address.sin_addr.s_addr = INADDR_ANY;

   if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
   {
      error("Error on bind.");
   }
   if (listen(sockfd, 3) < 0)
   {
      error("Error on listen");
   }

   addrlen = sizeof(address);
   printf("Waiting for connections ...\n");

   while (1)
   {
   first:
      printf("\nUSER ACCOUNTS IN DATABASE\n   USERID\tPASWORD\n");
      for (int count = 0; count < 30; count++)
      {
         if (strlen(clientData[count].userid) == 0)
            break;
         printf("%d. %s\t********** \n", count + 1, clientData[count].userid);
      }
      printf("\n#ACTIVE USER(S): %d\n", tot_cli);
      FD_ZERO(&readfds);        //clear the socket set
      FD_SET(sockfd, &readfds); //add master socket to set
      max_sd = sockfd;
      maxi = 0;
      //add child sockets to set
      for (i = 0; i < max_clients; i++)
      {
         sd = clifds[i];

         //if valid socket descriptor then add to read list
         if (sd > 0)
         {
            FD_SET(sd, &readfds);
            maxi = i;
         }

         //highest file descriptor number, need it for the select function
         if (sd > max_sd)
         {
            max_sd = sd;
         }
      }

      //wait for an activity on one of the sockets , timeout is NULL ,
      //so wait indefinitely
      nready = select(max_sd + 1, &readfds, NULL, NULL, NULL);

      if ((nready < 0) && (errno != EINTR))
      {
         printf("Error on select");
      }

      //If something happened on the master socket ,
      //then its an incoming connection
      if (FD_ISSET(sockfd, &readfds))
      {
         if ((confd = accept(sockfd,
                             (struct sockaddr *)&address, &addrlen)) < 0)
         {
            error("Error on accept.");
         }

      start:
         //Prompts options for registration and login
         write(confd, info, strlen(info));
         bzero(buffer, sizeof(buffer));
         read(confd, buffer, sizeof(buffer));

         if (strncmp(buffer, "1\n", 2) == 0)
         {
            //Receives id and password for Registration
            bzero(id, sizeof(id));
            n = read(confd, id, sizeof(id));
            id[n - 1] = '\0'; //Removes \n
            //Receives password
            bzero(password, sizeof(password));
            n = read(confd, password, sizeof(password));
            password[n - 1] = '\0';

            n = createUser(clientData, id, password); //Creates user account
            if (n == 1)
            {
               bzero(temp, sizeof(temp));
               strcat(temp, "SERVER: Hi, ");
               strcat(temp, id);
               strcat(temp, "!\n");
               fputs("New User successfully added\n", stdout);
               write(confd, temp, strlen(temp));
            }
         }
         else if (strncmp(buffer, "2\n", 2) == 0)
         {
            //Receives id for Login
            bzero(id, sizeof(id));
            n = read(confd, id, sizeof(id));
            id[n - 1] = '\0';
            //Receives password
            bzero(password, sizeof(password));
            n = read(confd, password, sizeof(password));
            password[n - 1] = '\0';

            n = login(clientData, id, password); //Checks user id and password
            if (n == 1)
            {

               bzero(temp, sizeof(temp));
               strcat(temp, "SERVER: WELCOME BACK, "); // DISPLAYS WELCOMING MESSAGE ON CLIENT
               strcat(temp, id);
               strcat(temp, "!\n");          // APPENDS '\n'
               fputs("NEW LOGIN\n", stdout); // Displays on SERVER
               write(confd, temp, strlen(temp));
            }
            else
            {
               fputs("WRONG ID\n", stdout);                        // Displays on SERVER(TESTING)
               write(confd, login_err_msg, strlen(login_err_msg)); // SENDS to CLIENT
               close(confd);
               goto first;
            }
         }
         else
         {
            goto start;
         }

         printf("%s connected\n", inet_ntoa(address.sin_addr));
         tot_cli++; //INCREMENTS #CLIENT

         for (i = 0; i < max_clients; i++)
         {
            if (clifds[i] == 0)
            {
               clifds[i] = confd;
               break;
            }
         }
      }

      //else its some IO operation on some other socket
      else
      {
         for (i = 0; i <= maxi; i++)
         {
            if ((sd = clifds[i]) < 1)
               continue;

            if (FD_ISSET(sd, &readfds))
            {
               //Check if it was for closing , and also read the
               //incoming message
               if ((valread = read(sd, buffer, 1024)) == 0)
               {
                  //Somebody disconnected , get his details and print
                  getpeername(sd, (struct sockaddr *)&address, &addrlen);
                  printf("%s disconnected.\n", inet_ntoa(address.sin_addr));
                  tot_cli--;
                  //Close the socket and mark as 0 in list for reuse
                  close(sd);
                  clifds[i] = 0;
               }

               else
               {
                  //Checks if received string is SOUND
                  if (strncmp(buffer, "SOUND\n", 6) == 0)
                  {
                     Write(sd, "SERVER: SOUND: OK", 18, clifds, i);
                     bzero(buffer, sizeof(buffer));
                     int n = Read(sd, buffer, 1024, clifds, i);
                     buffer[n - 1] = '\0'; //replacing \n by null character.
                     bzero(temp, sizeof(temp));
                     for (int j = 0; j < 15; j++)
                     {
                        if (strcmp(list[j].name, buffer) == 0)
                        {
                           strcat(temp, "SERVER: A ");
                           strcat(temp, list[j].name);
                           strcat(temp, " SAYS ");
                           strcat(temp, list[j].sound);
                           Write(sd, temp, strlen(temp), clifds, i);
                           break;
                        }
                        if (strlen(list[j].name) == 0)
                        {
                           char temp[1025] = {};
                           strcpy(temp, not_found);
                           strcat(temp, buffer);
                           Write(sd, temp, strlen(temp), clifds, i);
                           break;
                        }
                     }
                  }

                  //Checks if received string is STORE
                  else if (strncmp(buffer, "STORE\n", 6) == 0)
                  {
                     bzero(buffer, sizeof(buffer));
                     int n = Read(sd, buffer, 1024, clifds, i);
                     buffer[n - 1] = '\0'; //replacing \n by null character.
                     for (int j = 0; j < 15; j++)
                     {
                        if (strcmp(list[j].name, buffer) == 0)
                        {
                           bzero(buffer, sizeof(buffer));
                           int n = Read(sd, buffer, 1024, clifds, i);
                           buffer[n - 1] = '\0'; //replacing \n by null character.
                           strncpy(list[j].sound, buffer, n);
                           goto done;
                        }

                        if (strlen(list[j].name) == 0)
                        {
                           strncpy(list[j].name, buffer, n);
                           bzero(buffer, sizeof(buffer));
                           int n = Read(sd, buffer, 1024, clifds, i);
                           buffer[n - 1] = '\0'; //replacing \n by null character.
                           strncpy(list[j].sound, buffer, n);
                           goto done;
                        }
                     }

                     Read(sd, buffer, 1024, clifds, i); // dummy read, if storage is full i.e exceeds 15 animals

                  done:
                     Write(sd, "SERVER: STORE: OK", 18, clifds, i);
                  }

                  //Checks if received string is QUERY
                  else if (strncmp(buffer, "QUERY\n", 6) == 0)
                  {
                     bzero(temp, sizeof(temp));
                     strcat(temp, "SERVER: ");
                     int count = 0;
                     while (strlen(list[count].name) != 0 && count < 15)
                     {
                        strcat(temp, list[count].name);
                        strcat(temp, "\n");
                        count++;
                     }
                     strcat(temp, "QUERY: OK");
                     Write(sd, temp, strlen(temp), clifds, i);
                  }

                  //Checks if received string is BYE
                  else if (strncmp(buffer, "BYE\n", 4) == 0)
                  {
                     getpeername(sd, (struct sockaddr *)&address, &addrlen);
                     printf("%s disconnected.\n", inet_ntoa(address.sin_addr));
                     write(sd, "SERVER: BYE: OK", 16);
                     close(sd);
                     tot_cli--;
                     clifds[i] = 0;
                  }

                  //Checks if received string is END
                  else if (strncmp(buffer, "END\n", 4) == 0)
                  {
                     write(sd, "SERVER: END: OK", 16);
                     for (i = 0; i < max_clients; i++)
                     {
                        sd = clifds[i];
                        if (sd > 0)
                        {
                           close(sd);
                        }
                     }
                     close(sockfd);
                     printf("Server terminated prematurely.\n");
                     exit(1);
                  }

                  //ELSE DISPLAY LIST OF SUPPORTED COMMANDS
                  else
                  {

                     write(sd, help, strlen(help));
                  }
               }
               if (--nready <= 0)
                  break;
            }
         }
      }
   }
   return 0;
}