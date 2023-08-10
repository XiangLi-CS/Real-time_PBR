#include "Renderer.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

Vector3 lightPositions = Vector3(-2.0f, 0.0f, 0.0f);
Vector4 lightColors = Vector4(5.0f, 5.0f, 5.0f, 0.5f);		//Range = 0-255

Vector3 lightDirection = Vector3(-0.8574929257125442f, -0.5144957554275266f, 0.0f);
Vector4 lightDirColors = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

Vector4 lightData = Vector4(5.0f, 0.09f, 0.03f, 0.0f);

Renderer::Renderer(Window& parent) :OGLRenderer(parent)
{
	//Load Meshes
	quad = Mesh::GenerateQuad();	//Environment
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	cube = Mesh::GenerateCube();

	helmet = Mesh::LoadFromMeshFile("model.msh");
	//helmet = Mesh::LoadFromMeshFile("Helment.msh");
	//helmet = Mesh::LoadFromMeshFile("Samurai_low.msh");



	//Load Shaders
	unlitShader = new Shader("unlitVertex.glsl", "unlitFragment.glsl");

	pbrShader = new Shader("pbrVertex.glsl", "pbrFragment.glsl");

	equirectangularToCubemapShader = new Shader("pbr_cubemapVertex.glsl", "equirectangular_to_cubemapFragment.glsl");

	skyboxShader = new Shader("pbr_skyboxVertex.glsl", "pbr_skyboxFragment.glsl");

	irradianceShader = new Shader("pbr_cubemapVertex.glsl", "irradiance_convolutionFragment.glsl");

	prefilterShader = new Shader("pbr_cubemapVertex.glsl", "prefilteringFragment.glsl");

	brdfShader = new Shader("brdfVertex.glsl", "brdfFragment.glsl");

	blinnPhongShader = new Shader("pbrVertex.glsl", "blinn_phongFragment.glsl");

	pbrNoIBLShader = new Shader("pbrVertex.glsl", "pbr_NO_IBL_Fragment.glsl");

	if (!pbrShader->LoadSuccess() || !unlitShader->LoadSuccess() || !equirectangularToCubemapShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !irradianceShader->LoadSuccess() || !prefilterShader->LoadSuccess() || !brdfShader->LoadSuccess() || !blinnPhongShader->LoadSuccess() || !pbrNoIBLShader->LoadSuccess())
	{
		return;
	}

	//Load Textures
	//Different skybox
	hdrTex = loadHdrTexture(TEXTUREDIR"brown_photostudio_02_2k.hdr");
	//hdrTex = loadHdrTexture(TEXTUREDIR"newport_loft.hdr");
	//hdrTex = loadHdrTexture(TEXTUREDIR"industrial_sunset_puresky_2k.hdr");
	

	//Different model data
	//model.msh
	albedo = loadTexture(TEXTUREDIR"lambert1_albedo.jpg");
	normal = loadTexture(TEXTUREDIR"lambert1_normal.png");
	metallic = loadTexture(TEXTUREDIR"lambert1_metallic.jpg");
	roughness = loadTexture(TEXTUREDIR"lambert1_roughness.jpg");
	ao = loadTexture(TEXTUREDIR"lambert1_ao.jpg");

	//Helment.msh
	/*albedo = loadTexture(TEXTUREDIR"dd2_Turban_BaseColor.png");
	normal = loadTexture(TEXTUREDIR"dd2_Turban_Normal.png");
	metallic = loadTexture(TEXTUREDIR"dd2_Turban_Metallic.png");
	roughness = loadTexture(TEXTUREDIR"dd2_Turban_Roughness.png");
	ao = loadTexture(TEXTUREDIR"internal_ground_ao_texture.jpeg");*/

	//Samurai_low.msh
	/*albedo = loadTexture(TEXTUREDIR"Samurai_low_defaultMat_Color.png");
	normal = loadTexture(TEXTUREDIR"Samurai_low_defaultMat_Normal_OpenGL.png");
	metallic = loadTexture(TEXTUREDIR"Samurai_low_defaultMat_Metallic.png");
	roughness = loadTexture(TEXTUREDIR"Samurai_low_defaultMat_Roughness.png");
	ao = loadTexture(TEXTUREDIR"internal_ground_ao_texture.jpeg");*/

	//another texture
	/*albedo = loadTexture(TEXTUREDIR"plastic_albedo.png");
	normal = loadTexture(TEXTUREDIR"plastic_normal.png");
	metallic = loadTexture(TEXTUREDIR"plastic_metallic.png");
	roughness = loadTexture(TEXTUREDIR"plastic_roughness.png");
	ao = loadTexture(TEXTUREDIR"plastic_ao.png");*/

	//new
	if (!albedo || !normal || !metallic || !roughness || !ao || !hdrTex)
	{
		return;
	}

	//Camera
	camera = new Camera(0, 270, Vector3());
	camera->SetPosition(Vector3(-3.2,0,0.2));

	//Setup Framebuffer
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Setup capture data
	captureProjection = Matrix4::Perspective(0.1f, 10.0f, 1.0f, 90.0f);

	captureViews[0] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
	captureViews[1] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
	captureViews[2] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
	captureViews[3] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f));
	captureViews[4] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, -1.0f, 0.0f));
	captureViews[5] = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f));

	//Enable Functions
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


	init = true;

}

