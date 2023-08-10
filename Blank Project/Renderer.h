#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"


class Renderer :public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	//Scene
	void RenderScene() override;
	void RenderBlinnPhongScene();
	void RenderNoIBLScene();

	void UpdateScene(float dt) override;

protected:
	void DrawBall();
	void DrawModel();
	void DrawBlinnPhongModel();
	void DrawNoIBLModel();
	void DrawLight();
	void DrawEnvironmentCubemap();
	void DrawSkyBox();
	void DrawIrradianceCubemap();
	void DrawPrefilterCubemap();
	void DrawBRDFLUTmap();

	unsigned int loadTexture(const char* path);
	unsigned int loadHdrTexture(const char* path);

	Camera* camera;

	//Mesh
	Mesh* quad;		
	Mesh* sphere;
	Mesh* cube;
	Mesh* helmet;

	//Shader
	Shader* unlitShader;
	Shader* pbrShader;
	Shader* equirectangularToCubemapShader;
	Shader* skyboxShader;
	Shader* irradianceShader;
	Shader* prefilterShader;
	Shader* brdfShader;
	Shader* blinnPhongShader;
	Shader* pbrNoIBLShader;

	//Texture
	GLuint cubeMap;
	GLuint hdrTex;

	GLuint albedo;
	GLuint normal;
	GLuint metallic;
	GLuint roughness;
	GLuint ao;

	//FrameBuffer
	GLuint captureFBO;
	GLuint captureRBO;

	GLuint envCubemap;
	GLuint irradianceCubemap;
	GLuint prefilterCubemap;
	GLuint brdfLUTmap;

	Matrix4 captureProjection;
	Matrix4 captureViews[6];

	float elaspedTime = 0.0f;

	bool hasCapturedEnvCubemap = false;
	bool hasCapturedIrrCubemap = false;
	bool hasCapturedPrefilterCubemap = false;
	bool hasCapturedbrdfLUTmap = false;
};
