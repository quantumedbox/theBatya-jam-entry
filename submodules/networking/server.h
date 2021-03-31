#pragma once

#include "networking.h"
#include "../../errors.h"
#include "../../iter.h"

#define DEFAULT_MAX_CONNECTIONS 16

#define REG_REGISTRED	1
#define REG_BANNED		2
#define REG_CONNECTED	4

#define SERVER_WRITABILITY_TIMEO 100

typedef struct
{
	UID				id;
	uint32_t		ip_addr;		// do not care about port as they are constantly changing
	uint8_t			flags;
}
RegistryEntry;

typedef struct
{
	SOCKET 			listening_sock;	// initialized UDP IPv4 socket which is binded to port
	SOCKADDR_IN 	listening_addr;	// server listeting address port

	SOCKET 			answering_sock;	// initialized UDP IPv4 socket which is binded to port
	SOCKADDR_IN 	answering_addr;	// server answering address port

	uint16_t		connections;	// current num of connections
	uint16_t 		max_connections;

	Iterable*		tunnels;		// tunnel objects that are still open and have to be resolved
	Iterable*		registry;		// registry entries for incoming connections
}
ServerAPI;

ServerAPI* 	newServerAPI();
	_Bool 	initServer(ServerAPI*, uint32_t ip, uint16_t listening_port, uint16_t answering_port);
	 void 	printServerInfo(ServerAPI*);

SOCKADDR_IN bindSocketToPort(SOCKET, uint32_t ip, uint16_t port);

	_Bool 	serverSendPacketToAddr(ServerAPI*, SOCKADDR_IN addr);


ServerAPI* newServerAPI()
{
	ServerAPI* new = (ServerAPI*)calloc(1, sizeof(ServerAPI));
	new->max_connections = DEFAULT_MAX_CONNECTIONS;
	new->listening_sock	 = newUDPSocket();
	new->answering_sock	 = newUDPSocket();
	new->tunnels 		 = newIter();
	new->registry 		 = newIter();

	return new;
}

SOCKADDR_IN bindSocketToPort(SOCKET sock, uint32_t ip, uint16_t port)
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip;

	if(bind(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		EXITLASTWSAERROR("Error on initialization of socket");
	}

	return addr;
}

_Bool initServer(ServerAPI* server, uint32_t ip, uint16_t listening_port, uint16_t answering_port)
{
	server->listening_addr = bindSocketToPort(server->listening_sock, ip, listening_port);
	server->answering_addr = bindSocketToPort(server->answering_sock, ip, answering_port);

	char buffer[PACKET_MAX_SIZE] = {'\0'};
	SOCKADDR_IN addr_from;
	int addr_from_size = sizeof(addr_from);
	recvfrom(
		server->listening_sock, buffer,
		PACKET_MAX_SIZE, 0,
		(SOCKADDR*)&addr_from, &addr_from_size
	);

	newPacket(REGISTRY_ACCEPTED);
	addPacketData((UID)2525252);
	serverSendPacketToAddr(server, addr_from);

	return true;
}

_Bool serverSendPacketToAddr(ServerAPI* server, SOCKADDR_IN addr)
{
	if (!socketCheckForWritability(server->answering_sock, SERVER_WRITABILITY_TIMEO)) {
		printf("Answer socket isn't available for send\n");
		return false;
	}
	if (sendto(server->answering_sock, PACKET_BUFFER, PACKET_BUFFER_SIZE, NO_FLAGS, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		#ifdef STRICT_RUNTIME
		EXITLASTWSAERROR("sendto function failed");
		#else
		PRINTLASTWSAERROR("sendto function failed");
		#endif

		return false;
	}
	else {
		return true;
	}
}

void printServerInfo(ServerAPI* server)
{
	printf(":SERVER INFO:\n");

	printf("\tconnected: %d/%d\n", server->connections, server->max_connections);
	printf("\tregistry entries: %d\n", server->registry->len);
	printf("\topened tunnels: %d\n", server->tunnels->len);

	printf(
		"\tlisteting addr: %d.%d.%d.%d:%d\n",
		server->listening_addr.sin_addr.S_un.S_un_b.s_b1,
		server->listening_addr.sin_addr.S_un.S_un_b.s_b2,
		server->listening_addr.sin_addr.S_un.S_un_b.s_b3,
		server->listening_addr.sin_addr.S_un.S_un_b.s_b4,
		htons(server->listening_addr.sin_port)
	);
	printf(
		"\tanswering addr: %d.%d.%d.%d:%d\n",
		server->answering_addr.sin_addr.S_un.S_un_b.s_b1,
		server->answering_addr.sin_addr.S_un.S_un_b.s_b2,
		server->answering_addr.sin_addr.S_un.S_un_b.s_b3,
		server->answering_addr.sin_addr.S_un.S_un_b.s_b4,
		htons(server->answering_addr.sin_port)
	);
}
