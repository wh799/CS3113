#include "SheetSprite.h"

SheetSprite::SheetSprite() {};
SheetSprite::SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : 
	textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
}