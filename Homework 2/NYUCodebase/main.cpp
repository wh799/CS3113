#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ShaderProgram.h"
#include "Matrix.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

class Paddle {
public:
	Paddle(GLuint paddle) {}

	float left;
	float right;
	float top;
	float bottom;
};

class Ball {
public:
	Ball(GLuint pong) {}
	float pos_x = 0.0f;
	float pos_y = 0.0f;
	float speed = 3.0f;
};


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// Before Loop

	glViewport(0, 0, 640, 360);

	ShaderProgram program("vertex_textured.glsl", "fragment_textured.glsl");

	GLuint bluePad = LoadTexture("laserBlue14.png");
	GLuint redPong = LoadTexture("laserRed10.png");
	GLuint greenPad = LoadTexture("laserGreen06.png");

	Paddle leftPaddle(greenPad);
	Paddle rightPaddle(bluePad);
	Ball pong(redPong);

	float lastFrameTicks = 0.0f;

	Matrix projectionMatrix;
	Matrix viewMatrix;

	Matrix rightMatrix;
	Matrix pongMatrix;
	Matrix leftMatrix;


	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);

	glUseProgram(program.programID);

	float angle = 0.0f;
	float blueY = 0.0f;
	float blueX = 0.0f;
	float greenY = 0.0f;

	// During Loop

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);

		// Right Paddle
		program.setModelMatrix(rightMatrix);
		glBindTexture(GL_TEXTURE_2D, bluePad);
		float Rightvertices[] = { 2.4f, -0.5f, 2.3f, -0.5f, 2.3f, 0.5f, 2.3f, 0.5f, 2.4f, 0.5f, 2.4f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, Rightvertices);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Left Paddle

		program.setModelMatrix(leftMatrix);
		glBindTexture(GL_TEXTURE_2D, greenPad);
		float Leftvertices[] = { -4.4f, -0.5f, -4.3f, -0.5f, -4.3f, 0.5f, -4.3f, 0.5f, -4.4f, 0.5f, -4.4f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, Leftvertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Ball

		pongMatrix.identity();
		pongMatrix.Scale(0.3f, 0.3f, 1.0f);
		program.setModelMatrix(pongMatrix);

		glBindTexture(GL_TEXTURE_2D, redPong);
		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_UP]) {
			blueY += elapsed * 2;
		}
		if (keys[SDL_SCANCODE_DOWN]) {
			blueY -= elapsed * 2;
		}
		if (keys[SDL_SCANCODE_RIGHT]) {
			blueX += elapsed * 2;
		}
		if (keys[SDL_SCANCODE_LEFT]) {
			blueX -= elapsed * 2;
		}


		rightMatrix.identity();
		rightMatrix.Translate(blueX, blueY, 1.0);

		leftMatrix.identity();
		leftMatrix.Translate(1.0, greenY, 1.0);

		pongMatrix.identity();
		pongMatrix.Translate(1.0, cos(45) + pong.speed * elapsed, 1.0);

		program.setModelMatrix(rightMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);


		if (pong.pos_x <= leftPaddle.right && pong.pos_y <= leftPaddle.top && pong.pos_y >= leftPaddle.bottom ||
			pong.pos_x >= rightPaddle.left && pong.pos_y <= rightPaddle.top && pong.pos_y >= rightPaddle.bottom) {
			pongMatrix.Translate(-pong.speed * elapsed, pong.speed * elapsed, 0.0f);
		}
		else if (pong.pos_x >= rightPaddle.right) {
			pongMatrix.Translate(-pong.pos_x, -pong.pos_y, 0.0f);
		}
		else if (pong.pos_x <= leftPaddle.left) {
			pongMatrix.Translate(-pong.pos_x, -pong.pos_y, 0.0f);
		}
		else if (pong.pos_y + 0.1f >= 2.0f || pong.pos_y - 0.1f <= -2.0f) {
			pongMatrix.Translate(pong.speed * elapsed, -pong.speed * elapsed, 0.0f);
		}
		else {
			pongMatrix.Translate(pong.speed * elapsed, pong.speed * elapsed, 0.0f);
		}

		glUseProgram(program.programID);

		SDL_GL_SwapWindow(displayWindow);

		glClear(GL_COLOR_BUFFER_BIT);
	}

	SDL_Quit();
	return 0;
}