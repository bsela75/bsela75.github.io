
//header inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM math header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//image loader inclusion
#include "SOIL2/SOIL2.h"

using namespace std; //use standard namespace

#define WINDOW_TITLE "Bookcase" //window title macro

//shader program macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/*variable declaration for shader window size initialization buffer and array objects*/
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO, EBO, texture;
GLfloat degrees = glm::radians(0.0f); //converts float to degrees

//cube and light color
glm::vec3 lightColor(1.0f, 0.0f, 0.0f);
glm::vec3 secondLightColor(0.0f, 1.0f, 1.0f);

//Light position and scale
glm::vec3 lightPosition(1.0f, 0.5f, -3.0f);

                      //ambient   specular    highlight
glm::vec3 lightStrength(1.0f,     1.0f,       0.5f);

//camera rotation
float cameraRotation = glm::radians(-45.0f);
GLfloat cameraSpeed = 0.005f; //Movement speed per frame

GLchar currentKey; //Will store key pressed
GLfloat lastMouseX = 400, lastMouseY = 300; //Locks mouse cursor center screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f, zoom = 0.0f; //Mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.005f; //used for mouse camera rotation sensitivity
bool mouseDetected = true; //Initially true when mouse movement is detected
bool leftMouseButton = false;
bool rightMouseButton = false;
bool altDown = false;

//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -8.0f); //Initial camera position. placed -8 units z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); //Temporary z unit vector
glm::vec3 front; //Temporary z unit vector for mouse
glm::vec3 cameraRotateAmt;

//function prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UMouseClick(int button, int state, int x, int y);
void UMousePressedMove(int x, int y);
//void UKeyboard(unsigned char key, int x, int y);
void UGenerateTexture(void);

//Vertex shader source code
const GLchar * vertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; //Vertex data from vertex attrib pointer 0
	layout (location = 2) in vec2 textureCoordinate;

	out vec2 mobileTextureCoordinate;

	//global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main(){
			gl_Position = projection * view * model * vec4(position, 1.0f); //transforms vertices to clip coordinates
			mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); //flips the texture horizontal
		}
);

//fragment shader source code
const GLchar * fragmentShaderSource = GLSL(330,
	in vec2 mobileTextureCoordinate;

	out vec4 gpuTexture; //variable to pass color data to the GPU

	uniform sampler2D uTexture; //Useful when working with multiple textures

	void main(){
		gpuTexture = texture(uTexture, mobileTextureCoordinate);
	}
);

const GLchar * lightVertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; //VAP position 0 for vertex position data
	layout (location = 1) in vec3 normal; //VAP position 1 for normals
	layout (location = 2) in vec2 textureCoordinate;

	out vec3 Normal; //for outgoing normals to fragment shader
	out vec3 FragmentPos; // for outgoing color / pixels to fragment shader
	out vec2 mobileTextureCoordinate; // uv coords for texture

	//uniform / global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

    void main(){
        gl_Position = projection * view * model * vec4(position, 1.0f);//Transforms vertices into clip coordinates
        Normal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        FragmentPos = vec3(model * vec4(position, 1.0f)); //Gets fragment / pixel position in world space only (exclude view and projection)
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); //flips the texture horizontal
	}
);

