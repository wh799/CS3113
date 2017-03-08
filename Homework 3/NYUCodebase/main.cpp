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
#include <vector>
#include <iostream>
#include <string>


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

void DrawText(ShaderProgram program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUseProgram(program.programID);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}


class SheetSprite {
public:

	SheetSprite() {};
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) :
		textureID(textureID), x(x), y(y), width(width), height(height), size(size) {};
	float size;
	unsigned int textureID;
	float x;
	float y;
	float width;
	float height;
	float direction;

	void setAttribute(float newX, float newY) { x = newX;  y = newY; }
};

void DrawSheetSprite(ShaderProgram program, Matrix modelMatrix, int index, int textureID, float x, float y, float size)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	int numInRow = 6;
	int numInColumn = 6;
	float width = 1.0 / (float)numInRow;
	float height = 1.0 / (float)numInColumn;
	float u = (float)(((int)index) % numInRow) / (float)numInRow;
	float v = (float)(((int)index) / numInRow) / (float)numInColumn;


	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height,
	};

	modelMatrix.identity();
	modelMatrix.Translate(x, y, 0.0);
	program.setModelMatrix(modelMatrix);

	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);


	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}
/*
class Bullet : Entity {
public:
	Bullet();
	Bullet(ShaderProgram *program, SheetSprite sprite);
	SheetSprite sprite;
	float x_position;
	float y_position;
	float speed = 0.75f;
	float direction;
	bool alive;
	Matrix modelMatrix;
	ShaderProgram *program;
};


Bullet::Bullet() {};
Bullet::Bullet(ShaderProgram *program, SheetSprite sprite) : program(program), sprite(sprite), x_position(x_position), y_position(y_position), direction(1), alive(true) {};

void BulletUpdate(ShaderProgram program, Matrix modelMatrix,bool alive, float x, float y, float direction, float elapsed) {
	program.setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(x, y, 0.0f);
	if (alive) {
		y += elapsed * 0.75f * direction;
	}
	if ((direction == 1 && y > 15.0f) || (direction == -1 && y < -15.0f))
		alive = false;
}
*/

class Entity
{
public:
	Entity() {};
	Entity(float x, float y) : x(x), y(y), height(1.0f), width(1.0f) {}
	float x;
	float y;
	float width;
	float height;
	int textureID;

	SheetSprite sprite;

	void Draw(ShaderProgram program) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		glTranslatef(x, y, 0.0);

		GLfloat vertices[] = { width * -0.5, height * 0.5, width * -0.5, height * -0.5, width * 0.5, height * -0.5, width * 0.5, height * 0.5 };

		GLfloat texCoords[] = { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);


		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void DrawSpriteSheetSprite(ShaderProgram program, int index, int spriteCountX, int spriteCountY) {

		float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
		float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0 / (float)spriteCountX;
		float spriteHeight = 1.0 / (float)spriteCountY;

		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height,
		};

		GLfloat vertices[] = { width * -0.5, height * 0.5, width * -0.5, height * -0.5, width * 0.5, height * -0.5, width * 0.5, height * 0.5 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);


		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

	}
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

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	GLuint font1 = LoadTexture("font1.png");
	GLuint ship = LoadTexture("ship.png");
	GLuint enemy = LoadTexture("SpaceInvaders.png");
	GLuint bullet = LoadTexture("laserBlue04.png");
	std::vector<SheetSprite> enemies;
	projectionMatrix.setOrthoProjection(-16.0, 16.0, -9.0, 9.0, -1.0, 1.0);

	enum GameState { STATE_TITLE_SCREEN, STATE_GAME, STATE_GAME_OVER };
	GameState state = STATE_TITLE_SCREEN;
	float lastFrameTicks = 0.0f;
	float positionX = 0.0f;
	float positionY = 0.0f;
	
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

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		switch (state)
		{
			case STATE_TITLE_SCREEN:
			{
				if (keys[SDL_SCANCODE_SPACE])
					state = STATE_GAME;
				break;
			}
			case STATE_GAME:
			{
				
				if (keys[SDL_SCANCODE_LEFT])
				{
					if (positionX < -15.0f)
						positionX += 0.0;
					else
						positionX -= 0.5f;

				}
				else if (keys[SDL_SCANCODE_RIGHT])
				{
					if (positionX > 15.0f)
						positionX += 0.0;
					else
						positionX += 0.5f;
				}
				else if (keys[SDL_SCANCODE_SPACE])
				{
//					new Bullet();
//					glBindTexture(GL_TEXTURE_2D, bullet);
//					glDrawArrays(GL_TRIANGLES, 0, 6);

				}
				break;
			}
		}
		switch (state)
		{
			case STATE_TITLE_SCREEN:
			{
				modelMatrix.identity();
				modelMatrix.Translate(-15.0f, 1.50f, 1.0f);
				modelMatrix.Scale(1, 1, 0);
				program.setModelMatrix(modelMatrix);

				DrawText(program, font1, "Press Space To Begin...", 1.5f, 0.001f);

				modelMatrix.identity();
				modelMatrix.Translate(-1.0f, -1.5f, 1.0f);
				modelMatrix.Scale(4.0, 4.0, 1);
				program.setModelMatrix(modelMatrix);
				float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				float texCoords[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				glUseProgram(program.programID);

				break;
			}
			case STATE_GAME:
			{
				glClear(GL_COLOR_BUFFER_BIT);
				modelMatrix.identity();
				modelMatrix.Translate(0.0f, -7.0f, 0.0f);
				modelMatrix.Scale(1.0f, 1.0f, 1.0f);
				modelMatrix.Translate(positionX, 0.0f, 0.0f);
				program.setModelMatrix(modelMatrix);

				float vertices[] = { -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				float texCoords[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);

				glBindTexture(GL_TEXTURE_2D, ship);
				glDrawArrays(GL_TRIANGLES, 0, 6);


				SheetSprite enemy0;
				float x = -10.0f;
				float y = 6.0f;
				float x_change = 6.0f;
				float y_change = 3.0f;
				for (int i = 0; i < 12; i += 1)
				{
					enemies.push_back(enemy0);
				}
				
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						DrawSheetSprite(program, modelMatrix, 0, enemy, x + j * x_change, y - i* y_change, 2.5f);
					}
				}
					

				for (int i = 0; i < enemies.size(); i++) {

					enemies[i].x += elapsed * 1.0f * enemies[i].direction;
					enemies[i].setAttribute(x, y);
					DrawSheetSprite(program, modelMatrix, 0, enemy, x, y, 2.5f);
					if (enemies[i].x > 15.0f || enemies[i].x < -15.0f) {
						for (int j = 0; j < enemies.size(); j++) {
							enemies[j].x += .01 * -1 * enemies[j].direction;
							enemies[j].direction *= -1;
							enemies[j].y -= 0.075f;
							enemies[j].setAttribute(x, y);
						}
					}
				}
				
			}
		}

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		SDL_GL_SwapWindow(displayWindow);

		glClear(GL_COLOR_BUFFER_BIT);
	}

	SDL_Quit();
	return 0;
}
