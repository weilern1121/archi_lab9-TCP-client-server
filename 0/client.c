
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <bits/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "line_parser.h"
#include "common.h"

#define FREE(X) if(X) free((void*)X)


//GLOBALS
client_state *client;
const size_t twoK=2048;
char buffer[2048];

int exit_func_error(char* msg){
    perror(msg);
    return -1;
}




int conn(cmd_line *line){
    //check line
    if(line->arguments[1]==NULL){
        printf("arguments not leggal: %s , %s\n",line->arguments[0],line->arguments[1]);
        exit_func_error("");
    }
    if(client->conn_state!=IDLE)
        exit_func_error("client already connected!");
    struct addrinfo addrinfo, *adress;
    int legal;
    memset(&addrinfo, 0, sizeof addrinfo); //reset the addr
    addrinfo.ai_family = AF_UNSPEC;
    addrinfo.ai_socktype = SOCK_STREAM;

    if((legal=getaddrinfo(line->arguments[1], "2018", &addrinfo, &adress))<0)
        exit_func_error("Illegal address");
    if((legal=socket(AF_INET,SOCK_STREAM,0))<0)
        exit_func_error("Illegal socket");
    int fd=legal;
    if(connect(fd,adress->ai_addr,adress->ai_addrlen)<0)
        exit_func_error("Connection error!");
    //Receive and send data via the new socket created by socket().
    send(fd, "hello", sizeof( "hello"),0);
    client->conn_state=CONNECTING;
    //TODO - probably need to init the buffer
    recv(fd,buffer,twoK,0);
    printf("%s\n",buffer);
    if(strstr(buffer,"hello ")==0){ //first 5 chars are hello+space ->legal input for this func
        //Set client_id to <client_id>.
        client->client_id=malloc(strlen(buffer+6)); //TODO - free in bye
        strcpy(client->client_id,buffer+6);
        //Set conn_state to CONNECTED.
        client->conn_state=CONNECTED;
        //Set sock_fd to the connection's socket.
        client->sock_fd=fd;
        //Set server_addr to <serv_addr>.
        client->server_addr=line->arguments[1];
        return 0;
    }
    else{
        perror("illegal conn command!");
        init_client(client,"nil");
        close(fd);
        return -1;
    }
}

int bye(){
    if(client->conn_state!=CONNECTED)
        exit_func_error("the client is not connected!");
    //send bye
    send(client->sock_fd, "bye", sizeof( "bye"),0);
    //receive bye from server
    recv(client->sock_fd,buffer,twoK,0);
    printf("%s\n",buffer);
    if(buffer[0]!='b' && buffer[1]!='y' && buffer[2]!='e'){
        printf("Server Error: %s\n",buffer+4);
        return -1;
    }
    //else - legal respond from server->disconnect
    FREE(client->client_id); //free the client name (malloced in conn)
    close(client->sock_fd); // close stream
    init_client(client,"nil"); //init the client
    return 0;
}

int exec(cmd_line *line){
    int flag;
    //check input
    if(line->arguments[0]==NULL){
        perror("there are no arguments to execute!");
        exit(EXIT_FAILURE);
    }
    //case conn
    if(strcmp(line->arguments[0],"conn")==0)
        flag=conn(line);
    //case bye
}

void main(){
    int cond=1;
    char* input;
    size_t client_state_size= sizeof(struct client_state); //size of struct client_state for malloc
    client = ( client_state *) (malloc(client_state_size)); //create the client
    init_client(client,"nil");
    while(cond){
        printf("server:%s>",client->server_addr);
        input = fgets(buffer, 2048, stdin);//get the current command line
        //if command=="quit" -> no need to create the line
        if(strstr(input,"quit")==0)
            break;
        //else- exec the line
        cmd_line *line = parse_cmd_lines(input); //parse line
        exec(line);

        free_cmd_lines(line);//TODO- need to be in the end of the while iteration
    }
    FREE(client); //TODO - move to correct place
}

