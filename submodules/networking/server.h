#pragma once

#include "networking.h"
#include "../../errors.h"
#include "../../iter.h"

#define DEFAULT_MAX_CONNECTIONS 16

#define PACKET_LIMIT_PER_CONNECTION	16	// maximum amount of packets that could be processed from a single address in a single queue walk

#define REG_REGISTRED	1
#define REG_BANNED		2
#define REG_CONNECTED	4

typedef struct
{
	UID				id;
	SOCKADDR_IN		addr;
	uint8_t			flags;
}
RegistryEntry;

typedef struct
{
	SOCKET 			sock;			// initialized UDP IPv4 socket which is bind to port
	SOCKADDR_IN 	addr;			// server address

	uint16_t		connections;	// current num of connections
	uint16_t 		max_connections;

	Iterable*		tunnels;		// tunnel objects that are still open and have to be resolved
	Iterable*		registry;		// registry entries for incoming connections
}
ServerAPI;

ServerAPI* newServerAPI()
{
	ServerAPI* new = (ServerAPI*)calloc(1, sizeof(ServerAPI));
	new->max_connections = DEFAULT_MAX_CONNECTIONS;
	new->sock 		= newUDPSocket();
	new->tunnel 	= newIter();
	new->registry 	= newIter();

	return new;
}

_Bool initServerAtPort(ServerAPI* server, uint16_t port)
{
	SOCKADDR_IN local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(server->sock, (SOCKADDR*)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
	{
		PRINTLASTWSAERROR("Error on server initialization");
		return false;
	}

	server->addr = local_addr;

	return true;
}
