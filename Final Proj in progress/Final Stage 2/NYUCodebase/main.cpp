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
	if (type == "Player" || type == "Enemy") {
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


void collisionx(Entity &entity, int num) {
	int gridX, gridY;
	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < num)
	{
		entity.position.x += entity.position.x + TILE_SIZE / 2 - gridX * TILE_SIZE - TILE_SIZE / 2;
		entity.velocity.x = 0;
		entity.acceleration.x = 0;
	}
}

void collisiony(Entity &entity, int num) {
	int gridX, gridY;
	float top = 0;
	if (entity.type == "Player") {
		top = TILE_SIZE;
	}

	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < num)
	{
		entity.position.y += -1 * (entity.position.y) - gridY * TILE_SIZE + 0.001;
		entity.velocity.y = 0;
		entity.collidedBottom = true;
	}

	worldToTileCoordinates(entity.position.x + TILE_SIZE / 2, entity.position.y + TILE_SIZE + top, &gridX, &gridY);
	if (!(gridX < 0 || gridX > mapWidth || gridY < 0 || gridY > mapHeight) && levelData[gridY][gridX] < num)
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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 720, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	float lastFrameTicks = 0.0f;

	GLuint sheetSprite = LoadTexture("spritesheet.png");
	Mix_Chunk *jump;
	jump = Mix_LoadWAV("jump.wav");
	Mix_Chunk *pickup;
	pickup = Mix_LoadWAV("pickup.wav");
	Mix_Music *music;
	music = Mix_LoadMUS("song2.mp3");
	std::ifstream infile("mymap2new.txt");
	std::string line;
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				return 0;
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[Entity]") {
			readEntityData(infile);
		}
	}
	Mix_PlayMusic(music, -1);
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);


	float p1vy = 0;
	float p1ax = 0;
	int jewelCount = 4;

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

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						p1ax = 3;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						p1ax = -3;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 4;
						Mix_PlayChannel(-1, jump, 0);
					}
				}
				else if (event.type == SDL_KEYUP) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						p1ax = 0;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						p1vy = 0;
					}
				}
			}


			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].type == "Player") {
					entities[i].acceleration.x = p1ax;
					if (entities[i].collidedBottom) {
						entities[i].velocity.y = p1vy;
					}
				}
				if (entities[i].type == "Enemy") {
					entities[i].acceleration.x = -2;
				}
				if (!entities[i].isStatic) {
					entities[i].acceleration.y = -5;
				}
				entities[i].collidedTop = false;
				entities[i].collidedBottom = false;
				entities[i].collidedLeft = false;
				entities[i].collidedRight = false;

				entities[i].velocity.x = lerp(entities[i].velocity.x, 0.0f, FIXED_TIMESTEP * 5);
				entities[i].velocity.y = lerp(entities[i].velocity.y, 0.0f, FIXED_TIMESTEP * 1);

				entities[i].velocity.x += entities[i].acceleration.x * FIXED_TIMESTEP;
				entities[i].velocity.y += entities[i].acceleration.y * FIXED_TIMESTEP;

				entities[i].position.x += entities[i].velocity.x * FIXED_TIMESTEP;
				if (!entities[i].isStatic) {
					collisionx(entities[i], 193);
				}
				entities[i].position.y += entities[i].velocity.y * FIXED_TIMESTEP;
				if (!entities[i].isStatic) {
					collisiony(entities[i], 193);
				}
			}

			for (int i = 0; i < entities.size(); ++i) {
				if (entities[i].type == "Player") {
					for (int j = 0; j < entities.size(); ++j) {
						if (entities[j].type == "Jewel" && entityCollision(entities[i], entities[j])) {
							entities.erase(entities.begin() + j);
							jewelCount--;
							Mix_PlayChannel(-1, pickup, 0);
							if (jewelCount == 0) {
								done = true;
							}
							break;
						}
						if (entities[j].type == "Enemy" && entityCollision(entities[i], entities[j])) {
							done = true;
						}
					}
					break;
				}
			}


			glClear(GL_COLOR_BUFFER_BIT);
			for (int y = 0; y < mapHeight; ++y) {
				for (int x = 0; x < mapWidth; ++x) {
					modelMatrix.identity();
					modelMatrix.Translate(x*TILE_SIZE, (mapHeight - y - 1)*TILE_SIZE, 0);
					program.setModelMatrix(modelMatrix);
					program.setProjectionMatrix(projectionMatrix);
					program.setViewMatrix(viewMatrix);
					DrawSpriteSheetSprite(program, levelData[y][x], 30, 30, sheetSprite);
				}
			}
			for (int i = 0; i < entities.size(); ++i) {
				modelMatrix.identity();
				modelMatrix.Translate(entities[i].position.x, entities[i].position.y + mapHeight*TILE_SIZE, 0);

				program.setModelMatrix(modelMatrix);
				program.setProjectionMatrix(projectionMatrix);
				program.setViewMatrix(viewMatrix);
				if (entities[i].type == "Player") {
					DrawSpriteSheetSprite(program, 27, 30, 30, sheetSprite);

					modelMatrix.identity();
					modelMatrix.Translate(entities[i].position.x, entities[i].position.y + mapHeight*TILE_SIZE + TILE_SIZE, 0);

					viewMatrix.identity();
					viewMatrix.Translate(-entities[i].position.x, -1 * (entities[i].position.y + mapHeight*TILE_SIZE), 0);
					if (entities[i].position.y < -5.5) {
						done = true;
					}
				}
				if (entities[i].type == "Jewel")
					DrawSpriteSheetSprite(program, 379, 30, 30, sheetSprite);
				if (entities[i].type == "Enemy") {
					DrawSpriteSheetSprite(program, 269, 30, 30, sheetSprite);
//					if (entities[i].position.y < -5.5) {
//						entities[i].position.y += 0.5;
//						entities[i].velocity.x = -entities[i].velocity.x;
//						entities[i].acceleration.x = -entities[i].acceleration.x;
//					}
				}
				
			}
			SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