Renderer::~Renderer(void)
{
	//Delete Meshes
	delete quad;
	delete sphere;
	delete cube;
	delete helmet;
	
	//Delete Others
	delete camera;

	//Delete Shaders
	delete pbrShader;
	delete unlitShader;
	delete equirectangularToCubemapShader;
	delete skyboxShader;
	delete irradianceShader;
	delete prefilterShader;
	delete brdfShader;
	delete blinnPhongShader;
	delete pbrNoIBLShader;

	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	glDeleteTextures(1, &envCubemap);
	glDeleteTextures(1, &irradianceCubemap);
	glDeleteTextures(1, &prefilterCubemap);
}

//Camera
void Renderer::UpdateScene(float dt)
{
	elaspedTime += dt;

	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}


//Render Scene
void Renderer::RenderScene()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	if (!hasCapturedEnvCubemap)
	{
		hasCapturedEnvCubemap = true;
		DrawEnvironmentCubemap();
	}
	if (!hasCapturedIrrCubemap)
	{
		hasCapturedIrrCubemap = true;
		DrawIrradianceCubemap();
	}
	if (!hasCapturedPrefilterCubemap)
	{
		hasCapturedPrefilterCubemap = true;
		DrawPrefilterCubemap();
	}
	if (!hasCapturedbrdfLUTmap)
	{
		hasCapturedbrdfLUTmap = true;
		DrawBRDFLUTmap();
	}

	//DrawBall();

	DrawModel();

	DrawLight();
	
	DrawSkyBox();

}

void Renderer::RenderBlinnPhongScene()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	if (!hasCapturedEnvCubemap)
	{
		hasCapturedEnvCubemap = true;
		DrawEnvironmentCubemap();
	}

	DrawBlinnPhongModel();

	DrawLight();

	DrawSkyBox();

}

void Renderer::RenderNoIBLScene() 
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	if (!hasCapturedEnvCubemap)
	{
		hasCapturedEnvCubemap = true;
		DrawEnvironmentCubemap();
	}

	if (!hasCapturedIrrCubemap)
	{
		hasCapturedIrrCubemap = true;
		DrawIrradianceCubemap();
	}

	if (!hasCapturedPrefilterCubemap)
	{
		hasCapturedPrefilterCubemap = true;
		DrawPrefilterCubemap();
	}

	if (!hasCapturedbrdfLUTmap)
	{
		hasCapturedbrdfLUTmap = true;
		DrawBRDFLUTmap();
	}

	DrawNoIBLModel();

	DrawLight();

	DrawSkyBox();

}

