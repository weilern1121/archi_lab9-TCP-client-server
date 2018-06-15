
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <bits/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "line_parser.h"
#include "common.h"

#define FREE(X) if(X) free((void*)X)
#define port 2018


//GLOBALS
client_state *client;
const size_t twoK=2048;
char buffer[2048];
int main(int argc, char *argv[]) {
    int cond=1;
    char* input;
    size_t client_state_size= sizeof(struct client_state); //size of struct client_state for malloc
    client = ( client_state *) (malloc(client_state_size)); //create the client
    init_client(client,"nil");
    client->server_addr=gethostname();
}

