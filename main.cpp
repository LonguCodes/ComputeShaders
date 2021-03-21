#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include <chrono>
#include <iostream>
#include <cmath>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective

using namespace std::chrono;

char* shaderLoadSource( const char* filePath );
unsigned int loadComputeShader( const char* filePath );
unsigned int loadDisplayShader( const char* vertex, const char* fragment );

#define UPDATE_RATE 50
#define POINT_SIZE 10
#define MAX_SPEED 50
#define POINT_COUNT 3000


#define SCREEN_WIDTH 3000
#define SCREEN_HEIGHT 1000


struct v2
{
	float x;
	float y;
};

v2 positions[POINT_COUNT];
v2 velocities[POINT_COUNT];

unsigned int positionBuffer;
unsigned int velocityBuffer;
unsigned int VBO;
unsigned int VAO;

unsigned int computeShader;
unsigned int displayShader;

glm::mat4 P =  glm::ortho( 0.0f, (float)SCREEN_WIDTH, 0.0f, (float )SCREEN_HEIGHT ) ;
void display();
void timer( int value );


int main( int argc, char** argv )
{
	glutInit( &argc, argv );
	glutInitContextVersion( 4, 3 );
	glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	glutInitWindowPosition( 0, 0 );
	glutCreateWindow( "Pierwszy prog" );
	glewInit();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glPointSize( POINT_SIZE );
	computeShader = loadComputeShader( "compute_shader.glsl" );
	displayShader = loadDisplayShader( "vertex_shader.glsl", "fragment_shader.glsl" );
	for ( int i = 0 ; i < POINT_COUNT ; ++i )
	{
		positions[i].x = ( float( rand() ) / RAND_MAX  ) * SCREEN_WIDTH ;
		positions[i].y = ( float( rand() ) / RAND_MAX  ) * SCREEN_HEIGHT ;

		velocities[i].x = float( rand() ) / RAND_MAX * 2 * MAX_SPEED - MAX_SPEED;
		velocities[i].y = float( rand() ) / RAND_MAX * 2 * MAX_SPEED - MAX_SPEED;
	}

	glUseProgram( computeShader );
	glGenBuffers( 1, &positionBuffer );
	glGenBuffers( 1, &velocityBuffer );

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, positionBuffer );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( v2 ) * POINT_COUNT, positions, GL_STREAM_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, positionBuffer );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, velocityBuffer );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( v2 ) * POINT_COUNT, velocities, GL_STREAM_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, velocityBuffer );
	glUseProgram( displayShader );

	glUniformMatrix4fv( glGetUniformLocation( displayShader, "P" ), 1, false, ( const GLfloat* )&P );

	glGenVertexArrays( 1, &VAO );

	glBindVertexArray( VAO );

	glGenBuffers( 1, &VBO );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( positions ), positions, GL_STREAM_DRAW );

	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );


	glBindVertexArray( 0 );


	glutTimerFunc( 1000 / UPDATE_RATE, timer, 0 );
	glutDisplayFunc( display );
	glutMainLoop();

	return 0;
}

void timer( int value )
{
	glUseProgram( computeShader );


	glUniform1i( glGetUniformLocation( computeShader, "updateRate" ), UPDATE_RATE );
	glUniform1f( glGetUniformLocation( computeShader, "pointSize" ), POINT_SIZE );

	glUniform2i( glGetUniformLocation( computeShader, "boxSize" ), SCREEN_WIDTH,SCREEN_HEIGHT );


	glDispatchCompute( POINT_COUNT, 1, 1 );
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

	glBindBuffer( GL_SHADER_STORAGE_BUFFER, positionBuffer );

	glm::vec2* dataRead = ( glm::vec2* )glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER, 0, sizeof( glm::vec2 ) * POINT_COUNT, GL_MAP_READ_BIT );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( v2 ) * POINT_COUNT, dataRead, GL_STREAM_DRAW );
	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glutPostRedisplay();
	glutTimerFunc( 1000 / UPDATE_RATE, timer, 0 );
}

void display()
{
	glUseProgram( displayShader );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glBindVertexArray( VAO );
	glUniformMatrix4fv( glGetUniformLocation( displayShader, "P" ), 1, false, ( const GLfloat* )&P );
	glDrawArrays( GL_POINTS, 0, POINT_COUNT );

	glutSwapBuffers();
}


unsigned int loadComputeShader( const char* filePath )
{
	unsigned int computeShader = glCreateShader( GL_COMPUTE_SHADER );

	char* computeShaderSource = shaderLoadSource( filePath );
	if ( !computeShaderSource )
		return -1;

	glShaderSource( computeShader, 1, ( const char** )&computeShaderSource, NULL );
	glCompileShader( computeShader );

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader( shaderProgram, computeShader );
	glLinkProgram( shaderProgram );

	glDeleteShader( computeShader );

	return shaderProgram;
}

unsigned int loadDisplayShader( const char* vertex, const char* fragment )
{
	unsigned int vertexShader = glCreateShader( GL_VERTEX_SHADER );
	unsigned int fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	char* vertexShaderSource = shaderLoadSource( vertex );
	char* fragmentShaderSource = shaderLoadSource( fragment );

	if ( !fragmentShaderSource || !vertexShaderSource )
		return -1;

	glShaderSource( vertexShader, 1, ( const char** )&vertexShaderSource, NULL );
	glShaderSource( fragmentShader, 1, ( const char** )&fragmentShaderSource, NULL );
	glCompileShader( vertexShader );
	glCompileShader( fragmentShader );

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader( shaderProgram, vertexShader );
	glAttachShader( shaderProgram, fragmentShader );
	glLinkProgram( shaderProgram );
	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );
	return shaderProgram;
}

char* shaderLoadSource( const char* filePath )
{
	const size_t blockSize = 512;
	FILE* fp;
	char buf[512];
	char* source = NULL;
	size_t tmp, sourceLength = 0;
	fopen_s( &fp, filePath, "r" );
	if ( !fp )
	{
		fprintf( stderr, "shaderLoadSource(): Unable to open %s for reading\n", filePath );
		return NULL;
	}

	while ( ( tmp = fread( buf, 1, blockSize, fp ) ) > 0 )
	{
		char* newSource = ( char* )malloc( sourceLength + tmp + 1 );
		if ( !newSource )
		{
			fprintf( stderr, "shaderLoadSource(): malloc failed\n" );
			if ( source )
				free( source );
			return NULL;
		}

		if ( source )
		{
			memcpy( newSource, source, sourceLength );
			free( source );
		}
		memcpy( newSource + sourceLength, buf, tmp );

		source = newSource;
		sourceLength += tmp;
	}

	fclose( fp );
	if ( source )
		source[sourceLength] = '\0';

	return source;
}
