#pragma once
#include "EditorCore.h"
#include "World.hpp"
#include "Camera.h"

#include "WindowsClient.h"
#include "Character.h"
#include "PlaneComponent.h"
#include "CameraSystem.h"
#include "RenderSystem.h"

ECS_DEFINE_TYPE(SomeComponent);

class SampleCore : public EditorCore
{
public:
	ECS::World TheWorld;
	Camera* MainCamera;
	Camera* DebugCamera;
	Actor* DebugCameraActor = nullptr;

public:	
	CameraSystem* MainCameraSystem = nullptr;
	RenderSystem* MainRenderSystem = nullptr;
	Actor* MainCameraActor = nullptr;
	Character* PlayerCharacter = nullptr;
	Character* PickedCharacter = nullptr;

public:
	// create instance
	ImGui::FileBrowser	FileDlg;
	std::wstring			FilePath;
	//std::string			FileName;
public:
	SampleCore();
	~SampleCore();

public:
	void CameraMove(Vector3& eye, Vector3& target);

	virtual bool Initialize() override;
	virtual bool Frame() override;
	virtual bool Render() override;
	virtual bool Release() override;

public:
	void LoadTexture();

	void Init_Chara();
	void Init_Map();
};