// Source code by Patrick Crouch
// Each comment line refers to the line of code immediately following it

// includes standard library functions
#include <stdlib.h> 
// includes glew header files
#include <GL/glew.h>
// includes freeglut header files
#include <GL/freeglut.h>

// includes glm library
#include <glm/glm.hpp>
// includes operations and data types for matrix transforms
#include <glm/gtc/matrix_transform.hpp>
// includes part of glm operations
#include <glm/gtc/type_ptr.hpp>

// negates necessity of typing glm::(function) before every glm call
using namespace glm;

// preprocessor directive to change instances of BUFFER_OFFSET
#define BUFFER_OFFSET(offset) ((GLvoid *) offset)

// Create shader program reference
GLint program;

// Create array for buffer objects
GLuint bufferObjects[3];
// create variable for passing vertex data to vertex shader
GLint vPos;
// create variable for passing transformation matrix to shader
GLint mvpMatrixID;
// create variable for passing color vector to fragment shader
GLint uColor;

// create projection matrix
mat4 projMatrix;
// create view matrix
mat4 viewMatrix;
// create model matrix for triangle
mat4 modelMatrix;

// initialization and loading of vertex data and shaders
void init()
{
	// simple triangle model
	GLfloat trivertices[][3] = {
		// first vertex
		{ -1.0f, 0.0f, 0.0f },
		// second vertex
		{ 1.0f, 0.0f, 0.0f },
		// third vertex
		{ 0.0f, 1.5f, 0.0f },
	};

	// Generate buffers, (number of objects, array to hold them), 3 objects, 3-item array
	glGenBuffers(3, bufferObjects);

	// Connect first buffer from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[0]);

	// Fill the first buffer with trivertices vertex data
	glBufferData(GL_ARRAY_BUFFER, sizeof(trivertices), trivertices, GL_STATIC_DRAW);

	// simple set of vertices for box
	GLfloat boxVertices[][3] = {
		// first and second box vertices, indices 0 and 1
		{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
		// third and fourth box vertices, indices 2 and 3
		{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 1.0f },
		// fifth and sixth box vertices, indices 4 and 5
		{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 1.0f },
		// seventh and eighth box vertice, indices 6 and 7
		{ 1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f }
	};
	// Connect second buffer from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[1]);
	// Fill the second buffer with boxVertices data
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	// simple set of elements (faces) for box
	GLbyte boxIndices[] = {
		// two triangles for positive x element (face)
		4, 5, 7,	4, 7, 6,
		// two triangles for negative x element (face)
		0, 2, 3,	0, 3, 1,
		// two triangles for positive y element (face)
		2, 6, 7,	2, 7, 3,
		// two triangles for negative y element (face)
		0, 1, 5, 	0, 5, 4,
		// two triangles for positive z element (face)
		0, 4, 6, 	0, 6, 2,
		// two triangles for negative z element (face)
		1, 3, 7, 	1, 7, 5
	};
	// Connect third buffer from CPU to GPU 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[2]);
	// Fill the third buffer with boxIndices data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);

	// Create vertex shader source code
	const char* vSource = {
		// use GLSL version 430 ( to coordinate with OpenGL 4.3 )
		"#version 430\n"
		// pass in variable for vertex locations
		"in vec3 vPos;"
		// pass in variable for model-view-projection matrix
		"uniform mat4 mvp_matrix;"
		// vertex shader main method
		"void main() {"
		// next drawn vertex = position * transformation (right to left multiplication)
		"	gl_Position = mvp_matrix * vec4(vPos, 1.0f);"
		"}"
	};

	// Create fragment shader source code
	const char* fSource = {
		// use GLSL version 430 ( to coordinate with OpenGL 4.3 )
		"#version 430\n"
		// uniform variable to pass in different colors for each object
		"uniform vec4 uColor = vec4(1.0, 0.0, 0.0, 1.0);"
		// pass out variable for color of drawn pixels (fragments)
		"out vec4 fragColor;"
		// fragment shader main method
		"void main() {"
		// returns the defined color to the framebuffer for rendering
		"	fragColor = uColor;"
		"}"
	};

	// Create vertex and fragment shader references
	GLuint vShader, fShader;
	// Create empty vertex shader
	vShader = glCreateShader(GL_VERTEX_SHADER);
	// Create empty fragment shader
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Associate vertex shader source reference with vertex shader
	glShaderSource(vShader, 1, &vSource, NULL);
	// Associate fragment shader source reference with fragment shader
	glShaderSource(fShader, 1, &fSource, NULL);
	// Compile vertex shader
	glCompileShader(vShader);
	// Compile fragment shader
	glCompileShader(fShader);
	// Create shader program
	program = glCreateProgram();
	// Associate vertex shader with shader program
	glAttachShader(program, vShader);
	// Associate fragment shader with shader program
	glAttachShader(program, fShader);
	// Link the shader program (as a compilation step)
	glLinkProgram(program);
	// get vertex position reference from shader program
	vPos = glGetAttribLocation(program, "vPos");
	// get mvp matrix reference from shader program
	mvpMatrixID = glGetUniformLocation(program, "mvp_matrix");
	// get uColor reference from shader program to enable object color change
	uColor = glGetUniformLocation(program, "uColor");
	// Indicate background color for window (black in this case)
	glClearColor(0, 0, 0, 1);

	glEnable(GL_DEPTH_TEST);
}

// reshape function called whenever window size is changed
void reshape(int width, int height)
{
	// determines width and height within which the 3d picture will be drawn
	glViewport(0, 0, width, height);
	// projection matrix, defined using call to glm::perspective()
	projMatrix = perspective(radians(60.0f), (float)width / (float)height, 0.1f, 5000.0f);
	// view matrix, defined using call to glm::lookAt(), translates camera to z = 2, and retains orientation of camera view normal
	viewMatrix = lookAt(vec3(0.0f, 0.0f, 4.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

// display event loop, which does the object drawing onto the back buffer, then a buffer swap
void display()
{
	// clears the background color within specified depth value
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// specifies shader program to use in current rendering state
	glUseProgram(program);
	// defines translation matrix using glm to specified values
	mat4 translateMatrix = translate(mat4(1.0f), vec3(0.0f, 0.5f, 0.0f));
	// defines x-axis rotation matrix using glm to specified values
	mat4 rotationMatrixY = rotate(mat4(1.0f), radians(15.0f), vec3(0.0f, 1.0f, 0.0f));
	// combines previous four matrices into model matrix
	modelMatrix = translateMatrix * rotationMatrixY;
	// combines model, projection, and view matrices into mvp matrix for passing into vertex shader
	mat4 mvpMatrix = projMatrix * viewMatrix * modelMatrix;
	// passes mvp reference, count of matrices, transpose value, and the pointer for the mvp matrix to vertex shader
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, value_ptr(mvpMatrix));
	// declare color to pass to fragment shader
	GLfloat color[4] = { 0.8f, 0.8f, 0.0f, 1.0f };
	// pass color to uniform vector location in fragment shader
	glProgramUniform4fv(program, uColor, 1, color);
	// sets buffer object to first buffer in bufferObjects array (the triangle
	glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[0]);
	// Associates the vertex array in bufferObjects[0] (the triangle) with pointer to vPos
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// Enables vPos for use in the shader
	glEnableVertexAttribArray(vPos);
	// Final step of actually drawing the triangle
	glDrawArrays(GL_TRIANGLES, 0, 3);
	// (11/12/2016 12:15p comment to create a change in github) more garbage

	// change translation matrix to move cube to different location
	translateMatrix = translate(mat4(1.0f), vec3(-0.5f, -0.75f, 0.0f));
	// change rotation matrix to rotate cube
	rotationMatrixY = rotate(mat4(1.0f), radians(-15.0f), vec3(0.0f, 1.0f, 0.0f));
	// calculate new model matrix for cube
	modelMatrix = translateMatrix * rotationMatrixY;
	// calculate new mvp matrix for cube
	mvpMatrix = projMatrix * viewMatrix * modelMatrix;
	// Associate newly calculated mvp matrix with vertex shader in value
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, value_ptr(mvpMatrix));
	// declare color to pass to fragment shader
	GLfloat color2[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	// pass color to uniform vector location in fragment shader
	glProgramUniform4fv(program, uColor, 1, color2);
	// sets buffer object to second buffer in bufferObjects array (the box)
	glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[1]);
	// Associates the vertex array in bufferObjects[1] (the box) with pointer to vPos
	glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	// Enable vPos for use in the shader
	glEnableVertexAttribArray(vPos);
	// Associates element array in bufferObjects[2] (the box) implicit in element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[2]);
	// Final step of actually drawing the box
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));

	// Clear bound array buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// CLear bound element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// After completing display loop on back buffer, exchanges with front buffer to display what was drawn
	glutSwapBuffers();
}

// main function of program, defines callbacks and initializes various necessary functions
void main(int argc, char *argv[])
{
	// starts glut
	glutInit(&argc, argv);
	// sets initial glut values, such as color definitions, buffer number, and depth
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// Creates the display window for opengl
	glutCreateWindow(argv[0]);
	// initializes extension wrangler library
	glutReshapeWindow(800, 800);
	glewInit();
	// calls init function defined above
	init();
	// calls the drawing part of the CPU-side program
	glutDisplayFunc(display);
	// callback for when the window size changes
	glutReshapeFunc(reshape);
	// starts the show
	glutMainLoop();
}