#pragma once

#include "networking.h"
#include "../../errors.h"

#define MAX_REGISTRATION_REQUESTS 10	// how many reg packets will be send before giving up

typedef struct
{
	SOCKET 			sock;				// initialized UDP IPv4 socket
	SOCKADDR_IN 	addr;				// server address

	UID 			id;
}
ClientAPI;


ClientAPI* 		newClientAPI		();
	 void 		clientSendPacket	(ClientAPI*);
	_Bool 		clientConnect		(ClientAPI*, uint32_t address, 	uint16_t port);
	_Bool 		clientWaitForPacket	(ClientAPI*, PacketType_T, 		uint32_t ms);


ClientAPI* newClientAPI()
{
	ClientAPI* new = (ClientAPI*)calloc(1, sizeof(ClientAPI));	// zeroed memory
	new->sock = newUDPSocket();

	return new;
}

// Returns false on error or when server isn't responding
_Bool clientConnect(ClientAPI* client, uint32_t address, uint16_t port)//, uint32_t login_data)
{
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = address;

	if (server_addr.sin_addr.s_addr == INADDR_NONE)
	{
		PRINTLASTWSAERROR("Server address isn't valid");
		return false;
	}

	client->addr = server_addr;

	newPacket(REQUEST_REGISTRY);
	addPacketData((char*)"DING DONG, THE WICKED BITCH IS DEAD");

	for (int i = MAX_REGISTRATION_REQUESTS; i--;) {

		clientSendPacket(client);

		if(clientWaitForPacket(client, REGISTRY_ACCEPTED, 1000))
		{
			printf("registred with an ID of %llu\n", client->id);
			return true;
		}
		else
			printf("server isn't responding, next try...\n");
	}

	return false;
}

void clientSendPacket(ClientAPI* client)
{
	if (!socketCheckForWritability(client->sock, 1000))	// TODO automatic retry if needed
	{
		printf("Socket isn't available for send\n");
	}
	else if (sendto(client->sock, PACKET_BUFFER, PACKET_BUFFER_SIZE, NO_FLAGS, (SOCKADDR*)&client->addr, sizeof(client->addr)) == SOCKET_ERROR)
		{
			#ifdef STRICT_RUNTIME
			EXITLASTWSAERROR("sendto function failed");
			#else
			PRINTLASTWSAERROR("sendto function failed");
			#endif
		}
}

// wait for a particular, discarding everything else
// returns true when packet is recieved
_Bool clientWaitForPacket(ClientAPI* client, PacketType_T target, uint32_t delay)
{
	_Bool return_value = false;
	uint64_t startTime = getSeconds();

	if (!socketCheckForReadability(client->sock, 1000))
	{
		printf("Socket isn't available for read\n");
	}
	else
	{
		socketSetRecvTimeout(client->sock, delay);

		while (!return_value && (getSeconds() - startTime < delay))
		{
			// Should we check to which address incoming packet is belonging? it might be not a server
			char buffer[PACKET_MAX_SIZE] = {'\0'};
			socketPeek(client->sock, buffer);
			if (buffer[0] == target)
			{	// Registy data recieved
				client->id = *(UID*)&buffer[1];
				return_value = true;
			}
			socketPopInputQueue(client->sock);
		}

		// Don't forget to set delay to 0 afterwards
		socketSetRecvTimeout(client->sock, 0);
	}

	return return_value;
}
