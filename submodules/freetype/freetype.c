#include <freetype/freetype-gl.h>
#include <freetype/vertex-buffer.h>

#include "freetype.h"

// IT ALL IS QUITE HARD

StaticTextObj static_text_new(char* fontDir, char* text)
{
	texture_atlas_t* atlas = texture_atlas_new(512, 512, 1);
	texture_font_t* font = texture_font_new_from_file(atlas, 16, fontDir);
	vertex_buffer_t* buffer = vertex_buffer_new("v3i:t2f:c4f");
	vec2 pos = {{0,0}};
	vec4 color = {{1, 1, 1, 0.8}};
	// add_text(buffer, font, "HELLO", text, &color, &pos);

	return (StaticTextObj)buffer;
}

void static_text_render(StaticTextObj obj)
{
	vertex_buffer_t* buffer = (vertex_buffer_t*)obj;
	vertex_buffer_render(buffer, GL_TRIANGLES);
}
