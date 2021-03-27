#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <winsock2.h>

#include "errors.h"

#ifndef PACKET_BUFFER_MAX_SIZE
#	define PACKET_BUFFER_MAX_SIZE 1500	// in bytes
#endif

#define GETWSAERROR(description) WSAERROR(description, WSAGetLastError())


typedef struct
{
	SOCKET sock;
	SOCKADDR_IN local_addr;
}
HostSocket;

typedef struct
{
	SOCKET sock;
	SOCKADDR_IN server_addr;
}
ClientSocket;

typedef enum PacketType {
	// Special packet that tells to the other side that packet was recieved (UDP safety)
	PACKET_RECIEVED,

	// Client-side
	REQUEST_ID,
	REQUEST_GAMESTATE,
	CLIENT_DISCONECT,

	// Server-side
	SEND_ID,
	SEND_GAMESTATE,
	SERVER_CLOSED_CONNECTION,
}
PacketType_T;


__forceinline void newPacket(PacketType_T packet_type);
__forceinline void copyPacketData(char* dest);

__forceinline void addPacketDataINT8(int8_t data);
__forceinline void addPacketDataUINT8(uint8_t data);
__forceinline void addPacketDataINT16(int16_t data);
__forceinline void addPacketDataUINT16(uint16_t data);
__forceinline void addPacketDataINT32(int32_t data);
__forceinline void addPacketDataUINT32(uint32_t data);
__forceinline void addPacketDataINT64(int64_t data);
__forceinline void addPacketDataUINT64(uint64_t data);


SOCKET initSocket(int address_family, int type, int protocol)
{
	SOCKET sock = socket(address_family, type, protocol);
	if (sock == INVALID_SOCKET)
		GETWSAERROR("Error on socket creation\n");

	return sock;
}

__forceinline SOCKET initUDPSocket()
{
	return initSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

// ПРИНЦИП:
// в основном лупе non-blocking recvfrom сначала пытается получить данные в нулевой буффер с MSG_PEEK флагом
// это делается для того, чтобы получить IP и порт соединения, дабы понять, является ли адрес "зарегистрированным"
// если адрес актуален - вызывается новый recvfrom на этот раз списывающий данные пакета
// в ином случае снова вызывается нулевой recvfrom, дабы сбросить пакет с очереди

// Возможно в этом нет большой нужды, если пакеты ограничены по размерам до 1500 байт

char PACKET_BUFFER[PACKET_BUFFER_MAX_SIZE];
char* PACKET_BUFFER_PTR = PACKET_BUFFER;

#define PACKET_BUFFER_SIZE (long long)PACKET_BUFFER_PTR - (long long)PACKET_BUFFER

#define addPacketData(data)				\
		_Generic((data),				\
		 int8_t: addPacketDataINT8,		\
	    uint8_t: addPacketDataUINT8,	\
		int16_t: addPacketDataINT16,	\
	   uint16_t: addPacketDataUINT16,	\
	    int32_t: addPacketDataINT32,	\
	   uint32_t: addPacketDataUINT32,	\
	    int64_t: addPacketDataINT64,	\
	   uint64_t: addPacketDataUINT64,	\
	   	 )(data)

// TODO
// #define addPacketDataArray(data_ptr)		\
// 		_Generic((data_ptr),				\
// 	   int16_t*: addPacketDataArrayINT8,	\
// 	   )(data)

// Drops pointer to the beginning effectivly erasing the data
// Each packet has to begin with packet type index
__forceinline void newPacket(PacketType_T packet_type)
{
	PACKET_BUFFER[0] = packet_type;
	PACKET_BUFFER_PTR = &PACKET_BUFFER[1];
}

__forceinline void copyPacketData(char* dest)
{
	memcpy(dest, PACKET_BUFFER, PACKET_BUFFER_SIZE);
}

__forceinline void addPacketDataINT8(int8_t data)
{
	*(PACKET_BUFFER_PTR++) = data;
}

__forceinline void addPacketDataUINT8(uint8_t data)
{
	*(PACKET_BUFFER_PTR++) = data;
}

__forceinline void addPacketDataINT16(int16_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 8;
	*(PACKET_BUFFER_PTR++) = data & 0xFF;
}

__forceinline void addPacketDataUINT16(uint16_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 8;
	*(PACKET_BUFFER_PTR++) = data & 0xFF;
}

__forceinline void addPacketDataINT32(int32_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 24;
	*(PACKET_BUFFER_PTR++) = (data & 0xFF0000) >> 16;
	*(PACKET_BUFFER_PTR++) = (data & 0x00FF00) >>  8;
	*(PACKET_BUFFER_PTR++) =  data & 0x0000FF;
}

__forceinline void addPacketDataUINT32(uint32_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 24;
	*(PACKET_BUFFER_PTR++) = (data & 0xFF0000) >> 16;
	*(PACKET_BUFFER_PTR++) = (data & 0x00FF00) >>  8;
	*(PACKET_BUFFER_PTR++) =  data & 0x0000FF;
}

__forceinline void addPacketDataINT64(int64_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 56;
	*(PACKET_BUFFER_PTR++) = (data & 0xFF000000000000) >> 48;
	*(PACKET_BUFFER_PTR++) = (data & 0x00FF0000000000) >> 40;
	*(PACKET_BUFFER_PTR++) = (data & 0x0000FF00000000) >> 32;
	*(PACKET_BUFFER_PTR++) = (data & 0x000000FF000000) >> 24;
	*(PACKET_BUFFER_PTR++) = (data & 0x00000000FF0000) >> 16;
	*(PACKET_BUFFER_PTR++) = (data & 0x0000000000FF00) >>  8;
	*(PACKET_BUFFER_PTR++) =  data & 0x000000000000FF;
}

__forceinline void addPacketDataUINT64(uint64_t data)
{
	*(PACKET_BUFFER_PTR++) = data >> 56;
	*(PACKET_BUFFER_PTR++) = (data & 0xFF000000000000) >> 48;
	*(PACKET_BUFFER_PTR++) = (data & 0x00FF0000000000) >> 40;
	*(PACKET_BUFFER_PTR++) = (data & 0x0000FF00000000) >> 32;
	*(PACKET_BUFFER_PTR++) = (data & 0x000000FF000000) >> 24;
	*(PACKET_BUFFER_PTR++) = (data & 0x00000000FF0000) >> 16;
	*(PACKET_BUFFER_PTR++) = (data & 0x0000000000FF00) >>  8;
	*(PACKET_BUFFER_PTR++) =  data & 0x000000000000FF;
}
