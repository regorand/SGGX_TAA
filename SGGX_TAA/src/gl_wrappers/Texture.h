#pragma once

#include <string>

class Texture
{
private:
	unsigned int m_GlId;
	std::string m_FilePath;
	int m_bytesPerPixel, m_width, m_height;
	unsigned char* m_Buf;


public:
	Texture(const std::string& path);
	~Texture();

	void Bind(unsigned int textureSlot = 0);
	void Unbind();


};

