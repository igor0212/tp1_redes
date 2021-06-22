#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MIN_VALUE_VALID_COORDINATE 0
#define MAX_VALUE_VALID_COORDINATE 9999
#define MAX_LOCATION_QUANTITY 50
#define MAX_BYTES 500
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

void setArrays(){
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        locations[i].x = -1;
        locations[i].y = -1;    
        euclideanDistances[i] = 9999;        
    }
}

int countLocation(){
    int i;    
    int cont = 0;
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++){
        if(locations[i].x != -1 && locations[i].y != -1){
            cont += 1;
        }
    }

    return cont;
}

int getIndexToAdd(){
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        if(locations[i].x == -1 && locations[i].y == -1){
            return i;
        }
    }

    return -1;
}

int getIndexByLocation(){
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        if(locations[i].x == x && locations[i].y == y){
            return i;
        }
    }

    return -1;
}

void addLocation(){
    int index = getIndexToAdd();    
    if(index == -1){
        logexit("addLocation");
    }

    locations[index].x = x;
    locations[index].y = y;    
}

int removeLocation(){
    int index = getIndexByLocation();
    if(index == -1){
        return -1;
    }

    locations[index].x = -1;
    locations[index].y = -1;    

    return 0;
}

int menu(char *buf){
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
        return -1;
    }    

    return 0;
}

void add(char *buf){   
    int index = getIndexByLocation();
    if(index != -1){
        sprintf(buf,"%d %d already exists\n",x,y);
        return;
    } 

    addLocation();    
    
    sprintf(buf,"%d %d added\n",x,y);        
}

void removeCoordenate(char *buf){   
    int response = removeLocation();    
    if(response == 0){
        sprintf(buf,"%d %d removed\n",x,y);
    } else {
        sprintf(buf,"%d %d does not exist\n",x,y);        
    }
}

void list(char *buf){

    memset(buf, 0, BUFSZ); //Reseting buf for don't sending list name too

    if(countLocation() == 0){
        sprintf(buf,"none\n");
        return;
    } 

    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x != -1 && locations[i].y != -1){
            char *location = malloc(BUFSZ);
            memset(location, 0, BUFSZ);            

            sprintf(location,"%d %d ",locations[i].x,locations[i].y);
            strcat(buf, location);
        }        
    }  

    strcat(buf,"\n");  
}

void query(char *buf){     
    int min_distance = 9999;
    int index = -1;
    
    if(countLocation() == 0){
        sprintf(buf,"none\n");        
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

    sprintf(buf,"%d %d\n", locations[index].x, locations[index].y);       
}

void option(char *buf){
    if(strcmp(function, OPTION_ADD) == 0) {
        add(buf);        
    } else if(strcmp(function, OPTION_REMOVE) == 0) {
        removeCoordenate(buf);
    } else if(strcmp(function, OPTION_QUERY) == 0) {
        query(buf);
    } else if (strcmp(function, OPTION_LIST) == 0) {
        list(buf);
    } else {
        logexit("option");
    }    
}

void print_messagem(char* message){
    int i=0;
    int tamMsg = strlen(message);    
    while(i <= tamMsg){        
        char c = message[i];        
        printf("i: %d ---- %c = %d\n", i, c, c);
        i++;
    }    
    printf("\n");
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
         
        while(1) {

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);
            size_t count = recv(csock, buf, BUFSZ - 1, 0);   
            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);             

            buf[strlen(buf)-1] = 0;
            //print_messagem(buf);            
            
            if ((int)count > MAX_BYTES) { 
                printf("Desconectado pois ultrapassou os bytes: %d\n", (int)count);
                close(csock); //Terminating program execution if bytes greater than 500
                break;
            }                     
            
            if (strcmp(buf, "kill") == 0) {                
                close(csock); //Terminating program execution if client sends kill command 
                break;
            }

            if(menu(buf) != 0 ) {
                printf("Desconectado pois mandou errado:\n");
                print_messagem(buf);
                close(csock); //Terminating program execution if invalid request
                break;
            }

            option(buf);            

            count = send(csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logexit("send");
            }
        }        
    }

    exit(EXIT_SUCCESS);
}