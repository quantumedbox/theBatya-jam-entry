#define MAX_N_KEYS 128

#include "iter.h"

enum KEY_TYPES {
	UNKNOWN_KEY,
	MOVEMENT_KEY,
};

typedef struct {
	int key;
	int action;
	int type;
}
KeyBinding;

typedef struct
{
	Iter* keyBindings;
}
KeyLayout;

void keyLayout_bindNewKey(KeyLayout* layout, int key, int action, int type)
{
	KeyBinding* bind = (KeyBinding*)malloc(sizeof(KeyBinding));
	bind->key = key;
	bind->action = action;
	bind->type = type;
	addIter(layout->keyBindings, (void*)bind);
}

void keyLayout_initDefault(KeyLayout* layout)
{
	keyLayout_bindNewKey(layout, -1, -1, UNKNOWN_KEY);

	// keyLayout_bindNewKey(layout, GLFW_KEY_W, MOVE_FORWARD, MOVEMENT_KEY);
	// keyLayout_bindNewKey(layout, GLFW_KEY_S, MOVE_BACKWARD, MOVEMENT_KEY);
	// keyLayout_bindNewKey(layout, GLFW_KEY_A, MOVE_LEFT, MOVEMENT_KEY);
	// keyLayout_bindNewKey(layout, GLFW_KEY_D, MOVE_RIGHT, MOVEMENT_KEY);
	// keyLayout_bindNewKey(layout, GLFW_KEY_LEFT_SHIFT, MOVE_DOWN, MOVEMENT_KEY);
	// keyLayout_bindNewKey(layout, GLFW_KEY_SPACE, MOVE_UP, MOVEMENT_KEY);
}

KeyBinding* keyLayout_processKey(KeyLayout* layout, int target_key)
{
	startIter(layout->keyBindings);
	int remains;
	do {
		KeyBinding* bind = (KeyBinding*)nextIter(layout->keyBindings);
		if (bind->key == target_key)
			return bind;
		remains = remainsIter(layout->keyBindings);
	}
	while (remains);

	return indexedIter(layout->keyBindings, 0);
}
