#pragma once
#include "LandscapeComponent.h"
#include "TransformComponent.h"
#include "Camera.h"

class LandscapeComponents
{
public:
	std::vector<LandscapeComponent> Components;

	ID3D11InputLayout* VertexLayout = nullptr;
	ID3D11VertexShader* VertexShader = nullptr;
	ID3D11PixelShader* PixelShader = nullptr;
	ID3D11HullShader* HullShader = nullptr;
	ID3D11DomainShader* DomainShader = nullptr;
	ID3D11GeometryShader* GeometryShader = nullptr;
	ID3D11Buffer* TransformBuffer = nullptr;
	TransformMatrix			TransformData;

	ID3D11Buffer* StreamOutputBuffer = nullptr;

	SculptingData	SculptData;
	ID3D11Buffer* SculptingBuffer = nullptr;

	DXTexture* BaseTexture = nullptr;
	std::vector<DXTexture*> LayerTextureList;

	int Row;
	int Column;

	Vector3 Center;

	std::vector<DirectX::BoundingBox> BoundingBoxList;

	TransformComponent* Transform;

public:
	Camera* MainCamera;

public:
	bool Initialize();

public:
	void Build(int row, int column, int sectionSize, int cellDistance = 100);
	void Render();
	void SetCamera(Camera* camera);
	void UpdateTransformMatrix(const TransformComponent& transform);
	void SetSculptingData(Vector3 pos, float radius, float attenuation, float strength, float weight);
	void SetBaseTexture(DXTexture* texture);
	void AddLayerTexture(DXTexture* texture);
};