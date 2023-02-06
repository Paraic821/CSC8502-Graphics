#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour) {
	this->mesh = mesh;
	this->colour = colour;
	parent = NULL;
	modelScale = Vector3(1, 1, 1);

	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
	anim = nullptr;
}

SceneNode::~SceneNode(void) {
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}

	delete mesh;
	delete material;
	delete shader;
}

void SceneNode::AddChild(SceneNode* s) {
	children.push_back(s);
	s->parent = this;
}

void SceneNode::Scale(Vector3 s) {
	transform = transform * transform.Scale(s);
}

void SceneNode::Draw(const OGLRenderer& r) {
	if (mesh) { mesh->Draw(); }
}

void SceneNode::Update(float dt) {
	if (parent) {	//This node has a parent...
		worldTransform = parent->worldTransform * transform;
	}
	else {			//Root node, world transform is local transform!
		worldTransform = transform;
	}
	for (vector<SceneNode*>::iterator i = children.begin();
		i != children.end(); ++i) {
		(*i) -> Update(dt);
	}
}

void SceneNode::SetMaterial(MeshMaterial* m) {
	material = m;
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
		const string* filename = nullptr;
		string path;
		if (matEntry->GetEntry("Diffuse", &filename)) {
			path = TEXTUREDIR + *filename;
			GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			if (texID != 0) {
				diffuseTextures.emplace_back(texID);
			}
		}

		filename = nullptr;
		if(matEntry->GetEntry("Bump", &filename)){
			path = TEXTUREDIR + *filename;
			GLuint texID2 = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			if (texID2 != 0) {
				normalMaps.emplace_back(texID2);
			}
		}
	}
}
