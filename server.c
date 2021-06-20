#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MIN_VALUE_VALID_COORDINATE 0
#define MAX_VALUE_VALID_COORDINATE 9999
#define MAX_LOCATION_QUANTITY 50
#define OPTION_ADD "add"
#define OPTION_REMOVE "rm"
#define OPTION_LIST "list"
#define OPTION_QUERY "query"
#define ERROR_RETURN "Method not found"

char function[5];
int x, y;

struct location
{
    int x;
    int y;    
};

struct location locations[MAX_LOCATION_QUANTITY];

int euclideanDistances[MAX_LOCATION_QUANTITY];

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void setArrays()
{
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        locations[i].x = -1;
        locations[i].y = -1;    
        euclideanDistances[i] = 9999;        
    }
}

int countLocation()
{
    int i;    
    int cont = 0;
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        if(locations[i].x != -1 && locations[i].y != -1){
            cont += 1;
        }
    }

    return cont;
}

int getIndexToAdd()
{
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        if(locations[i].x == -1 && locations[i].y == -1){
            return i;
        }
    }

    return -1;
}

int getIndexByLocation()
{
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        if(locations[i].x == x && locations[i].y == y){
            return i;
        }
    }

    return -1;
}

void addLocation()
{
    int index = getIndexToAdd();    
    if(index == -1){
        logexit("addLocation");
    }

    locations[index].x = x;
    locations[index].y = y;    
}

int removeLocation()
{
    int index = getIndexByLocation();
    if(index == -1){
        return 0;
    }

    locations[index].x = -1;
    locations[index].y = -1;    

    return 1;
}

char * listLocation()
{
    char *response = malloc(BUFSZ);
    memset(response, 0, BUFSZ);

    if(countLocation() == 0){
        sprintf(response,"none");
        return response;
    } 

    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x != -1 && locations[i].y != -1){
            char *locationStr = malloc(BUFSZ);
            memset(locationStr, 0, BUFSZ);

            sprintf(locationStr,"%d %d ",locations[i].x,locations[i].y);
            strcat(response, locationStr);
        }        
    }

    return response;
}

int menu(char *buf)
{
    char * token = strtok(buf, " ");
    int cont = 0;    

    while( token != NULL ) {        
        if(cont == 0) {
            strcpy(function, token);
        } else if(cont == 1) {
            x = atoi(token);
        } else if(cont == 2) {        
            y = atoi(token);
        }
        cont += 1;
        token = strtok(NULL, " ");    
    }

    int optionAdd = strcmp(function, OPTION_ADD) == 0;
    int optionRm = strcmp(function, OPTION_REMOVE) == 0;
    int optionQuery = strcmp(function, OPTION_QUERY) == 0;
    int optionList = strcmp(function, OPTION_LIST) == 0;    

    int functionWithParameters = (optionAdd || optionRm || optionQuery) && cont == 3;
    int functionWithoutParameters = optionList && cont == 1;    
    if(!functionWithParameters && !functionWithoutParameters) {             
        return -1;        
    }

    int xValid = x >= MIN_VALUE_VALID_COORDINATE && x <= MAX_VALUE_VALID_COORDINATE;
    int yValid = y >= MIN_VALUE_VALID_COORDINATE && y <= MAX_VALUE_VALID_COORDINATE;
    if(functionWithParameters && (!xValid || !yValid)){        
        return -2;
    }    

    return 1;
}

char * add()
{   
    char *response = malloc(BUFSZ);
    memset(response, 0, BUFSZ);

    int index = getIndexByLocation();
    if(index != -1){
        sprintf(response,"%d %d already exists",x,y);
        return response;
    } 

    addLocation();    
    
    sprintf(response,"%d %d added",x,y);    
    return response;
}

char * removeCoordenate()
{   
    char *response = malloc(BUFSZ);
    memset(response, 0, BUFSZ);

    int removeLocationResponse = removeLocation();    
    if(removeLocationResponse){
        sprintf(response,"%d %d removed",x,y);
    } else {
        sprintf(response,"%d %d does not exist",x,y);        
    }
    
    return response;
}

char * list()
{   
    char *response = malloc(BUFSZ);
    memset(response, 0, BUFSZ);    
    sprintf(response, "%s", listLocation());       
    return response;
}

char * query()
{ 
    char *response = malloc(BUFSZ);
    memset(response, 0, BUFSZ);
    int min_distance = 9999;
    int index = -1;
    
    if(countLocation() == 0){
        sprintf(response,"none");
        return response;
    }     

    int i;        
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {      
        if(locations[i].x != x && locations[i].x != -1 && locations[i].y != y && locations[i].y != -1){
            int distance = sqrt(((locations[i].x - x) ^ 2) + ((locations[i].y - y) ^ 2));            
            if(distance > 0){
                euclideanDistances[i] = distance;
            }            
        }
    }   

    for(i = 0; i < MAX_LOCATION_QUANTITY; i++){
        if(euclideanDistances[i] > 0){
            if(euclideanDistances[i] < min_distance){
                min_distance = euclideanDistances[i];
                index = i;
            }
        }
    }

    if(index == -1){
        logexit("query");
    }

    sprintf(response,"%d %d", locations[index].x, locations[index].y);   

    return response;
}

char * option()
{
    int optionAdd = strcmp(function, OPTION_ADD) == 0;
    int optionRm = strcmp(function, OPTION_REMOVE) == 0;
    int optionQuery = strcmp(function, OPTION_QUERY) == 0;
    int optionList = strcmp(function, OPTION_LIST) == 0;
    if(optionAdd) {
        return add();        
    } else if(optionRm) {
        return removeCoordenate();
    } else if(optionQuery) {
        return query();
    } else if (optionList) {
        return list();
    }

    return ERROR_RETURN;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }    

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }   

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }    

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }   

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    } 

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    setArrays();

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);        

        int csock = accept(s, caddr, &caddrlen);        
        if (csock == -1) {
            logexit("accept");
        }        

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);        
         
        while(1)
        {
            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            size_t count = recv(csock, buf, BUFSZ - 1, 0);            

            //Terminating program execution if client sends kill command 
            if (strcmp(buf, "kill") == 0) {                
                close(csock);
                break;
            }

            char response[BUFSZ];
            memset(response, 0, BUFSZ);

            int responseMenu = menu(buf);                     
            if (responseMenu == 1) {
                char *responseOption = option();
                strcpy(response, responseOption);
                free(responseOption);
            } else if (responseMenu == -1){                
                strcpy(response, "\nerror:\nusage <function> <x coordinate> <y coordinate>\nexample: add 111 222\n\n");
            } else if (responseMenu == -2){
                strcpy(response, "\nerror:\nminimum value of valid coordinate: 0\nmaximum value of valid coordinate: 9999\nexample: add 4444 6666\n\n");
            } else {
                logexit("menu");
            }            
            
            sprintf(buf, "%s", response);

            count = send(csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logexit("send");
            }
        }        
    }

    exit(EXIT_SUCCESS);
}