#version 330 core

//light reflection depends on orientation of surface, the normal value for eack vertex gives an indication of it's orientation
//this vertex shader has a position and normal attribute for each vertex

// input data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

// uniform input data
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat3 uNormalMatrix;

// output data
//for phong shading (per fragment lighting) the vertex positions and normals must be output by the vertex shader
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	// set vertex position
    gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0f);

	// set vertex shader output
	//this will be interpolated for each fragment
	//the position and normal values are multiplied by the model matrix and normal matrix (provided from the application code)
	//and lighting will be performed in the world space (some prefer to do this in view space)
	vPosition = (uModelMatrix * vec4(aPosition, 1.0f)).xyz;
	vNormal = uNormalMatrix * aNormal;
}
