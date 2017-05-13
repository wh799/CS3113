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
#include <fstream> 
#include <string>
#include <iostream> 
#include <sstream> 
#include <SDL_mixer.h>

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "assignment1.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


class Vector3 {
public:
	Vector3() { Vector3(0, 0, 0); }
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;

};

class SheetSprite {
public:
	SheetSprite() { SheetSprite(0.0f, 0); }
	SheetSprite(float size, unsigned int textureID, float u = 0, float v = 0, float width = 0, float height = 0) : size(size), textureID(textureID), u(u), v(v), width(width), height(height) {}

	void Draw(ShaderProgram program) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, 0.5f * size,
			0.5f * size * aspect, 0.5f * size,
			-0.5f * size * aspect, -0.5f * size ,
			0.5f * size * aspect, -0.5f * size };
		// draw our arrays
		glBindTexture(GL_TEXTURE_2D, textureID);
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

	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

class Entity {
public:
	Entity(std::string type, bool isStatic, Vector3 position, Vector3 velocity = Vector3(0, 0, 0), Vector3 acceleration = Vector3(0, 0, 0)) :
		type(type), isStatic(isStatic), position(position), velocity(velocity), acceleration(acceleration), collidedTop(false), collidedBottom(false), collidedLeft(false), collidedRight(false) {}
	std::string type;
	bool isStatic;
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float rotation;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
	SheetSprite sprite;
};

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

float TILE_SIZE = .2;

void DrawSpriteSheetSprite(ShaderProgram program, int index, int spriteCountX, int spriteCountY, GLuint textureID) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;

	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};

	float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE, -0.5f*TILE_SIZE, 0.5f*TILE_SIZE, -0.5f*TILE_SIZE };

	glBindTexture(GL_TEXTURE_2D, textureID);
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

int mapWidth;
int mapHeight;
unsigned char** levelData;

bool readHeader(std::ifstream &stream) {
	string line;
	mapWidth = -1;
	mapHeight = -1;
	while
		(getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			mapHeight = atoi(value.c_str());
		}
	}
	if
		(mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for
			(int i =
				0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char
				[mapWidth];
		}
		return true;
	}
}

bool readLayerData(std::ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

std::vector<Entity> entities;

void placeEntity(std::string type, float x, float y) {
	bool isStatic = true;
	if (type == "Player") {
		isStatic = false;
	}
	Entity entity(type, isStatic, Vector3(x, y, 0));
	entities.push_back(entity);
}

bool readEntityData(std::ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY);
		}
	}
	return true;
}


float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-(worldY) / TILE_SIZE);
}


void collisionx(Entity &entity) {
	int gridX, gridY;
	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 100)
	{
		entity.position.x += entity.position.x + TILE_SIZE / 2 - gridX * TILE_SIZE - TILE_SIZE / 2;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
	}
}

void collisiony(Entity &entity) {
	int gridX, gridY;
	float top = 0;
	if (entity.type == "Player") {
		top = TILE_SIZE;
	}

	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 100)
	{
		entity.position.y += -1 * (entity.position.y) - gridY * TILE_SIZE + 0.001;
		entity.velocity.y = 0;
		entity.collidedBottom = true;
	}

	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < 100)
	{
		entity.position.y += -1 * (entity.position.y + TILE_SIZE + top) - gridY * TILE_SIZE - TILE_SIZE - 0.001;
		entity.velocity.y = 0;
		entity.collidedTop = true;
	}
}

bool entityCollision(Entity e1, Entity e2) {
	int top = 0;
	if (e1.type == "Player") {
		top = TILE_SIZE;
	}
	return (!(
		e1.position.x - TILE_SIZE / 2 > e2.position.x + TILE_SIZE / 2 ||
		e1.position.x + TILE_SIZE / 2 < e2.position.x - TILE_SIZE / 2 ||
		e1.position.y - TILE_SIZE / 2 > e2.position.y + TILE_SIZE / 2 ||
		e1.position.y + TILE_SIZE / 2 + top < e2.position.y - TILE_SIZE / 2));
}

