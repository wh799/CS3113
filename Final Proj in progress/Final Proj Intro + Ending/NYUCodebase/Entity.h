#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>


	class Entity
{
public:
	Entity(float x, float y, float h, float w);

	float getX() const;
	float getY() const;
	float getHeight() const;
	float getWidth() const;
	bool collidesWith(float x, float y);
	bool isAlive() const;
	void dies();
	void draw(GLuint textureID, ShaderProgram& program, Matrix& modelMatrix);

private:
	float posX;
	float posY;
	bool alive;
	float height;
	float width;
};
