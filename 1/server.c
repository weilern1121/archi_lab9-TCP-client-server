
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "line_parser.h"
#include <netdb.h>
#include <arpa/inet.h>
#include "common.h"
#include <limits.h>
#include <sys/sendfile.h>

#define FREE(X) if(X) free((void*)X)
#define port 2018
#define local_addr "127.0.0.1"
#define MAX_WAIT 10 // how many pending connections queue will hold


//GLOBALS
client_state *client;
size_t twoK=2048;
char buffer[2048];
char user_addr;

void disconnect_client(){
    printf("Client %s disconnected\n",client->client_id);
    //3- server will free any resources allocated for this client
    FREE(client->client_id);
    //4- close streaming
    close(client->sock_fd);
    //5- init the client - there is only one by mission description
    init_client(client,"nil");
}

int hello (int new_fd,cmd_line* command){
    //printf("client->conn_state= %s\n",client->conn_state);
    if(client->conn_state==CONNECTED ){
        send(new_fd,"nok state", sizeof("nok state"),0);
        return -1;
    }
    //else - connect the client ->update the global client
    client->sock_fd=new_fd;
    client->conn_state=CONNECTED;
    //now- prepare the return msg
//    char msg[20];
//    strcpy(msg,command->arguments[0]);
//    msg[5]=32;
//    msg[6]=user_addr+'0';
//    msg[7]='\0';
    char* msg=malloc(10);
    msg[0]='h'; msg[1]='e';msg[2]='l';msg[3]='l';msg[4]='o';msg[5]=' ';
    msg[6]=user_addr;
    msg[7]='\0';
    //sent the msg-respond with "hello %s" where %s is the client_id assigned to the client
    send(new_fd,msg,strlen(msg),0);
    //The server will print to stdout "Client %s connected" where %s is client_id.
    client->client_id=malloc(1); //TODO - free in bye
    //client->client_id=(char*) user_addr+48;
    *(client->client_id)=msg[6];
    printf("Client %s connected\n",client->client_id);

    FREE(msg);
    user_addr++;
    return  0;
}

int bye(){
    if(client->conn_state!=CONNECTED){
        send(client->sock_fd,"nok state", sizeof("nok state"),0);
        return -1;
    }
    //else - disconnect from client:
    //1- the server will respond with "bye" and print to stdout
    //2- "Client %s disconnected" where %s is client_id
    send(client->sock_fd,"bye",strlen("bye"),0);
    disconnect_client();
    return 0;
}

int ls (){
    if(client->conn_state!=CONNECTED){//check connection
        send(client->sock_fd,"nok state", sizeof("nok state"),0);
        disconnect_client();
        return -1;
    }
    char* ls_files=list_dir();//list_dir ->func from common
    if(!ls_files){//if NULL list
        send(client->sock_fd,"nok state", sizeof("nok state"),0); //send nok to client
        return 1;//TODO-return value
    }
    //else- list is ready
    getcwd(buffer,twoK);
    send(client->sock_fd,"ok", sizeof("ok"),0); //send ok to client
    send(client->sock_fd,ls_files, strlen(ls_files),0); //send ls_files to client
    printf(" Listed files at %s\n",buffer);
    return 0;

}

int main(int argc, char *argv[]) {
    int cond=1;
    //char* input;
    user_addr='0';
    //init the handled client
    size_t client_state_size= sizeof(client_state); //size of struct client_state for malloc
    client = ( client_state *) (malloc(client_state_size)); //create the client
    init_client(client,"nil");

    //1- getadderinfo - Get the address information for the server (not client).
    client->server_addr=gethostname();
    //2- socket - Get a file descriptor for the server's socket (its door to the internet).
    struct addrinfo addrinfo, *adress;
    int legal, opt=1,fd,new_fd,flag=0;
    socklen_t addr_size;
    struct sockaddr_storage their_addr;
    memset(&addrinfo, 0, sizeof addrinfo); //reset the addr
    addrinfo.ai_family = AF_UNSPEC;
    addrinfo.ai_socktype = SOCK_STREAM;
    addrinfo.ai_flags=AI_PASSIVE; //fill in my ip for me

    if((legal=getaddrinfo(local_addr, "2018", &addrinfo, &adress))<0)
        exit_func_error("Illegal address");
    if((legal=socket(AF_INET,SOCK_STREAM,0))<0)
        exit_func_error("Illegal socket");
    fd=legal;
    setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&opt, sizeof(int));
    //3- bind - Associate a Layer-4 port with this process.
    if(bind(fd,adress->ai_addr,adress->ai_addrlen)<0) // bind it to the port we passed in to getaddrinfo():
        exit_func_error("bind ERROR!");
    //4- listen - Set the socket into listen mode to allow incoming connections.
    listen(fd,MAX_WAIT);
    while(cond){
        //5- accept - Wait for a new connection and accept it once it arrives.
        //            This socket needs to be closed to end communication with current client.
        addr_size = sizeof their_addr;
        new_fd = accept(fd, (struct sockaddr *)&their_addr, &addr_size);
        while(cond){ //after get a client - handle only him
            flag=0;
            //6- recv and send - Receive and send data via the new socket created by accept().
            //TODO -
            char buffer[twoK];
            recv(new_fd,buffer,twoK,0);
            cmd_line* command=parse_cmd_lines(buffer);

            if(strstr(command->arguments[0],"hello")){
                flag=1;
                hello(new_fd,command);
            }
            if(strstr(command->arguments[0],"bye")){
                flag=2;
                bye();
            }
            if(strstr(command->arguments[0],"ls")){
                flag=1;
                if(ls(new_fd,command)<0)
                    flag=2;
            }
            if(flag==0)
                fprintf(stderr,"%s|ERROR unknown message %s",client->client_id, buffer);
            free_cmd_lines(command);
            if(flag==2)//if bye->go get new client (exit 1 while)
                break;
        }



    }

}
