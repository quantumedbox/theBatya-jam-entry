#pragma once

#include "networking.h"
#include "../../errors.h"

#define MAX_REGISTRATION_REQUESTS 5		// how many reg packets will be send before giving up

typedef struct
{
	SOCKET 			sock;	// initialized UDP IPv4 socket
	SOCKADDR_IN 	addr;	// server address
}
ClientAPI;

ClientAPI* newClientAPI()
{
	ClientAPI* new = (ClientAPI*)calloc(1, sizeof(ClientAPI));	// zeroed memory
	new->sock = newUDPSocket();

	return new;
}

// Returns false on error
_Bool connectToServer(ClientAPI* client, char* address, uint16_t port)//, uint32_t login_data)
{
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET6;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(address);

	if (server_addr.sin_addr.s_addr == INADDR_NONE)
	{
		PRINTLASTWSAERROR("Server address isn't valid");
		return false;
	}

	client->addr = server_addr;

	return true;
}