const GLchar * lightFragmentShaderSource = GLSL(330,
	in vec3 Normal; //For incoming normals
	in vec3 FragmentPos; //for incoming fragment position
	in vec2 mobileTextureCoordinate;

	out vec4 result; //for outgoing light color to the GPU

	//Uniform / Global variables for object color, light color, light position and camera/view position
	uniform vec3 lightColor;
	uniform vec3 secondLightColor;
	uniform vec3 lightPos;
	uniform vec3 viewPosition;
    uniform vec3 lightStrength;
	uniform sampler2D uTexture; //Useful when working with multiple textures

    void main(){
    	vec3 norm = normalize(Normal); //Normalize vectors to 1 unit
    	vec3 ambient = lightStrength.x * lightColor; //Generate ambient light color
    	vec3 ambientTwo = lightStrength.x * secondLightColor;//Generate second ambient light color
    	vec3 lightDirection = normalize(lightPos - FragmentPos); //Calculate distance (light direction) between light source and fragments/pixels on
    	float impact = max(dot(norm, lightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
    	vec3 diffuse = impact * lightColor; //Generate diffuse light color
    	vec3 viewDir = normalize(viewPosition - FragmentPos); //Calculate view direction
    	vec3 reflectDir = reflect(-lightDirection, norm); //Calculate reflection vector
    	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);
    	vec3 specular = lightStrength.y * specularComponent * lightColor;

    	//Calculate phong result
    	vec3 phongOne = (ambient + diffuse + specular) * vec3(texture(uTexture, mobileTextureCoordinate));

    	// hardcode second light position
    	lightDirection = normalize(vec3(1.0f, 0.0f, -3.0f)- FragmentPos);
    	impact = max(dot(norm, lightDirection), 0.0); //Calculate diffuse impact by generating dot product of normal and light
    	diffuse = impact * secondLightColor; //Generate diffuse light color
    	viewDir = normalize(viewPosition - FragmentPos); //Calculate view direction
    	reflectDir = reflect(-lightDirection, norm); //Calculate reflection vector
    	specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), lightStrength.z);
    	// hardcode second light spec
    	vec3 specularTwo = 0.1f * specularComponent * secondLightColor;

    	vec3 phongTwo = (ambientTwo + diffuse + specularTwo) * vec3(texture(uTexture, mobileTextureCoordinate));

    	result = vec4(phongOne + phongTwo, 1.0f); //Send lighting results to GPU
	}
);

//main program
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK)
			{
				std::cout << "Failed to initialize GLEW" << std::endl;
				return -1;
			}

	UCreateShader();

	UCreateBuffers();

	UGenerateTexture();

	//use the shader program
	glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Set background color

	glutDisplayFunc(URenderGraphics);

	glutMouseFunc(UMouseClick);//detects mouse movement

	glutMotionFunc(UMousePressedMove); //Detects mouse press and movement.

	glutMainLoop();

	//Destroys buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	return 0;
}

//Resize the window
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

//Renders Graphics
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); //Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clears the screen

	glBindVertexArray(VAO); //Activate the Vertex array object before rendering and transforming them
	front.x = 10.0f * cos(yaw);
	front.y = 10.0f * sin(pitch);
	front.z = sin(yaw) * cos(pitch) * 10.0f;

	//Transforms the object
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); //Increase the object size by a scale of 2
	//model = glm::rotate(model, degrees, glm::vec3(0.0, 1.0f, 0.0f)); //Rotate the object y -45 degrees
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f)); //Place the object at the center of the viewport

	//Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(cameraPosition, CameraForwardZ, CameraUpY);
	view = glm::rotate(view, cameraRotateAmt.x, glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, cameraRotateAmt.y, glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, zoom));

	//Creates a perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);



	//retrieves and passes transform matrices to the shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
	GLint secondLightColorLoc, lightColorLoc, lightPositionLoc, lightStrengthLoc, viewPositionLoc;

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(shaderProgram, "lightPos");
    lightStrengthLoc = glGetUniformLocation(shaderProgram, "lightStrength");
    secondLightColorLoc = glGetUniformLocation(shaderProgram, "secondLightColor");
	viewPositionLoc = glGetUniformLocation(shaderProgram, "viewPosition");

	//pass color, light, and camera data to the cube shader programs corresponding uniforms
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(secondLightColorLoc, secondLightColor.r, secondLightColor.g, secondLightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(lightStrengthLoc, lightStrength.x, lightStrength.y, lightStrength.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glutPostRedisplay();

	glBindTexture(GL_TEXTURE_2D, texture);

	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 60);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


	glBindVertexArray(0); //deactivate the vertex array object

	glutSwapBuffers(); //flips the back buffer with the front buffer every frame. similar to gl flush
}

