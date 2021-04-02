#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTextureCoords;

out vec2 fTextureCoords;

uniform mat4 	model;
uniform mat4 	view;
uniform mat4 	projection;
uniform float 	scale;

void main()
{
	// Spherical billboarding
	mat4 modelView = view * model;
	modelView[0][0] = 1;
	modelView[0][1] = 0;
	modelView[0][2] = 0;

	modelView[2][0] = 0;
	modelView[2][1] = 0;
	modelView[2][2] = 1;

	// Previous operation does erase scale, so, we should apply it after
	vec3 scaledPos = vPos * scale;

	gl_Position = projection * modelView * vec4(scaledPos, 0.2f);
	fTextureCoords = vTextureCoords;
}
