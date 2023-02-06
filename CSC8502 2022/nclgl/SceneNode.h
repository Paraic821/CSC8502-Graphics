#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>
#include "MeshMaterial.h"
#include "MeshAnimation.h"

class SceneNode {
public:
	SceneNode(Mesh* m = NULL, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);
	
	void SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	Matrix4 GetWorldTransform() const { return worldTransform; }
	
	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }
	
	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }
		
	Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	MeshAnimation* GetAnimation() const { return anim; }
	void SetAnimation(MeshAnimation* a) { 
		anim = a; 
		hasAnim = true;
	}
	bool HasAnim() { return hasAnim; }

	MeshMaterial* GetMaterial () const { return material; }

	vector<GLuint> GetDiffuseTextures() const { return diffuseTextures; }
	//void SetTextures(vector<GLuint> t) { 
	//	diffuseTextures.clear();
	//	for (int i = 0; i < t.size(); i++) {
	//		diffuseTextures.push_back(t[i]);
	//	}
	//}

	vector<GLuint> GetNormalMaps() const { return normalMaps; }

	int GetChildCount() { return children.size(); }

	Shader* GetShader() const { return shader; }
	void SetShader(Shader* s) { shader = s; }
	
	void AddChild(SceneNode* s);

	void Scale(Vector3 s);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer& r);
	virtual void SetMaterial(MeshMaterial* m);
	
	std::vector <SceneNode*>::const_iterator GetChildIteratorStart() {
		return children.begin();
	}
	
	std::vector <SceneNode*>::const_iterator GetChildIteratorEnd() {
		return children.end();
	}

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }
	
	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }
	
	void SetTexture(GLuint tex) { texture = tex; }
	GLuint GetTexture() const { return texture; }
	
	static bool CompareByCameraDistance(SceneNode * a, SceneNode * b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true : false;
	}

	vector<SceneNode*> GetChildren() {
		return children;
	}


protected:
	SceneNode* parent;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;

	Mesh* mesh;
	MeshMaterial* material;
	vector<GLuint> diffuseTextures;
	vector<GLuint> normalMaps;
	MeshAnimation* anim;

	bool hasAnim = false;

	Shader* shader;

	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;
};
