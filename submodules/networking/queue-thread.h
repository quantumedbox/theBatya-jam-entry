#include <pthread.h>

#include "networking.h"

// Queue threads are designed to process sockets independently from the main program
// It's done via lists of commands that are constantly being written to be processed in parent thread

/*
		. Возможность родителя запросить текущий список вне зависимости от того, был ли он записан полностью (via control flags)
		. Сервер должен иметь возможность ограничивать количество единовременно получаемых пакетов с одного адреса
		. Пакеты с новых подключений записываются лишь в определенные временные окна
*/

#define PACKET_LIMIT_PER_CONNECTION	16	// maximum amount of packets that could be processed from a single address in a single queue walk
#define MAX_PACKETS_PER_PAGE		128
// #define MAX_BYTES_PER_PAGE			PACKET_MAX_SIZE * MAX_PACKETS_PER_PAGE
#define MAX_PAGES					4	// if all pages are filled - drop the oldest one and continue to write (??? maybe it should be optional ???)

// ----------------------------------------------------------------------- Typedefs -- //


typedef struct
{
	char**		packets;

	uint16_t	len;

	// used for determine which packet is latest or oldest
	uint64_t	timeStamp;

	// TODO ??? maybe just add a mutex ???
}
Page;


typedef struct
{
	// thread handle
	pthread_t 	thread;

	// write-read bytes buffers
	Page* 		pages;

	// index of page that is curently being written by thread 	// negative value means no current 	// only THREAD can write to it
	int16_t		currentWritePage;

	// index of page that is curently being read by parent 		// negative value means no current 	// only PARENT can write to it
	int16_t 	currentReadPage;

	// sets to true by parent when it needs any data to process even if page isn't written fully 	// only PARENT can write to it
	_Bool 		shouldDropWritePage;

	// sets to true by parent when thread needs to be finished										// only PARENT can write to it
	_Bool 		shouldExit;

	// socket from which data is coming
	SOCKET 		socket;
}
SocketQueueHandle;


// --------------------------------------------------------------------- Signatures -- //

SocketQueueHandle* 	socketQueueNew		(SOCKET);
void 				socketQueueExit		(SocketQueueHandle*);
void 				socketQueueStart	(SocketQueueHandle*);

// 					Process sub functions
void 				socketQueueProcessGetWritePage	(SocketQueueHandle*);

// ----------------------------------------------------------------- Thread process -- //


static void* socketQueueProcess(void* hanle)
{
	SocketQueueHandle* self = (SocketQueueHandle*)hanle;

	while (true)
	{
		if (self->shouldExit)
			pthread_exit(0);

		if (self->currentWritePage < 0) {
			socketQueueProcessGetWritePage(self);
			logf("new write page: %d\n", self->currentWritePage);
		}

		if (self->currentWritePage == self->currentReadPage)
			EXIT_ERROR(RACING_COND_ERR);

		if (socketCheckForReadability(self->socket, 1000))
		{

		}

		// ...
	}

	return NULL;
}

void socketQueueProcessGetWritePage(SocketQueueHandle* s)
{
	// first - check, if there's any empty page to write to

	for (int i = 0; i < MAX_PAGES; i++)
	{
		if (s->pages[i].len == 0 && s->currentReadPage != i)
		{
			s->currentWritePage = i;
			return;
		}
	}

	// if there's no empty pages then find the oldest and discard it

	int oldest = -1;

	for (int i = MAX_PAGES; i--;)
	{
		if (s->currentReadPage == i)
			continue;

		if (oldest == -1 || s->pages[oldest].timeStamp < s->pages[i].timeStamp)
			oldest = i;
	}

	if (oldest != -1)
	{
		s->currentWritePage = oldest;
		s->pages[oldest].len = 0;
	}

	else EXIT_ERROR(QUEUE_NO_FREE_PAGE_ERR);
}

// ---------------------------------------------------------------------- Functions -- //


SocketQueueHandle* socketQueueNew(SOCKET sock)
{
	SocketQueueHandle* new = (SocketQueueHandle*)malloc(sizeof(SocketQueueHandle));
	
	new->pages = (Page*)malloc(sizeof(Page) * MAX_PAGES);

	for (int i = MAX_PAGES; i--;) {
		new->pages[i].packets = (char**)malloc(sizeof(char*) * MAX_PACKETS_PER_PAGE);

		for (int n = MAX_PACKETS_PER_PAGE; n--;)
			new->pages[i].packets[n] = (char*)malloc(PACKET_MAX_SIZE);

		new->pages[i].len = 0;
	}

	new->socket		= sock;

	new->currentReadPage 		= -1;
	new->currentWritePage 		= -1;
	new->shouldDropWritePage 	=  0;
	new->shouldExit 			=  0;

	socketQueueStart(new);		// should it start here ?

	return new;
}

void socketQueueStart(SocketQueueHandle* s)
{
	pthread_t thread;

	int err;
	if ((err = pthread_create(&thread, NULL, socketQueueProcess, (void*)s)) != 0)
	{
		char* desc = '\0';
		sprintf(desc, "thread creation error %d", err);
		EXIT_ERROR_DESC(THREAD_STARTUP_ERR, desc);
	}

	s->thread = thread;
}

void socketQueueExit(SocketQueueHandle* s)
{
	s->shouldExit = true;

	int err;
	if ((err=pthread_join(s->thread, NULL)) != 0)
	{
		char* desc = '\0';
		sprintf(desc, "thread join error %d", err);
		EXIT_ERROR_DESC(THREAD_JOIN_ERR, desc);
	}
}
