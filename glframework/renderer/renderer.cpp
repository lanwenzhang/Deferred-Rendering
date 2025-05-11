#include "renderer.h"
#include "../material/screenMaterial.h"
#include "../material/cubeMaterial.h"
#include "../material/advanced/pbrMaterial.h"
#include "../material/phongMaterial.h"
#include "../material/gBufferMaterial.h"
#include "../material/lightingMaterial.h"
#include "../../application/camera/orthographicCamera.h"

Renderer::Renderer() {

	mScreenShader = new Shader("assets/shaders/screen.vert", "assets/shaders/screen.frag");
	mGBufferShader = new Shader("assets/shaders/gBuffer.vert", "assets/shaders/gBuffer.frag");
	mLightingShader = new Shader("assets/shaders/lighting.vert", "assets/shaders/lighting.frag");
}

Renderer::~Renderer() {

}

void Renderer::setClearColor(glm::vec3 color) {

	glClearColor(color.r, color.g, color.b, 1.0);
}

void Renderer::setDepthState(Material* material) {

	if (material->mDepthTest) {

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(material->mDepthFunc);
	}
	else {

		glDisable(GL_DEPTH_TEST);
	}

	// 2.2 Depth write
	if (material->mDepthWrite) {

		glDepthMask(GL_TRUE);
	}
	else {

		glDepthMask(GL_FALSE);
	}

}

void Renderer::setPolygonOffsetState(Material* material) {

	if (material->mPolygonOffset) {

		glEnable(material->mPolygonOffsetType);
		glPolygonOffset(material->mFactor, material->mUnit);

	}
	else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);

	}

}

void Renderer::setStencilState(Material* material) {

	if (material->mStencilTest) {

		glEnable(GL_STENCIL_TEST);

		glStencilOp(material->mSFail, material->mZFail, material->mZPass);
		glStencilMask(material->mStencilMask);
		glStencilFunc(material->mStencilFunc, material->mStencilRef, material->mStencilFuncMask);

	}
	else {

		glDisable(GL_STENCIL_TEST);
	}
}

void Renderer::setBlendState(Material* material) {

	if (material->mBlend) {

		glEnable(GL_BLEND);
		glBlendFunc(material->mSFactor, material->mDFactor);
	}
	else {

		glDisable(GL_BLEND);
	}
}

void Renderer::setFaceCullingState(Material* material) {

	if (material->mFaceCulling) {

		glEnable(GL_CULL_FACE);
		glFrontFace(material->mFrontFace);
		glCullFace(material->mCullFace);

	}
	else {

		glDisable(GL_CULL_FACE);
	}
}

void Renderer::projectObject(Object* obj) {

	if (obj->getType() == ObjectType::Mesh) {

		Mesh* mesh = (Mesh*)obj;
		auto material = mesh->mMaterial;
		if (material->mBlend) {

			mTransparentObjects.push_back(mesh);
		}
		else {

			mOpacityObjects.push_back(mesh);
		}
	}

	auto children = obj->getChildren();
	for (int i = 0; i < children.size(); i++) {

		projectObject(children[i]);
	}
}

Shader* Renderer::pickShader(MaterialType type) {

	Shader* result = nullptr;

	switch (type) {

	case MaterialType::ScreenMaterial:
		result = mScreenShader;
		break;
	case MaterialType::CubeMaterial:
		result = mCubeShader;
		break;
	case MaterialType::PhongMaterial:
		result = mPhongShader;
		break;
	default:
		std::cout << "Unkown material type to shader" << std::endl;
		break;
	}

	return result;
}


void Renderer::renderGBuffer(Scene* scene, Camera* camera, unsigned int fbo) {

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_FRAMEBUFFER_SRGB);
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	for (auto mesh : mOpacityObjects) {

		auto geometry = mesh->mGeometry;
		GBufferMaterial* gBufferMat = (GBufferMaterial*)mesh->mMaterial;
		Shader* shader = mGBufferShader;
		shader->begin();

		// MVP
		shader->setMatrix4x4("modelMatrix", mesh->getModelMatrix());
		shader->setMatrix4x4("viewMatrix", camera->getViewMatrix());
		shader->setMatrix4x4("projectionMatrix", camera->getProjectionMatrix());

		// Normal
		auto normalMatrix = glm::mat3(glm::transpose(glm::inverse(mesh->getModelMatrix())));
		shader->setMatrix3x3("normalMatrix", normalMatrix);
		
		// Diffuse + Specular
		shader->setInt("diffuse", 0);
		gBufferMat->mDiffuse->setUnit(0);
		gBufferMat->mDiffuse->bind();

		shader->setInt("specular", 1);
		gBufferMat->mSpecularMask->setUnit(1);
		gBufferMat->mSpecularMask->bind();

		shader->setInt("normalTex", 2);
		gBufferMat->mNormal->setUnit(2);
		gBufferMat->mNormal->bind();


		// Camera
		shader->setVector3("cameraPosition", camera->mPosition);

		// VAO
		glBindVertexArray(geometry->getVao());

		// 3.4 Draw
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
		shader->end();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderLighting(Mesh* mesh, Camera* camera, std::vector<PointLight*> pointLights) {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_FRAMEBUFFER_SRGB);
	auto geometry = mesh->mGeometry;
	LightingMaterial* lightingMat = (LightingMaterial*)mesh->mMaterial;
	Shader* shader = mLightingShader;
	shader->begin();

	shader->setInt("gPosition", 0);
	lightingMat->mPosition->setUnit(0);
	lightingMat->mPosition->bind();

	shader->setInt("gNormal", 1);
	lightingMat->mNormal->setUnit(1);
	lightingMat->mNormal->bind();

	shader->setInt("gAlbedoSpec", 2);
	lightingMat->mAlbedoSpec->setUnit(2);
	lightingMat->mAlbedoSpec->bind();

	shader->setVector3("cameraPosition", camera->mPosition);

	for (int i = 0; i < pointLights.size(); i++) {

		shader->setVector3("pointLights[" + std::to_string(i) + "].color", pointLights[i]->mColor);
		shader->setVector3("pointLights[" + std::to_string(i) + "].position", pointLights[i]->getPosition());
	}

	glBindVertexArray(geometry->getVao());

	// 3.4 Draw
	glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
	shader->end();

}