//Creates the Shader Program
void UCreateShader()
{
	//Vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // creates the vertex shader
	glShaderSource(vertexShader, 1, &lightVertexShaderSource, NULL); //Attaches the vertex shader to the source code
	glCompileShader(vertexShader); //compiles the vertex shader

	//Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Creates the fragment shader
	glShaderSource(fragmentShader, 1, &lightFragmentShaderSource, NULL); //Attaches the fragment shader to the source code
	glCompileShader(fragmentShader); //compiles the fragment shader

	//Shader Program
	shaderProgram = glCreateProgram(); //Creates the shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); //Attach vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); //Attach fragment shader to the shader program
	glLinkProgram(shaderProgram); //link vertex and fragment shader to shader program

	//delete the vertex and fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void UCreateBuffers()
{
	GLfloat vertices[] = {
				//X   //Y   //Z			//Normals					//Texture Coordinates

				// bottom shelf 4th row
				-0.5f, 0.0f, -0.25f, 	0.0f, -1.0f,  0.0f,			0.0f, 1.0f, // 0
				-0.5f, 0.0f,  0.25f, 	0.0f, -1.0f,  0.0f,			0.0f, 0.0f, // 1
				1.0f, 0.0f, -0.25f, 	0.0f, -1.0f,  0.0f,			1.0f, 1.0f, // 2
				1.0f, 0.0f, -0.25f, 	0.0f, -1.0f,  0.0f,			1.0f, 1.0f, // 3
				-0.5f, 0.0f,  0.25f, 	0.0f, -1.0f,  0.0f,			0.0f, 0.0f, // 4
				1.0f, 0.0f,  0.25f, 	0.0f, -1.0f,  0.0f,			1.0f, 0.0f, // 5

				//row 3
				-0.5f, 0.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 1.0f, // 6
				-0.5f, 0.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 7
				1.0f, 0.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 8
				1.0f, 0.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 9
				-0.5f, 0.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 10
				1.0f, 0.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 0.0f, // 11

				//2nd row
				0.0f, 1.0f, -0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 1.0f, // 12
				0.0f, 1.0f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 13
				1.0f, 1.0f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 14
				1.0f, 1.0f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 15
				0.0f, 1.0f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 16
				1.0f, 1.0f,  0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 0.0f, // 17

				//1st row top
				0.5f, 1.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 1.0f, // 18
				0.5f, 1.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 19
				1.0f, 1.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 20
				1.0f, 1.5f, -0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 1.0f, // 21
				0.5f, 1.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			0.0f, 0.0f, // 22
				1.0f, 1.5f,  0.25f, 	0.0f,  1.0f,  0.0f,			1.0f, 0.0f, // 23

				//1st column left side
				1.0f, 1.5f, -0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 24
				1.0f, 1.5f,  0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // 25
				1.0f, 0.0f, -0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 26
				1.0f, 0.0f, -0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 27
				1.0f, 0.0f,  0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // 28
				1.0f, 1.5f,  0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 29

				//2nd column
				0.5f, 1.5f, -0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 30
				0.5f, 1.5f,  0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // 31
				0.5f, 0.0f, -0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 32
				0.5f, 0.0f, -0.25f, 	1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 33
				0.5f, 0.0f,  0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // 34
				0.5f, 1.5f,  0.25f, 	1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 35

				//3rd column
				0.0f, 1.0f, -0.25f,    -1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 36
				0.0f, 1.0f,  0.25f,    -1.0f,  0.0f,  0.0f,			0.0f, 1.0f, // 37
				0.0f, 0.0f, -0.25f,    -1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 38
				0.0f, 0.0f, -0.25f,    -1.0f,  0.0f,  0.0f,			0.0f, 0.0f, // 39
				0.0f, 0.0f,  0.25f,    -1.0f,  0.0f,  0.0f,			1.0f, 0.0f, // 40
				0.0f, 1.0f,  0.25f,    -1.0f,  0.0f,  0.0f,			1.0f, 1.0f, // 41

				//column 4 right
				-0.5f, 0.5f, -0.25f,    -1.0f,  0.0f,  0.0f,		1.0f, 1.0f, // 42
				-0.5f, 0.5f,  0.25f,    -1.0f,  0.0f,  0.0f,		0.0f, 1.0f, // 43
				-0.5f, 0.0f, -0.25f,    -1.0f,  0.0f,  0.0f,		0.0f, 0.0f, // 44
				-0.5f, 0.0f, -0.25f,    -1.0f,  0.0f,  0.0f,		0.0f, 0.0f, // 45
				-0.5f, 0.0f,  0.25f,    -1.0f,  0.0f,  0.0f,		1.0f, 0.0f, // 46
				-0.5f, 0.5f,  0.25f,    -1.0f,  0.0f,  0.0f,		1.0f, 1.0f, // 47

		};
void UCreateBuffers()
{
	//position and color data
		GLfloat vertices[] = {
				//vertex positions 		//colors
				0.0f, 0.75f, 0.0f,		1.0f, 0.0f, 0.0f, // 0
				-0.5f, 0.0f, -0.5f,		0.0f, 1.0f, 0.0f, // 1
				-0.5f, 0.0f, 0.5f,		0.0f, 0.0f, 1.0f, // 2
				0.5f, 0.0f, 0.5f,		1.0f, 0.0f, 1.0f,//  3
				0.5, 0.0f, -0.5f,		0.0f, 1.0f, 1.0f, // 4

								};
		//index data to share position data
		GLuint indices[] = {
				0, 1, 2,  //triangle 1
				0, 2, 3, //triangle 2
				0, 3, 4, //triangle 3
				0, 1, 4, //triangle 4
				1, 2, 4, //triangle 5
				2, 3, 4 //triangle 6

					};
	//Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	//Activate the VAO before binding and setting VBOs and VAPs
	glBindVertexArray(VAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO

	//set attribute pointer 0 to hold position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); //Enables vertex attribute

	//activates element buffer object / indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); //copy indices to ebo

	//Set attribute pointer 1 to hold Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Set attribute pointer 2 to hold Texture coordinate data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); //Unbind the VAO
}

//Generate and load the texture
void UGenerateTexture(){
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("wood.jpg", &width, &height, 0, SOIL_LOAD_RGB);//loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind the texture
}

//Implements the UMouseClick function
void UMouseClick(int button, int state, int x, int y)
{
	//sets the state for the alt key; true or false
	altDown = glutGetModifiers();

	//Sets the state for the mouse click
	if((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)){
		cout<<"Left Mouse Button Clicked!"<<endl; // for testing purposes
		leftMouseButton = true;
	}

	//Sets the state for the mouse click
	if((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)){
		cout<<"Left Mouse Button Released!"<<endl;// for testing purposes
		leftMouseButton = false;
	}

	//Sets the state for the mouse click
	if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)){
		cout<<"Right Mouse Button Clicked!"<<endl;// for testing purposes
		rightMouseButton = true;
	}

	//Sets the state for the mouse click
	if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)){
		cout<<"Right Mouse Button Released!"<<endl;// for testing purposes
		rightMouseButton = false;
	}

}

//Implements the UMouseMove function
void UMousePressedMove(int x, int y)
{
	//Immediately replaces center locked coordinates with new mouse coordinates
	if(mouseDetected)
	{
		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;
	}

	//Gets the direction the mouse was moved in x and y
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y; //Inverted Y

	//Updates with new mouse coordinates
	lastMouseX = x;
	lastMouseY = y;

	//Applies sensitivity to mouse direction
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	//Orbits around the center
	if(altDown == true) {
		//Accumulates the yaw and pitch variables
		if(leftMouseButton == true){
			// rotate around the model
			cameraRotateAmt.x += mouseXOffset;
			cameraRotateAmt.y += mouseYOffset;
		}

	if(rightMouseButton == true){
		// zoom feature
		zoom += mouseYOffset;
		}
	} else {
		yaw += mouseXOffset;
		pitch += mouseYOffset;
	}
}



