#include "Entity.h"


	Entity::Entity(float x, float y, float h, float w) : posX(x), posY(y), height(h), width(w), alive(true) {}

float Entity::getX() const
{
	return posX;
}

float Entity::getY() const
{
	return posY;
}

float Entity::getHeight() const
{
	return height;
}

float Entity::getWidth() const
{
	return width;
}

bool Entity::isAlive() const
{
	return alive;
}

void Entity::dies()
{
	alive = false;
}

bool Entity::collidesWith(float x, float y)
{
	if (x >= (posX - width / 2.0) && x <= (posX + width / 2.0) && y >= (posY - height / 2.0) && y <= (posY + height / 2.0))
	{
		return true;
	}
}

void Entity::draw(GLuint textureID, ShaderProgram& program, Matrix& modelMatrix)
{
	if (alive == true)
	{
		float vertices[] =
		{
			posX - width / 2.0, posY - height / 2.0,
			posX + width / 2.0, posY + height / 2.0,
			posX - width / 2.0, posY + height / 2.0,
			posX - width / 2.0, posY - height / 2.0,
			posX + width / 2.0, posY - height / 2.0,
			posX + width / 2.0, posY + height / 2.0
		};

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords[] = { 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		modelMatrix.identity();
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}

}