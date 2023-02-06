//#pragma once
//#include "../NCLGL/OGLRenderer.h"
//
//class Renderer : public OGLRenderer	{
//public:
//	Renderer(Window &parent);
//	 ~Renderer(void);
//	 void RenderScene()				override;
//	 void UpdateScene(float msec)	override;
//protected:
//	Mesh*	triangle;
//	Shader* basicShader;
//};

#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustrum.h"
class Camera;
class Shader;
class HeightMap;
class Mesh;
class MeshAnimation;
class MeshMaterial;


class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt, double totalTime);

protected:
	void DrawHeightmap();
	void DrawWater();
	void DrawSkybox();
	void DrawShadowScene();
	void DrawSSAO();
	void DrawCharacter();
	void DrawRock();
	void RenderRain();

	void GBuffer();
	void SetUpBuffers();
	void FillBuffers();

	void GenerateScreenTexture(GLuint& into, bool depth = false, bool largeFormat = false);

	void PopulateSceneGraph();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void Animate(Shader* s, Mesh* m, MeshAnimation* a);

	void CalculateSampleKernels();

	inline float lerp(float a, float b, float f) {
		return a + f * (b - a);
	}

	Shader* lightShader;
	Shader* heightMapShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* shadowShader;
	Shader* ssaoShader;
	Shader* characterShader;
	Shader* gBufferShader;
	Shader* rainShader;

	HeightMap* heightMap;
	Vector3 hmSize;
	Mesh* quad;
	Mesh* mesh;
	Mesh* rockMesh;
	Mesh* pointSprites;

	MeshAnimation* anim;
	MeshMaterial* material;
	MeshMaterial* rockMaterial;
	vector<GLuint> matTextures;
	vector<GLuint> rockMatTextures;


	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint rockTex;
	GLuint rockBump;
	GLuint mountainTex;
	GLuint grassTex;
	GLuint mountainBump;

	GLuint shadowTex;
	GLuint shadowFBO;


	GLuint gPosition;
	GLuint gNormal;
	GLuint gBuffer;
	GLuint gAlbedo;

	GLuint gBufferFBO;
	GLuint bufferColourTex;
	GLuint gBufferNormalTex;
	GLuint gBufferDepthTex;
	GLuint gBufferWorldPosTex;

	GLuint noiseTex;
	GLuint ssaoFBO;
	GLuint ssaoColourTex;

	float waterRotate;
	float waterCycle;

	vector<Vector3> ssaoKernel;
	vector<Vector3> ssaoNoise;

	Vector2 screenSize;

	int currentFrame;
	float frameTime;

	SceneNode* root;
	Frustum frameFrustum;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};