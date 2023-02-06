//#include "Renderer.h"
//
//Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
//	triangle = Mesh::GenerateTriangle();
//
//	basicShader = new Shader("basicVertex.glsl","colourFragment.glsl");
//
//	if(!basicShader->LoadSuccess()) {
//		return;
//	}
//
//	init = true;
//}
//Renderer::~Renderer(void)	{
//	delete triangle;
//	delete basicShader;
//}
//
//void Renderer::UpdateScene(float dt) {
//
//}
//
//void Renderer::RenderScene()	{
//	glClearColor(0.2f,0.2f,0.2f,1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);	
//
//	BindShader(basicShader);
//	triangle->Draw();
//}
//

#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"

#include <random>
#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	root = new SceneNode();
	screenSize = parent.GetScreenSize();


	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"mud.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//earthBump = SOIL_load_OGL_texture(TEXTUREDIR"mudnormals.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rockTex = SOIL_load_OGL_texture(TEXTUREDIR"Rock5.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//rockBump = SOIL_load_OGL_texture(TEXTUREDIR"Rock5_nmp.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	mountainTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	grassTex = SOIL_load_OGL_texture(TEXTUREDIR"grass.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"skybox_west.png", TEXTUREDIR"skybox_east.png",
									TEXTUREDIR"skybox_up.png", TEXTUREDIR"skybox_down.png",
									TEXTUREDIR"skybox_north.png", TEXTUREDIR"skybox_south.png",
									SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex) return;

	SetTextureRepeating(earthTex, true);
	//SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(rockTex, true);
	//SetTextureRepeating(rockBump, true);
	SetTextureRepeating(mountainTex, true);
	SetTextureRepeating(grassTex, true);

	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	//heightMapShader = new Shader("HeightMapVertex.glsl", "HeightMapFragment.glsl");
	heightMapShader = new Shader("PerPixelVertex.glsl", "HeightMapFragmentOLD.glsl");
	ssaoShader = new Shader("ssaoVertex2.glsl", "ssaoFragment.glsl");
	//characterShader = new Shader("SkinningVertex.glsl", "SkinningFragment.glsl");
	characterShader = new Shader("LitSkinningVertex.glsl", "LitSkinningFragment.glsl");
	//rainShader = new Shader("basicVertex.glsl", "colourFragment.glsl", "pointGeom.glsl");
	//shadowSceneShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
	//shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	//gBufferShader = new Shader("gBufferVertex.glsl", "gBufferFragment.glsl");
	gBufferShader = new Shader("bumpVertex.glsl", "bufferFragment.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !characterShader->LoadSuccess()) {
		return;
	}

	heightMap = new HeightMap(TEXTUREDIR"noise2.png");
	hmSize = heightMap->GetHeightmapSize();

	PopulateSceneGraph();

	mesh = Mesh::LoadFromMeshFile("Role_T.msh");
	anim = new MeshAnimation("Role_T.anm");
	material = new MeshMaterial("Role_T.mat");

	CalculateSampleKernels();
	SetUpBuffers();

	camera = new Camera(-45.0f, 0.0f, hmSize * Vector3(0.5f, 2.0f, 0.5f));
	light = new Light(hmSize * Vector3(0.5f, 2.0f, 0.5f), Vector4(1, 1, 1, 1), hmSize.x * 1.5f);
	
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry =	material->GetMaterialForLayer(i);		
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}

	currentFrame = 0;
	frameTime = 0.0f;

	init = true;
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete mesh;
	delete anim;
	delete material;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete characterShader;
	delete heightMapShader;
	delete light;

	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteTextures(1, &noiseTex);
	glDeleteTextures(2, &ssaoColourTex);
	glDeleteFramebuffers(1, &ssaoFBO);
}

void Renderer::UpdateScene(float dt, double totalTime) {
	camera->UpdateCamera(dt, totalTime);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 0.1f;
	waterCycle += dt * 0.01f;

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);

	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawSkybox();
	DrawCharacter();

	//GBuffer();
	//FillBuffers();
	//DrawSSAO();
	//SetUpBuffers();
	FillBuffers();
	DrawSSAO();

	BuildNodeLists(root);
	SortNodeLists();
	DrawNodes();
	ClearNodeLists();

	DrawHeightmap();
	DrawWater();
}

