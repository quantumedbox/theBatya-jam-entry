// Queue threads are designed to process sockets independently from the main program
// It's done via lists of commands that are constantly being written to be processed in parent thread

/*
		. Возможность родителя запросить текущий список вне зависимости от того, был ли он записан полностью (via control flags)
		. Сервер должен иметь возможность ограничивать количество единовременно получаемых пакетов с одного адреса
		. Пакеты с новых подключений записываются лишь в определенные временные окна
*/

#define PACKET_LIMIT_PER_CONNECTION	16	// maximum amount of packets that could be processed from a single address in a single queue walk
#define MAX_PACKETS_PER_PAGE		128
#define MAX_BYTES_PER_PAGE			PACKET_MAX_SIZE * MAX_PACKETS_PER_PAGE
#define MAX_PAGES					4	// if all pages are filled - drop the oldest one and continue to write