void Renderer::DrawBall() 
{
	pbrShader->Bind();

	pbrShader->SetVector3("cameraPos", camera->GetPosition());

	//Render light (point light)
	pbrShader->SetVector3("lightPos", lightPositions);
	pbrShader->SetVector4("lightColor", lightColors);

	//Render light (directional light)
	pbrShader->SetVector3("lightDir", lightDirection);
	pbrShader->SetVector4("lightDirColor", lightDirColors);

	pbrShader->SetTexture("albedoMap", albedo, 0);
	pbrShader->SetTexture("normalMap", normal, 1);
	pbrShader->SetTexture("metallicMap", metallic, 2);
	pbrShader->SetTexture("roughnessMap", roughness, 3);
	pbrShader->SetTexture("aoMap", ao, 4);

	//IBL
	pbrShader->SetTextureCubeMap("irradianceMap", irradianceCubemap, 5);
	pbrShader->SetTextureCubeMap("prefilterMap", prefilterCubemap, 6);
	pbrShader->SetTexture("brdfLUT", brdfLUTmap, 7);

	Vector3 hSize = Vector3(0,0,0);	
	 
	modelMatrix = Matrix4::Translation(hSize) *
		Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f));

	pbrShader->SetMat4("projMatrix", projMatrix);
	pbrShader->SetMat4("modelMatrix", modelMatrix);
	pbrShader->SetMat4("viewMatrix", viewMatrix);

	sphere->Draw();

}

void Renderer::DrawModel() {
	pbrShader->Bind();

	pbrShader->SetVector3("cameraPos", camera->GetPosition());

	//Render light (point light)
	pbrShader->SetVector3("lightPos", lightPositions);
	pbrShader->SetVector4("lightColor", lightColors);

	//Render light (directional light)
	pbrShader->SetVector3("lightDir", lightDirection);
	pbrShader->SetVector4("lightDirColor", lightDirColors);

	pbrShader->SetTexture("albedoMap", albedo, 0);
	pbrShader->SetTexture("normalMap", normal, 1);
	pbrShader->SetTexture("metallicMap", metallic, 2);
	pbrShader->SetTexture("roughnessMap", roughness, 3);
	pbrShader->SetTexture("aoMap", ao, 4);

	//IBL
	pbrShader->SetTextureCubeMap("irradianceMap", irradianceCubemap, 5);
	pbrShader->SetTextureCubeMap("prefilterMap", prefilterCubemap, 6);
	pbrShader->SetTexture("brdfLUT", brdfLUTmap, 7);

	Vector3 hSize = Vector3(0, 0, 0);		

	modelMatrix = Matrix4::Translation(hSize) *
		Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f)) *
		Matrix4::Rotation(-90, Vector3(0, 1, 0));

	pbrShader->SetMat4("projMatrix", projMatrix);
	pbrShader->SetMat4("modelMatrix", modelMatrix);
	pbrShader->SetMat4("viewMatrix", viewMatrix);

	helmet->Draw();
}

void Renderer::DrawBlinnPhongModel() {
	blinnPhongShader->Bind();

	blinnPhongShader->SetVector3("cameraPos", camera->GetPosition());

	//Render light (point light)
	blinnPhongShader->SetVector3("lightPos", lightPositions);
	blinnPhongShader->SetVector4("lightColor", lightColors);
	blinnPhongShader->SetVector4("lightData", lightData);

	//Render light (directional light)
	blinnPhongShader->SetVector3("lightDir", lightDirection);
	blinnPhongShader->SetVector4("lightDirColor", lightDirColors);

	blinnPhongShader->SetTexture("albedoMap", albedo, 0);
	blinnPhongShader->SetTexture("normalMap", normal, 1);

	Vector3 hSize = Vector3(0, 0, 0);		

	modelMatrix = Matrix4::Translation(hSize) *
		Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f)) *
		Matrix4::Rotation(-90, Vector3(0, 1, 0));

	blinnPhongShader->SetMat4("projMatrix", projMatrix);
	blinnPhongShader->SetMat4("modelMatrix", modelMatrix);
	blinnPhongShader->SetMat4("viewMatrix", viewMatrix);

	helmet->Draw();
}

