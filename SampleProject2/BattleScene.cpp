#include "BattleScene.h"
#include "LightSystem.h"
#include "DirectionalLight.h"

#include "FBXLoader.hpp"
#include "Landscape.h"
#include "SkeletalMeshComponent.h"
#include "MaterialManager.h"
#include "SkyRenderSystem.h"
#include "SkyDomeComponent.h"
//�߰�
#include "SocketComponent.h"
#include "AnimationComponent.h"
#include "UpdateAnimSystem.h"
#include "MovementSystem.h"
#include "Character.h"
#include "SelectAnimSystem.h"
#include "BoundingBoxComponent.h"
#include "CameraArmComponent.h"

//�߰�
#include "ColliderSystem.h"

///////////////////
//Effect Include
///////////////////
#include "EffectInclude/EffectSystem.h"

bool BattleScene::Init()
{
	
	MainCameraSystem = new CameraSystem;
	MainCameraActor = new Actor;
	MainCamera = MainCameraActor->AddComponent<Camera>();
	
	MainCamera->CreateViewMatrix(Vector3(0.0f, 50.0f, -70.0f), Vector3(150.0f, 20.0f, 50.0f), Vector3(0.0f, 1.0, 0.0f));
	MainCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));
	MainCamera->Roll += 30.0f;
	MainCameraSystem->MainCamera = MainCamera;
	TheWorld.AddEntity(MainCameraActor);
	
	Init_Map();
	Init_Chara();



	// ī�޶� �ý��� �� ������ �ý��� �߰�.
	TheWorld.AddSystem(MainCameraSystem);
	TheWorld.AddSystem(new ColliderSystem);
	MainRenderSystem = new RenderSystem;
	MainRenderSystem->MainCamera = MainCameraSystem->MainCamera;
	TheWorld.AddSystem(MainRenderSystem);

	TheWorld.AddSystem(new MovementSystem);
	TheWorld.AddSystem(new UpdateAnimSystem);
	// SelectAnimSystem �߰�
	TheWorld.AddSystem(new SelectAnimSystem);

	LightSystem* lightSystem = new LightSystem;
	lightSystem->MainCamera = MainCameraSystem->MainCamera;
	lightSystem->Initialize();
	TheWorld.AddSystem(lightSystem);

 

	return true;
}

bool BattleScene::Frame()
{
	
	KeyState btnLC = Input::GetInstance()->getKey(VK_LBUTTON);
	if (btnLC == KeyState::Hold || btnLC == KeyState::Down)
	{
		float MinGamDoe = 0.3f; // ���߿� ���콺 �ΰ��� �ʿ��ϸ�?
		//MainCameraSystem->MainCamera->Pitch += MinGamDoe * Input::GetInstance()->m_ptOffset.x;
	}

	// ī�޶� ��ȯ ���ÿ�. �ʿ信 ���� ���� �ٶ�.
	KeyState btnZ = Input::GetInstance()->getKey('Z');
	if (btnZ == KeyState::Hold || btnZ == KeyState::Down)
	{
		MainCameraSystem->MainCamera = PlayerCharacter->GetComponent<Camera>();
	}
	KeyState btnX = Input::GetInstance()->getKey('X');
	if (btnX == KeyState::Hold || btnX == KeyState::Down)
	{
		
	}
	KeyState btnC = Input::GetInstance()->getKey('C');
	if (btnC == KeyState::Hold || btnC == KeyState::Down)
	{
		MainCameraSystem->MainCamera = MainCamera;
	}

	// ��� �ϴ� �ֱ������� ������Ʈ �������.
	MainRenderSystem->MainCamera = MainCameraSystem->MainCamera;

	PickedCharacter = (Character*)MAIN_PICKER.lastSelect.pTarget;
	
	return true;
}

bool BattleScene::Render()
{

	return true;
}

bool BattleScene::Release()
{
    return true;
}


void BattleScene::Init_Map()
{	
	//// ���� ���� �߰�.
	Landscape* landscape = new Landscape;
	auto landscapeComponents = landscape->GetComponent<LandscapeComponents>();
	landscapeComponents->Build(16, 16, 7, 10);
	landscapeComponents->SetCamera(MainCameraSystem->MainCamera);
	TheWorld.AddEntity(landscape);
	

	DirectionalLight* light = new DirectionalLight;
	auto lightComp = light->GetComponent<DirectionalLightComponent>();
	lightComp->Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	lightComp->Direction = Vector4(1.0f, -1.0f, 1.0f, 1.0f);
	TheWorld.AddEntity(light);

	DirectionalLight* light2 = new DirectionalLight;
	auto lightComp2 = light2->GetComponent<DirectionalLightComponent>();
	lightComp2->Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	lightComp2->Direction = Vector4(-1.0f, 1.0f, 1.0f, 1.0f);
	TheWorld.AddEntity(light2);

	
}

