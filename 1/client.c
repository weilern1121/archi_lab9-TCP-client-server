
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "line_parser.h"
#include "common.h"

#define FREE(X) if(X) free((void*)X)


//GLOBALS
client_state *client;
const size_t twoK=2048;
char buffer[2048];


int conn(cmd_line *line){
    //check line
    if(line->arguments[1]==NULL){
        printf("arguments not legal: %s , %s\n",line->arguments[0],line->arguments[1]);
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
    memset(buffer,0, sizeof(buffer));
    recv(fd,buffer,10,0);
    printf("%s\n",buffer);
    if(strstr(buffer,"hello")){ //first 5 chars are hello+space ->legal input for this func
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

int ls(){
    if(client->conn_state!=CONNECTED){ //can't ls while not connected to server
        return -2;
    }
    send(client->sock_fd, "ls", sizeof( "ls"),0);//send ls to server

    //receive first respond from THE server
    recv(client->sock_fd,buffer,twoK,0);
    //A "nok <error_name>" response message should be
    // printed to stderr as "Server Error: %s" where %s is <error_name>.
    if(strstr(buffer,"nok")){ //if got error from server
        printf("Server Error: %s\n",buffer+4);
        //disconnect user- mission requirement
        FREE(client->client_id); //free the client name (malloced in conn)
        close(client->sock_fd); // close stream
        init_client(client,"nil"); //init the client
        return -1;
    }
    //else ->recive the ls files
    printf("ok\n");
    memset(buffer, 0, sizeof(buffer)); //reset the buffer
    recv(client->sock_fd,buffer,twoK,0);
    printf("%s \n",buffer); //print the ls files
    memset(buffer, 0, sizeof(buffer)); //reset the buffer
    return 0;
}

int exec(cmd_line *line){
    //int flag;
    //check input
    if(line->arguments[0]==NULL){
        /*perror("there are no arguments to execute!");
        exit(EXIT_FAILURE);*/
        exit_func_error("there are no arguments to execute!");
    }
    //case conn
    if(strcmp(line->arguments[0],"conn")==0)
        return (conn(line));
    //case bye
    if(strcmp(line->arguments[0],"bye")==0)
        return (bye());
    //case ls
    if(strcmp(line->arguments[0],"ls")==0)
        return (ls());
    fprintf(stderr,"%s|ERROR unknown message %s\n",client->client_id, line->arguments[0]);
    return -1;
}

int main(){
    int cond=1;
    char* input;
    size_t client_state_size= sizeof(client_state); //size of struct client_state for malloc
    client = ( client_state *) (malloc(client_state_size)); //create the client
    init_client(client,"nil");
    while(cond){
        printf("server: %s >",client->server_addr);
        input = fgets(buffer, 2048, stdin);//get the current command line
        //if command=="quit" -> no need to create the line
        if(strstr(input,"quit"))
            break;
        //else- exec the line
        cmd_line *line = parse_cmd_lines(input); //parse line
        exec(line);

        free_cmd_lines(line);//TODO- need to be in the end of the while iteration
    }
    FREE(client); //TODO - move to correct place
    return 0;
}