void Renderer::DrawNoIBLModel() {
	pbrNoIBLShader->Bind();

	pbrNoIBLShader->SetVector3("cameraPos", camera->GetPosition());

	//Render light (point light)
	pbrNoIBLShader->SetVector3("lightPos", lightPositions);
	pbrNoIBLShader->SetVector4("lightColor", lightColors);

	//Render light (directional light)
	pbrNoIBLShader->SetVector3("lightDir", lightDirection);
	pbrNoIBLShader->SetVector4("lightDirColor", lightDirColors);

	pbrNoIBLShader->SetTexture("albedoMap", albedo, 0);
	pbrNoIBLShader->SetTexture("normalMap", normal, 1);
	pbrNoIBLShader->SetTexture("metallicMap", metallic, 2);
	pbrNoIBLShader->SetTexture("roughnessMap", roughness, 3);
	pbrNoIBLShader->SetTexture("aoMap", ao, 4);

	Vector3 hSize = Vector3(0, 0, 0);		

	modelMatrix = Matrix4::Translation(hSize) *
		Matrix4::Scale(Vector3(1.0f, 1.0f, 1.0f)) *
		Matrix4::Rotation(-90, Vector3(0, 1, 0));

	pbrNoIBLShader->SetMat4("projMatrix", projMatrix);
	pbrNoIBLShader->SetMat4("modelMatrix", modelMatrix);
	pbrNoIBLShader->SetMat4("viewMatrix", viewMatrix);

	helmet->Draw();
}

void Renderer::DrawLight() {
	unlitShader->Bind();

	lightPositions.x = std::sin(elaspedTime * 1.0f) * 2.0f;
	lightPositions.z = std::cos(elaspedTime * 1.0f) * 2.0f;

	unlitShader->SetVector4("lightColor", lightColors);

	modelMatrix = Matrix4::Translation(lightPositions) *
		Matrix4::Scale(Vector3(0.1f, 0.1f, 0.1f));
		//Matrix4::Rotation(90, Vector3(1, 0, 0));

	unlitShader->SetMat4("projMatrix", projMatrix);
	unlitShader->SetMat4("modelMatrix", modelMatrix);
	unlitShader->SetMat4("viewMatrix", viewMatrix);

	sphere->Draw();
}

void Renderer::DrawEnvironmentCubemap() {
	//Setup Environment Cubemap
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	equirectangularToCubemapShader->Bind();

	equirectangularToCubemapShader->SetTexture("equirectangularMap", hdrTex, 0);
	equirectangularToCubemapShader->SetMat4("projMatrix", captureProjection);

	glViewport(0, 0, 512, 512); 
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; i++) {
		equirectangularToCubemapShader->SetMat4("viewMatrix", captureViews[i]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube->Draw();

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Renderer::DrawIrradianceCubemap() {
	//Setup irradiance cupbemap
	glGenTextures(1, &irradianceCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	irradianceShader->Bind();

	irradianceShader->SetTextureCubeMap("environmentMap", envCubemap, 0);
	irradianceShader->SetMat4("projMatrix", captureProjection);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; i++) {
		irradianceShader->SetMat4("viewMatrix", captureViews[i]);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cube->Draw();

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Renderer::DrawPrefilterCubemap() {
	glGenTextures(1, &prefilterCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	prefilterShader->Bind();

	prefilterShader->SetTextureCubeMap("environmentMap", envCubemap, 0);
	prefilterShader->SetMat4("projMatrix", captureProjection);

	//glViewport(0, 0, 128, 128);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader->SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader->SetMat4("viewMatrix", captureViews[i]);
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterCubemap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cube->Draw();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::DrawBRDFLUTmap() {
	glGenTextures(1, &brdfLUTmap);

	glBindTexture(GL_TEXTURE_2D, brdfLUTmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTmap, 0);

	glViewport(0, 0, 512, 512);
	brdfShader->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	quad->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
}

void Renderer::DrawSkyBox() {
	glDepthFunc(GL_LEQUAL);

	skyboxShader->Bind();

	skyboxShader->SetTextureCubeMap("environmentMap", envCubemap, 0);
	//skyboxShader->SetTextureCubeMap("environmentMap", prefilterCubemap, 0);
	//skyboxShader->SetTextureCubeMap("environmentMap", irradianceCubemap, 0);

	skyboxShader->SetMat4("projMatrix", projMatrix);
	skyboxShader->SetMat4("viewMatrix", viewMatrix);

	cube->Draw();

	glDepthFunc(GL_LESS);
}

unsigned int Renderer::loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int Renderer::loadHdrTexture(const char* path) {
	stbi_set_flip_vertically_on_load(true);

	unsigned int hdrTextureID;
	glGenTextures(1, &hdrTextureID);

	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
	
	if (data)
	{
		
		glBindTexture(GL_TEXTURE_2D, hdrTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
		stbi_image_free(data);
	}

	return hdrTextureID;

}