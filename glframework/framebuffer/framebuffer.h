#pragma once

#include "../core.h"
#include "../texture.h"

class Framebuffer {

public:
	Framebuffer();
	Framebuffer(unsigned int width, unsigned int height);
	~Framebuffer();
	static Framebuffer* createGBufferFbo(unsigned int width, unsigned int height);

public:

	unsigned int mWidth{ 0 };
	unsigned int mHeight{ 0 };

	unsigned int mFBO{ 0 };
	Texture* mColorAttachment{ nullptr };
	Texture* mDepthStencilAttachment{ nullptr };
	Texture* mDepthAttachment{ nullptr };

	std::vector<Texture*> mGBufferAttachment{};
};