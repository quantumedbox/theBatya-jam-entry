// Queue threads are designed to process sockets independently from main program
// It's done via lists of commands that are constantly being written to be processed in parent thread

/*
		. Возможность родителя запросить текущий список вне зависимости от того, был ли он записан полностью (via control flags)
		. Сервер должен иметь возможность ограничивать количество единовременно получаемых пакетов с одного адреса
*/