void Renderer::GBuffer() {
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	//BindShader(gBufferShader);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenSize.x, screenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);


	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	//unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	//glDrawBuffers(3, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetUpBuffers() {
	glGenFramebuffers(1, &gBufferFBO);

	GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	// Generate our scene depth texture ...
	GenerateScreenTexture(gBufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(gBufferNormalTex);

	//And now attach them to our FBOs
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBufferDepthTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glGenFramebuffers(1, &ssaoFBO);
	//GLenum buf = { GL_COLOR_ATTACHMENT0 };

	glGenTextures(1, &noiseTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GenerateScreenTexture(ssaoColourTex);
	
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColourTex, 0);
	//glDrawBuffer(buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	//glGenFramebuffers(1, &ssaoFBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	//glGenTextures(1, &ssaoColourTex);
	//glBindTexture(GL_TEXTURE_2D, ssaoColourTex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenSize.x, screenSize.y, 0, GL_RED, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColourTex, 0);

	//glDrawBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::FillBuffers() {
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(gBufferShader);
	UpdateShaderMatrices();
	
	for (int i = 0; i < root->GetChildren().size(); ++i) {
		if (root->GetChildren()[i]->GetMesh()) {
			modelMatrix = root->GetChildren()[i]->GetWorldTransform() * Matrix4::Scale(root->GetChildren()[i]->GetModelScale());
			UpdateShaderMatrices();
			for (int j = 0; j < root->GetChildren()[i]->GetMesh()->GetSubMeshCount(); j++) {
				root->GetChildren()[i]->GetMesh()->DrawSubMesh(j);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void Renderer::FillBuffers() {
//	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
//	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//
//	BindShader(gBufferShader);
//	UpdateShaderMatrices();
//
//	for (int i = 0; i < root->GetChildren().size(); ++i) {
//		if (root->GetChildren()[i]->GetMesh()) {
//			modelMatrix = root->GetChildren()[i]->GetWorldTransform() * Matrix4::Scale(root->GetChildren()[i]->GetModelScale());
//			//modelMatrix = nodeList[i]->GetTransform();
//			UpdateShaderMatrices();
//			for (int j = 0; j < root->GetChildren()[i]->GetMesh()->GetSubMeshCount(); j++) {
//				root->GetChildren()[i]->GetMesh()->DrawSubMesh(j);
//			}
//		}
//	}
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

//void Renderer::RenderRain() {
//
//	//Matrix4 modelMat = Matrix4::Translation(Vector3(0, 0, -30));
//
//	//RenderObject* object = new RenderObject(pointSprites, modelMat);
//
//
//	//object - > SetBaseTexture(
//	//	OGLTexture::RGBATextureFromFilename(" nyan .png "));
//	//object - > SetShader(newShader);
//	BindShader(rainShader);
//	UpdateShaderMatrices();
//
//	pointSprites->Draw();
//
//}

void Renderer::PopulateSceneGraph() {
	SceneNode* sn = new SceneNode();
	sn->SetShader(lightShader);
	//sn->SetTransform(Matrix4::Translation(Vector3(630.0f, -355.0f, 4168.0f)) * Matrix4::Rotation(-4.0f, Vector3(0,0,1)) );
	sn->SetTransform(Matrix4::Translation(Vector3(630.0f, -355.0f, 2700.0f)) * Matrix4::Rotation(-4.0f, Vector3(0,0,1)) );
	sn->SetModelScale(Vector3(150.0f, 50.0f, 50.0f));
	sn->SetMesh(Mesh::LoadFromMeshFile("Rock5A.msh"));
	sn->SetMaterial(new MeshMaterial("Rock5A1.mat"));
	sn->SetBoundingRadius(100000000.0f);
	root->AddChild(sn);

	SceneNode* sn1 = new SceneNode();
	sn1->SetShader(lightShader);
	sn1->SetTransform(Matrix4::Translation(Vector3(2000.0f, 10.0f, 2700.0f)));
	sn1->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn1->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_1.msh"));
	sn1->SetMaterial(new MeshMaterial("Col_1_tree_1.mat"));
	sn1->SetBoundingRadius(100000000.0f);
	root->AddChild(sn1);
		
	SceneNode* sn2 = new SceneNode();
	sn2->SetShader(lightShader);
	sn2->SetTransform(Matrix4::Translation(Vector3(3000.0f, 17.0f, 1100.0f)));
	sn2->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn2->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_2.msh"));
	sn2->SetMaterial(new MeshMaterial("Col_1_tree_2.mat"));
	sn2->SetBoundingRadius(100000000.0f);
	root->AddChild(sn2);

	/*SceneNode* sn3 = new SceneNode();
	sn3->SetShader(lightShader);
	sn3->SetTransform(Matrix4::Translation(Vector3(1375.0f, 10.0f, 2990.0f)));
	sn3->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn3->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_3.msh"));
	sn3->SetMaterial(new MeshMaterial("Col_1_tree_3.mat"));
	sn3->SetBoundingRadius(100000000.0f);
	root->AddChild(sn3);

	SceneNode* sn4 = new SceneNode();
	sn4->SetShader(lightShader);
	sn4->SetTransform(Matrix4::Translation(Vector3(1870.0f, 35.0f, 2335.0f)));
	sn4->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn4->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_4.msh"));
	sn4->SetMaterial(new MeshMaterial("Col_1_tree_4.mat"));
	sn4->SetBoundingRadius(100000000.0f);
	root->AddChild(sn4);

	SceneNode* sn5 = new SceneNode();
	sn5->SetShader(lightShader);
	sn5->SetTransform(Matrix4::Translation(Vector3(2600.0f, 35.0f, 2910.0f)));
	sn5->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn5->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_5.msh"));
	sn5->SetMaterial(new MeshMaterial("Col_1_tree_5.mat"));
	sn5->SetBoundingRadius(100000000.0f);
	root->AddChild(sn5);

	SceneNode* sn6 = new SceneNode();
	sn6->SetShader(lightShader);
	sn6->SetTransform(Matrix4::Translation(Vector3(1850.0f, 35.0f, 1520.0f)));
	sn6->SetModelScale(Vector3(8.0f, 8.0f, 8.0f));
	sn6->SetMesh(Mesh::LoadFromMeshFile("Col_1_tree_6.msh"));
	sn6->SetMaterial(new MeshMaterial("Col_1_tree_6.mat"));
	sn6->SetBoundingRadius(100000000.0f);
	root->AddChild(sn6);*/
}

//void Renderer::DrawShadowScene() {
//	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
//
//	glClear(GL_DEPTH_BUFFER_BIT);
//	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
//	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//
//	BindShader(shadowShader);
//
//	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
//	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
//	shadowMatrix = projMatrix * viewMatrix; // used later
//	
//	for (int i = 0; i < nodeList.size(); ++i) {
//		if (nodeList[i]->GetMesh()) {
//			modelMatrix = nodeList[i]->GetWorldTransform() * Matrix4::Scale(nodeList[i]->GetModelScale());
//			//modelMatrix = nodeList[i]->GetTransform();
//			UpdateShaderMatrices();
//			for (int j = 0; j < nodeList[i]->GetMesh()->GetSubMeshCount(); j++) {
//				nodeList[i]->GetMesh()->DrawSubMesh(j);
//			}
//		}
//	}
//
//	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//	glViewport(0, 0, width, height);
//
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

void Renderer::DrawCharacter() {
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(characterShader);
	SetShaderLight(*light);
	glUniform1i(glGetUniformLocation(characterShader->GetProgram(), "characterTex"), 0);
	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());
	
	modelMatrix = modelMatrix * modelMatrix.Rotation(270.0f, Vector3(1, 0, 0));
	modelMatrix = modelMatrix * modelMatrix.Scale(Vector3(0.025f, 0.4f, 0.025f));


	UpdateShaderMatrices();
	
	vector<Matrix4>frameMatrices;
	
	const Matrix4* invBindPose = mesh->GetInverseBindPose();
	const Matrix4* frameData = anim->GetJointData(currentFrame);
	
	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}
	
	int j = glGetUniformLocation(characterShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		mesh->DrawSubMesh(i);
	}
}

//void Renderer::DrawRock() {
//	BindShader(lightShader);
//	SetShaderLight(*light);
//
//	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());
//
//	//glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
//	//glActiveTexture(GL_TEXTURE0);
//	//glBindTexture(GL_TEXTURE_2D, rockTex);
//
//	//glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
//	//glActiveTexture(GL_TEXTURE1);
//	//glBindTexture(GL_TEXTURE_2D, rockBump);
//
//	//modelMatrix.ToIdentity();
//	//textureMatrix.ToIdentity();
//
//	UpdateShaderMatrices();
//
//	for (int i = 0; i < rockMesh->GetSubMeshCount(); ++i) {
//		//glActiveTexture(GL_TEXTURE0);
//		//glBindTexture(GL_TEXTURE_2D, rockMatTextures[i]);
//		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, rockTex);
//
//		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, rockBump);
//
//		rockMesh->DrawSubMesh(i);
//	}
//}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	BindShader(heightMapShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	//glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "bumpTex"), 1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, earthBump);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "grassTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grassTex);

	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "rockDiffuseTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mountainTex);

	//glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "shadowTex"), 3);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	//glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "rockBumpTex"), 3);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, mountainBump);

	//float heights[2] = { 2.0f, 5.0f };
	//glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "heights"), 1, heights);

	modelMatrix.ToIdentity(); // New !
	textureMatrix.ToIdentity(); // New !

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Matrix4 translation = Matrix4::Translation(Vector3(hmSize.x, hmSize.y * 0.2f, hmSize.z) * 0.5f);

	modelMatrix = translation * Matrix4::Scale(hmSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	// SetShaderLight (* light ); // No lighting in this shader !
	quad->Draw();
}

void Renderer::DrawSSAO() {
/*	glGenFramebuffers(1, &ssaoFBO);

	GLenum buf = {	GL_COLOR_ATTACHMENT0 };
	GenerateScreenTexture(ssaoColourTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColourTex, 0);
	glDrawBuffer(buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}*/
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(ssaoShader);
	   
	glDisable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noiseTex);

	glUniform1i(glGetUniformLocation(ssaoShader->GetProgram(), "texNoise"), 0);
	glUniform3fv(glGetUniformLocation(ssaoShader->GetProgram(), "samples"), 64, (float*) ssaoKernel.data());
	glUniform2fv(glGetUniformLocation(ssaoShader->GetProgram(), "screenSize"), 1, (float*)& screenSize);
	glUniform2f(glGetUniformLocation(ssaoShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(ssaoShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();

	//glActiveTexture(GL_TEXTURE1);
	//glUniform1i(glGetUniformLocation(ssaoShader->GetProgram(), "gPosition"), 1);
	//glBindTexture(GL_TEXTURE_2D, gPosition);

	//glActiveTexture(GL_TEXTURE2);
	//glUniform1i(glGetUniformLocation(ssaoShader->GetProgram(), "gNormal"), 2);
	//glBindTexture(GL_TEXTURE_2D, gNormal);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(ssaoShader->GetProgram(), "depthTex"), 1);
	glBindTexture(GL_TEXTURE_2D, gBufferDepthTex);

	glActiveTexture(GL_TEXTURE2);
	glUniform1i(glGetUniformLocation(ssaoShader->GetProgram(), "normTex"), 2);
	glBindTexture(GL_TEXTURE_2D, gBufferNormalTex);

	quad->Draw();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CalculateSampleKernels() {
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	ssaoKernel.clear();
	ssaoNoise.clear();

	for (unsigned int i = 0; i < 64; ++i){
		Vector3 sample =	Vector3(randomFloats(generator) * 2.0 - 1.0,
									randomFloats(generator) * 2.0 - 1.0,
									randomFloats(generator));

		float scale = (float)i / 64.0;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample = sample * scale;		
		ssaoKernel.push_back(sample);
	}

	for (unsigned int i = 0; i < 16; i++) {
		Vector3 noise = Vector3(randomFloats(generator) * 2.0 - 1.0,
								randomFloats(generator) * 2.0 - 1.0,
								0.0f);
		ssaoNoise.push_back(noise);
	}
}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		BindShader(n->GetShader());
		SetShaderLight(*light);

		//Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());

		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();

		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
		//glUniform4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());
		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

		//if (n->HasAnim()) 
		//	Animate(n->GetShader(), n->GetMesh(), n->GetAnimation());

		for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); i++) {
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, n->GetTextures()[i]);

			GLuint a = n->GetDiffuseTextures()[i];
			//glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);
			glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "diffuseTex"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, n->GetDiffuseTextures()[i]);



			float hasBumpMap = n->GetNormalMaps().size() != 0;
			//glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "hasBumpMap"), 1, (float*)& hasBumpMap);
			glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "hasBumpMap"), hasBumpMap);
			if (hasBumpMap) {
				GLuint b = n->GetNormalMaps()[i];
				glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "bumpTex"), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, n->GetNormalMaps()[i]);
			}

			glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "ssaoTex"), 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, ssaoColourTex);

			/*modelMatrix.ToIdentity();
			textureMatrix.ToIdentity();*/

			//n->GetMesh()->Draw();
			n->GetMesh()->DrawSubMesh(i);
		}

		//n->Draw(*this);
	}
}

void Renderer::Animate(Shader* s, Mesh* m, MeshAnimation* a) {
	vector<Matrix4>frameMatrices;

	const Matrix4* invBindPose = m->GetInverseBindPose();
	const Matrix4* frameData = a->GetJointData(currentFrame);

	for (unsigned int i = 0; i < m->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(s->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}