void readMap(string filename) {
	std::ifstream infile(filename);
	std::string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Entity]") {
			readEntityData(infile);
		}
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif


	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	float lastFrameTicks = 0.0f;

	GLuint sheetSprite = LoadTexture("spritesheet.png");
	GLuint font1 = LoadTexture("font1.png");
	GLuint yoshi = LoadTexture("yoshi.png");
	Mix_Chunk *jump;
	jump = Mix_LoadWAV("jump.wav");
	Mix_Chunk *pickup;
	pickup = Mix_LoadWAV("pickup.wav");
	Mix_Music *music;
	music = Mix_LoadMUS("startMusic.mp3");
	float screenShakeValue = 0.0;
	float screenShakeSpeed = 5.0f;
	float screenShakeIntensity = .070f;


	Mix_PlayMusic(music, -1);
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-16.0, 16.0, -9.0, 9.0, -1.0, 1.0);
	glUseProgram(program.programID);

	enum GameState { STATE_TITLE_SCREEN, STATE_LEVEL_ONE, STATE_LEVEL_TWO, STATE_LEVEL_THREE, STATE_FINAL };
	GameState state = STATE_TITLE_SCREEN;


	float p1vy = 0;
	float p1ax = 0;
	bool gameDone = false;
	bool winGame = false;
	int jewelCount;

	SDL_Event event;
	bool done = false;
	while (!done) {

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		// 60 FPS (1.0f/60.0f)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
		float fixedElapsed = elapsed;
		if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
			fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
		}
		while (fixedElapsed >= FIXED_TIMESTEP) {
			fixedElapsed -= FIXED_TIMESTEP;
			//Update(FIXED_TIMESTEP);
		}
		//Update(fixedElapsed);
		screenShakeValue += elapsed;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

			const Uint8* keys = SDL_GetKeyboardState(NULL);

			switch (state)
			{
				case STATE_TITLE_SCREEN:
				{
					if (keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_RETURN2])
					{
						state = STATE_LEVEL_ONE;
						state = STATE_FINAL;
						Mix_PlayMusic(music, -1);
					}

					if (keys[SDL_SCANCODE_ESCAPE])
					{
						done = true;
					}
					break;
				}
				case STATE_LEVEL_ONE:
				{
					
				}
				case STATE_LEVEL_TWO:
				{
					
				}
				case STATE_LEVEL_THREE:
				{
					
				}
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		switch (state)
		{
			case STATE_TITLE_SCREEN:
			{
				if (screenShakeValue < 6.1f)
				{
					viewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
				}
				else
					viewMatrix.identity();

				modelMatrix.identity();
				modelMatrix.Translate(-2.0f, 7.0f, 1.0f);
				program.setModelMatrix(modelMatrix);
				DrawText(program, font1, "More Jewels", 2.0f, -0.5f);
				modelMatrix.identity();
				modelMatrix.Translate(-1.8f, 4.0f, 1.0f);
				program.setModelMatrix(modelMatrix);
				DrawText(program, font1, "Objective", 1.6f, -0.09f);
				modelMatrix.identity();
				modelMatrix.Translate(-1.8f, 2.0f, 1.0f);
				program.setModelMatrix(modelMatrix);
				DrawText(program, font1, "Get Jewels", 1.6f, -0.09f);

				modelMatrix.identity();
				modelMatrix.Translate(-4.5f, -3.50f, 1.0f);
				modelMatrix.Scale(.65, .65, 1);
				program.setModelMatrix(modelMatrix);
				DrawText(program, font1, "Press Enter To Start", 1.6f, -0.09f);

				modelMatrix.identity();
				modelMatrix.Translate(-4.50f, -5.50f, 1.0f);
				modelMatrix.Scale(.65, .65, 1);
				program.setModelMatrix(modelMatrix);
				DrawText(program, font1, "Press ESC To Exit", 1.6f, -0.001f);

				modelMatrix.identity();
				program.setModelMatrix(modelMatrix);
				float vertices[] = { -3.0f, -3.0f, 3.0f, 3.0f, -3.0f, 3.0f, 3.0f, 3.0f, -3.0f, -3.0f, 3.0f, -3.0f };
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				float texCoords[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				glUseProgram(program.programID);

				break;
			}
			case STATE_LEVEL_ONE:
			{
				
			}
			case STATE_LEVEL_TWO:
			{
				
			}
			case STATE_LEVEL_THREE:
			{
			
			}
			case STATE_FINAL: 
			{
				if (gameDone == true) {
					modelMatrix.identity();
					modelMatrix.Translate(-4.5f, 7.0f, 1.0f);
					modelMatrix.Scale(.65, .65, 1);
					program.setModelMatrix(modelMatrix);
					DrawText(program, font1, "Congratulations!!", 2.0f, -0.09f);
				}
				else
				{
					modelMatrix.identity();
					modelMatrix.Translate(-4.5f, 7.0f, 1.0f);
					modelMatrix.Scale(.65, .65, 1);
					program.setModelMatrix(modelMatrix);
					DrawText(program, font1, "Game Over!", 3.0f, -0.09f);
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
