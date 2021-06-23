#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 500
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

double euclidean_distances[MAX_LOCATION_QUANTITY];

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void set_arrays() {
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++)
    {
        locations[i].x = -1;
        locations[i].y = -1;    
        euclidean_distances[i] = 9999;        
    }
}

int get_size_locations() {
    int i;    
    int cont = 0;
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x != -1 && locations[i].y != -1){
            cont += 1;
        }
    }

    return cont;
}

int get_index_to_add() {
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x == -1 && locations[i].y == -1){
            return i;
        }
    }

    return -1;
}

int get_index_by_location() {
    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x == x && locations[i].y == y){
            return i;
        }
    }

    return -1;
}

int add_coordenate() {
    int index = get_index_to_add();    
    if(index == -1){
        return -1;
    }

    locations[index].x = x;
    locations[index].y = y;    

    return 0;
}

int remove_coordenate() {
    int index = get_index_by_location();
    if(index == -1) {
        return -1;
    }

    locations[index].x = -1;
    locations[index].y = -1;

    int i;
    for(i = index + 1; i < MAX_LOCATION_QUANTITY; i++) {
        locations[i-1].x = locations[i].x;
        locations[i-1].y = locations[i].y;        
    }

    locations[MAX_LOCATION_QUANTITY - 1].x = -1;
    locations[MAX_LOCATION_QUANTITY - 1].y = -1;

    return 0;
}

int valid_command(char *buf) {
    char * token = strtok(buf, " ");
    int cont = 0;    

    while(token != NULL) {
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
    if(functionWithParameters && (!xValid || !yValid)) {        
        return -1;
    }    

    return 0;
}

void add_location(char *buf) {   
    int index = get_index_by_location();
    if(index != -1) {
        sprintf(buf,"%d %d already exists\n",x,y);
        return;
    } 

    if(add_coordenate() == 0) {
        sprintf(buf,"%d %d added\n",x,y);
    } else {
        sprintf(buf,"limit exceeded\n");
    }
}

void remove_location(char *buf) {
    if(remove_coordenate() == 0) {
        sprintf(buf,"%d %d removed\n",x,y);
    } else {
        sprintf(buf,"%d %d does not exist\n",x,y);        
    }
}

void print_messagem(char* message) {
    int i=0;
    int tamMsg = strlen(message);    
    while(i <= tamMsg) {        
        char c = message[i];        
        printf("i: %d ---- %c = %d\n", i, c, c);
        i++;
    }    
    printf("\n");
}

void list_locations(char *buf) {
    memset(buf, 0, BUFSZ); //Reseting buf for don't sending list name too

    if(get_size_locations() == 0) {
        sprintf(buf,"none\n");
        return;
    } 

    int i;    
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(locations[i].x != -1 && locations[i].y != -1) {
            char location[BUFSZ];
            memset(location, 0, BUFSZ);

            sprintf(location,"%d %d ",locations[i].x,locations[i].y);
            strcat(buf, location);
        }        
    }

    buf[strlen(buf)-1] = 0;

    strcat(buf,"\n");
}

void query(char *buf) {
    double min_distance = 9999;
    int index = -1;
    
    if(get_size_locations() == 0) {
        sprintf(buf,"none\n");        
    }     

    int i;        
    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {      
        if(locations[i].x != -1 && locations[i].y != -1) {  

            double x_value = (double)locations[i].x - x;       
            double y_value = (double)locations[i].y - y;
            
            double distance = sqrt(pow(x_value, 2) + pow(y_value, 2));
            if(distance >= 0){
                euclidean_distances[i] = distance;
            }            
        }
    }

    for(i = 0; i < MAX_LOCATION_QUANTITY; i++) {
        if(euclidean_distances[i] >= 0) {
            if(euclidean_distances[i] < min_distance){
                min_distance = euclidean_distances[i];
                index = i;
            }
        }
    }

    if(index == -1) {
        logexit("query");
    }

    sprintf(buf,"%d %d\n", locations[index].x, locations[index].y);       
}

void select_command(char *buf) {
    if(strcmp(function, OPTION_ADD) == 0) {
        add_location(buf);        
    } else if(strcmp(function, OPTION_REMOVE) == 0) {
        remove_location(buf);
    } else if(strcmp(function, OPTION_QUERY) == 0) {
        query(buf);
    } else if (strcmp(function, OPTION_LIST) == 0) {
        list_locations(buf);
    } else {
        logexit("option");
    }    
}

void format_message(char *buf){
    int i=0;    
    while(i <= strlen(buf)) {        
        if(buf[i] == '\n') {
            buf[i] = 0;
        }        
        i++;
    }    
}

int continue_command(char *buf, int count, int csock) {
    
    format_message(buf);

    printf("[msg recebida] %d bytes: %s\n", (int)count, buf);
    
    if ((int)count > MAX_BYTES) { 
        printf("Desconectado pois ultrapassou os bytes: %d\n", (int)count);
        printf("---------------------------------------------\n\n");
        close(csock); //Terminating program execution if bytes greater than 500
        return -1;
    }            
    
    if (strcmp(buf, "kill") == 0) {                
        printf("Desconectado pois mandou kill\n");
        printf("---------------------------------------------\n\n");
        set_arrays();
        close(csock); //Terminating program execution if client sends kill command 
        return -1;
    }

    if(valid_command(buf) != 0 ) {
        printf("Desconectado pois mandou errado: %s\n", buf);        
        printf("---------------------------------------------\n\n");                
        close(csock); //Terminating program execution if invalid request
        return -1;
    }

    select_command(buf);              

    count = send(csock, buf, strlen(buf), 0);
    if (count != strlen(buf)) {
        logexit("send");
    }

    printf("[msg retornada] %d bytes: %s\n", (int)count, buf);
    printf("---------------------------------------------\n\n");

    return 0;
}

void check_incomplete_message(char *buf, int count, int csock) {
    int hasIncompleteMessage = 0;
    while (hasIncompleteMessage == 0) {
        int i;
        for(i = 0; i < strlen(buf); i++)
        {                    
            if(buf[i] == '\n'){                        
                hasIncompleteMessage = 1;
                break;
            }
        }  

        if(hasIncompleteMessage == 0) {
            printf("[msg incompleta] %d bytes: %s\n", (int)count, buf);
            char buf2[BUFSZ];
            memset(buf2, 0, BUFSZ);
            count += recv(csock, buf2, BUFSZ, 0);
            strcat(buf, buf2);
            strcat(buf,"\n");
            printf("[msg buscada novamente] %d bytes: %s\n", (int)count, buf2);                               
        }
    } 

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

    set_arrays();

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
            size_t count = recv(csock, buf, BUFSZ, 0);

            check_incomplete_message(buf, count, csock);

            if(continue_command(buf, count, csock) != 0) {
                break;
            }            
        }        
    }

    exit(EXIT_SUCCESS);
}