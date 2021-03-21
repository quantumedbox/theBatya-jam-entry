#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

void populateTextureObjFromFile(TextureObj* obj, char* dir)
{
	GLuint texture;
	glGenTextures(1, &texture);

	obj->texture_ptr = texture;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, channels;
	// stbi_set_flip_vertically_on_load(1);
	unsigned char *imagedata = stbi_load(dir, &width, &height, &channels, 0);

	obj->size[0] = width; obj->size[1] = height;

	if (!imagedata) {
		FILE_ERROR(TEXTURE_LOADING_ERR, dir);
	}

	_Bool is_alpha = false;
	if (channels > 3) is_alpha = true;

	glTexImage2D(
		GL_TEXTURE_2D,	0,
		is_alpha ? GL_RGBA : GL_RGB,
		width,	height,	0,
		is_alpha ? GL_RGBA : GL_RGB,
		GL_UNSIGNED_BYTE,
		imagedata
	);

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(imagedata);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}
