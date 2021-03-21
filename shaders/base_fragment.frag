#version 330 core

out vec4 fColor;

in vec2 fTextureCoords;
uniform sampler2D bindedTexture;
uniform vec2 subtextureCoords;
uniform vec2 subtextureSize;

void main()
{
	vec2 actualTextureCoords = vec2(
		subtextureCoords.x + subtextureSize.x * fTextureCoords.x,
		subtextureCoords.y + subtextureSize.y * fTextureCoords.y
	);
	fColor = texture(bindedTexture, actualTextureCoords);
	// fColor = vec4(subtextureCoords.x, 0.0, 0.0, 1.0);
	if (fColor.a < 0.1)
		discard;
}
