#include "framebuffer.h"


Framebuffer::Framebuffer() {

}
Framebuffer::Framebuffer(unsigned int width, unsigned int height) {

    mWidth = width;
    mHeight = height;

    // 1 Create FBO
    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    // 2 Create color attachment
    mColorAttachment = Texture::createColorAttachment(mWidth, mHeight, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment->getTexture(), 0);

    // 3 Create depth and stencil attchment 
    mDepthStencilAttachment = Texture::createDepthStencilAttachment(mWidth, mHeight, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilAttachment->getTexture(), 0);

    // 4 Check error
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error:FrameBuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

Framebuffer* Framebuffer::createGBufferFbo(unsigned int width, unsigned int height) {

    Framebuffer* fb = new Framebuffer();

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    auto positionTex = Texture::createGBufferAttachment(width, height, 0, GL_RGBA16F, GL_FLOAT);
    auto normalTex = Texture::createGBufferAttachment(width, height, 0, GL_RGBA16F, GL_FLOAT);
    auto albedoSpecTex = Texture::createGBufferAttachment(width, height, 0, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTex->getTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex->getTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedoSpecTex->getTexture(), 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // 2 Depth attachment
    auto depthTex = Texture::createDepthAttachment(width, height, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex->getTexture(), 0);


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

        std::cout << "Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fb->mFBO = fbo;
    fb->mGBufferAttachment.push_back(positionTex);
    fb->mGBufferAttachment.push_back(normalTex);
    fb->mGBufferAttachment.push_back(albedoSpecTex);
    fb->mDepthAttachment = depthTex;
    fb->mWidth = width;
    fb->mHeight = height;

    return fb;
}


Framebuffer::~Framebuffer() {

	if (mFBO){

		glDeleteFramebuffers(1, &mFBO);

	}
	if (mColorAttachment != nullptr){

		delete mColorAttachment;
	}
	if (mDepthStencilAttachment != nullptr) {

		delete mDepthStencilAttachment;
	}
}