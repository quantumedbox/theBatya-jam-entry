// Void templetes for del functions

#include "engine.h"

// TODO Maybe we should state the standart for del functions in all modules? that they are always executing through void* arg

void VOID_delGameObj(void* in)
{
	delGameObj((GameObj*)in);
}
