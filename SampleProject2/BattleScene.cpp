#include "BattleScene.h"
#include "LightSystem.h"
#include "DirectionalLight.h"

#include "FBXLoader.hpp"
#include "Landscape.h"
#include "SkeletalMeshComponent.h"
#include "MaterialManager.h"
#include "SkyRenderSystem.h"
#include "SkyDomeComponent.h"
//추가
#include "SocketComponent.h"
#include "AnimationComponent.h"
#include "UpdateAnimSystem.h"
#include "MovementSystem.h"
#include "Character.h"
#include "SelectAnimSystem.h"
#include "BoundingBoxComponent.h"
#include "CameraArmComponent.h"

//추가
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



	// 카메라 시스템 및 랜더링 시스템 추가.
	TheWorld.AddSystem(MainCameraSystem);
	TheWorld.AddSystem(new ColliderSystem);
	MainRenderSystem = new RenderSystem;
	MainRenderSystem->MainCamera = MainCameraSystem->MainCamera;
	TheWorld.AddSystem(MainRenderSystem);

	TheWorld.AddSystem(new MovementSystem);
	TheWorld.AddSystem(new UpdateAnimSystem);
	// SelectAnimSystem 추가
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
		float MinGamDoe = 0.3f; // 나중에 마우스 민감도 필요하면?
		//MainCameraSystem->MainCamera->Pitch += MinGamDoe * Input::GetInstance()->m_ptOffset.x;
	}

	// 카메라 전환 예시용. 필요에 따라 수정 바람.
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

	// 얘는 일단 주기적으로 업데이트 해줘야함.
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
	//// 지형 액터 추가.
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

	// GenerateAnimationFromFileData()에서 애니메이션 컴포넌트에 애니메이션 추가하는 방식 
	// ClipList에 저장되며 SetClipByName(name) 함수로 변경가능 <- name = 확장자명 제외한 파일명
	auto playerCharAnimComp = PlayerCharacter->AddComponent<AnimationComponent>();
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Run.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Run.fbx", playerCharAnimComp);				// 달리기
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Idle.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Idle.fbx", playerCharAnimComp);				// 아이들
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Shooting.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Shooting.fbx", playerCharAnimComp,false);				// 공격
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Hit.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Hit.fbx", playerCharAnimComp,false);		// 피격
	}
	if (FBXLoader::GetInstance()->Load(L"../resource/FBX/Adam_fbx/Adam_anim/Dying.fbx"))
	{
		FBXLoader::GetInstance()->GenerateAnimationFromFileData(L"../resource/FBX/Adam_fbx/Adam_anim/Dying.fbx", playerCharAnimComp, false);				// 사망
	}

	playerCharAnimComp->SetClipByName(L"Idle");



	// 소켓 컴포넌트 추가
	auto socketComp = PlayerCharacter->AddComponent<SocketComponent>();
	// 스켈레탈 메시 & 본 이름 넘겨서 소켓 부착
	socketComp->Attach(*playerCharMeshComp, "RightHand");
	// 오프셋 조정 T R S
	socketComp->SetOffset(Vector3(2.0f, 2.0f, 0.0f), 
						  Vector3(-75.0f, -90.0f, -0.0f), 
						  Vector3(1.0f, 1.0f, 1.0f)); 

	// 무기로 쓸 스태틱 메시 추가 - 
	//엔티티가 소켓 컴포넌트를 갖고있으면 소켓 위치에 따라 스태틱 메시의 위치가 업데이트 되도록 렌더 시스템 설정
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

	// 플레이어용 카메라 및 카메라 암 설정.
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

