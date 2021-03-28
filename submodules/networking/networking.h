#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "errors.h"

#define ETHERNET_MTU 	1500
#define WLAN_MTU		2272

#define LOCALHOST		"127.0.0.1"
#define DEFAULT_PORT 	49123	// for local machine hosting

#ifndef PACKET_BUFFER_MAX_SIZE
#	define PACKET_BUFFER_MAX_SIZE ETHERNET_MTU	// defaults to ethernet
#endif

#define LASTWSAERROR(description) 		WSAERROR(description, WSAGetLastError())
#define PRINTLASTWSAERROR(description)	printf("WSAError: %d\n%s\n", WSAGetLastError(), description)

/* 		:: RELIABLE DATAGRAM SUPERSTRUCTURE ::

	Требования:
		. Избежание повторных отправлений пакетов. Клиент должен сообщать лично, что ему нужно.
		. Передача частей крупных данных без привязки к порядку получения.
	      Сервер держит всю информацию до тех пор, пока клиент её не получит или наступит timeout
		. Возможность открытия множества туннелей одновременно, распараллеливание
		. Принимающей и отправляющей стороной могут быть любые части сети, но решение остаётся за сервером

	Структура инкапсулируемого загаловка:
		. Tunnel session ID 	-- индификатор сессии к которой относится данный пакет
		. Packet ID 			-- если текущий флаг TUNNEL_DATA, то Packet ID используется
								   для указания того, к какой части потока относится полученная информация
								   при TUNNEL_REQUEST_PACKET к пакету не должна прикрепляться никакая информация,
								   а Packet ID использовать для формирования нового пакета и отправки его
		. Flag 					-- могут быть: TUNNEL_DATA, TUNNEL_REQUEST_PACKET

*/

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
	// Tunnel organisation (implementation of Reliable Datagram Superstructure)
	TUNNEL_REQUEST,			// Try to establish a tunnel connection
	TUNNEL_ACCEPT,			// Say to server that client is ready to recieve packets
	TUNNEL_PACKET,			// Packet with header designed to both data transfer and data request purposes

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

void initWSA()
{
	WORD winsock_version = 0x202;
	WSADATA winsock_data;
	if (WSAStartup(winsock_version, &winsock_data)) {
		LASTWSAERROR("WSA initialization error");
	}
}

SOCKET newSocket(int address_family, int type, int protocol)
{
	SOCKET sock = socket(address_family, type, protocol);
	if (sock == INVALID_SOCKET)
		LASTWSAERROR("Error on socket creation\n");

	return sock;
}

// Defaults to UPD IPv4
__forceinline SOCKET newUDPSocket()
{
	return newSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

// ПРИНЦИП:
// в основном лупе non-blocking recvfrom сначала пытается получить данные в нулевой буффер с MSG_PEEK флагом
// это делается для того, чтобы получить IP и порт соединения, дабы понять, является ли адрес "зарегистрированным"
// если адрес актуален - вызывается новый recvfrom на этот раз списывающий данные пакета
// в ином случае снова вызывается нулевой recvfrom, дабы сбросить пакет с очереди

// > Возможно, в этом нет большой нужды, если пакеты ограничены по размерам до 1500 байт

char PACKET_BUFFER[PACKET_BUFFER_MAX_SIZE];
char* PACKET_BUFFER_PTR = PACKET_BUFFER;

#define PACKET_BUFFER_SIZE (long long)PACKET_BUFFER_PTR - (long long)PACKET_BUFFER //ptrdiff_t

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
// #define addPacketDataArray(data_ptr)
// 		_Generic((data_ptr),
// 	   int16_t*: addPacketDataArrayINT8,
// 	   )(data)

// TODO Должны ли мы думать о том, что может понадобиться писать два пакета одновременно?
// 		Текущая реализация через state machine локанична, но лишена такой возможности.

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
