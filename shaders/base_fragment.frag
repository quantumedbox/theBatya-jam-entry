#version 330 core

out vec4 fColor;

in vec2 fTextureCoords;

uniform sampler2D bindedTexture;
uniform vec2 subtextureCoords;

void main()
{
	fColor = texture(bindedTexture, fTextureCoords);

	if (fColor.a < 0.1)
		discard;
}