void BattleScene::Init_Chara()
{
	PlayerCharacter = new Character;
	player->chara = PlayerCharacter;
	auto playerCharMeshComp = PlayerCharacter->AddComponent<SkeletalMeshComponent>();


	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam.fbx")) 
	{
		FBXLoader::GetInstance()->GenerateSkeletalMeshFromFileData(L"../resource/FBX/Adam_fbx/Adam.fbx", playerCharMeshComp);
	}

	// GenerateAnimationFromFileData()���� �ִϸ��̼� ������Ʈ�� �ִϸ��̼� �߰��ϴ� ��� 
	// ClipList�� ����Ǹ� SetClipByName(name) �Լ��� ���氡�� <- name = Ȯ���ڸ� ������ ���ϸ�
	auto playerCharAnimComp = PlayerCharacter->AddComponent<AnimationComponent>();
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Run.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Run.fbx", playerCharAnimComp);				// �޸���
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Idle.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Idle.fbx", playerCharAnimComp);				// ���̵�
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Shooting.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Shooting.fbx", playerCharAnimComp,false);				// ����
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Hit.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Hit.fbx", playerCharAnimComp,false);		// �ǰ�
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Dying.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Dying.fbx", playerCharAnimComp, false);				// ���
	}

	playerCharAnimComp->SetClipByName(L"Idle");



	// ���� ������Ʈ �߰�
	auto socketComp = PlayerCharacter->AddComponent<SocketComponent>();
	// ���̷�Ż �޽� & �� �̸� �Ѱܼ� ���� ����
	socketComp->Attach(*playerCharMeshComp, "RightHand");
	// ������ ���� T R S
	socketComp->SetOffset(Vector3(2.0f, 2.0f, 0.0f), 
						  Vector3(-75.0f, -90.0f, -0.0f), 
						  Vector3(1.0f, 1.0f, 1.0f)); 

	// ����� �� ����ƽ �޽� �߰� - 
	//��ƼƼ�� ���� ������Ʈ�� ���������� ���� ��ġ�� ���� ����ƽ �޽��� ��ġ�� ������Ʈ �ǵ��� ���� �ý��� ����
	auto weaponMesh = PlayerCharacter->AddComponent<StaticMeshComponent>();

	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Pistol_fbx/Pistol.FBX"))
	{
		FBXLoader::GetInstance()->GenerateStaticMeshFromFileData(
							L"../resource/FBX/Adam_fbx/Pistol_fbx/Pistol.FBX", weaponMesh);
	}


	auto playerCharTransformComp = PlayerCharacter->GetComponent<TransformComponent>();
	playerCharTransformComp->Scale = Vector3(15.f, 15.f, 15.f);
	playerCharTransformComp->Rotation = Vector3(0.0f, -90.0f, 0.0f);
	playerCharTransformComp->Translation = Vector3(-100.0f, 0.0f, 0.0f);

	auto playerCharMovementComp = PlayerCharacter->GetComponent<MovementComponent>();
	playerCharMovementComp->Speed = 25.0f;
	PlayerCharacter->MoveTo(Vector3(-20.0f, 0.0f, 0.0f));

	//Picking Info Test
	playerCharMeshComp->Name = "player";

	////////////// Bounding Box Add /////////////////
	auto playerOBBComp = PlayerCharacter->AddComponent<BoundingBoxComponent>(Vector3(0.75f, 1.1f, 0.75f), Vector3(0.0f, 1.1f, 0.0f));

	// �÷��̾�� ī�޶� �� ī�޶� �� ����.
	auto playerCamera = PlayerCharacter->AddComponent<Camera>();
	auto playerCameraArm = PlayerCharacter->AddComponent<CameraArmComponent>();
	playerCameraArm->Distance = 100.0f;
	playerCameraArm->Roll = 35.0f;
	playerCameraArm->Pitch = 180.0f - 40.0f;
	playerCamera->CreateViewMatrix(Vector3(0.0f, 25.0f, -100.0f), Vector3(0.0f, 0.0f, 00.0f), Vector3(0.0f, 1.0, 0.0f));
	playerCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));
	
	//MainCamera = PlayerCharacter->AddComponent<Camera>();
	//MainCamera->CreateViewMatrix(Vector3(0.0f, 25.0f, -100.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0, 0.0f));
	//MainCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));
	
	TheWorld.AddEntity(PlayerCharacter);
	
	

}

