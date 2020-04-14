// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

struct animal
{
    char name[15];
    char sound[15];
};

struct database
{
    char userid[1024];
    char password[1024];
};

void error(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

//wraper func for send()
int Write(int sockfd, char *buffer, int size, int *clifds, int i)
{
    int n;
    n = write(sockfd, buffer, size);
    if (n <= 0)
    {
        fprintf(stderr, "Client terminated prematurely.\n");
        close(sockfd);
        clifds[i] = 0;
    }
    else
    {
        return n;
    }
}

int Read(int sockfd, char *buffer, int size, int *clifds, int i)
{
    int n;
    n = read(sockfd, buffer, size);
    if (n <= 0)
    {
        fprintf(stderr, "Client terminated prematurely.\n");
        close(sockfd);
        clifds[i] = 0;
    }
    else
    {
        return n;
    }
}

int Send(int sockfd, char *arr, int size)
{
    int n = write(sockfd, arr, size);

    if (n < 1)
    {
        close(sockfd);
        exit(1);
    }
    else
        return n;
}
int Recv(int sockfd, char *arr, int size)
{
    int n = read(sockfd, arr, size);

    if (n < 1)
    {
        close(sockfd);
        exit(1);
    }
    else
        return n;
}

int login(struct database *clientData, char *id, char *password)
{
    for (int j = 0; j < 30; j++)
    {
        if (strcmp(clientData[j].userid, id) == 0)
        {
            if (strcmp(clientData[j].password, password) == 0)
            {
                return 1;
            }
        }
    }

    return -1;
}

int createUser(struct database *clientData, char *id, char *password)
{
    for (int j = 0; j < 30; j++)
    {
        if (strlen(clientData[j].userid) == 0)
        {
            strcpy(clientData[j].userid, id);
            strcpy(clientData[j].password, password);
            return 1;
        }
    }

    return -1;
}

void upper_string(char s[])
{
    int c = 0;
    while (s[c] != '\0')
    {
        if (s[c] >= 'a' && s[c] <= 'z')
        {
            s[c] = s[c] - 32;
        }
        c++;
    }
}