// Function interface

// Pointer to buffer struct
// Implementation does all the magic, caller module should not care
typedef void* StaticTextObj;

StaticTextObj static_text_new(char* fontDir, char* text);
void static_text_render(StaticTextObj obj);
