#include "Sample.h"
#include "FBXLoader.hpp"


#include "ColliderSystem.h"
#include "BoundingBoxComponent.h"
#include "BoundingSphereComponent.h"


//////////////////
#include "LightSystem.h"
#include "DirectionalLight.h"

#include "FBXLoader.hpp"
#include "Landscape.h"
#include "SkeletalMeshComponent.h"
#include "MaterialManager.h"
#include "SocketComponent.h"
#include "AnimationComponent.h"
#include "UpdateAnimSystem.h"
#include "MovementSystem.h"
#include "SelectAnimSystem.h"
#include "CameraArmComponent.h"

struct CustomEvent
{
	int SomeNumber;
	bool SomeBoolean;
};

struct TickEvent
{
	float fCurTime;
};

ECS_DEFINE_TYPE(SomeEvent);
namespace ECS
{
	class TestSystem : public System,
		public EventSubscriber<CommonEvents::OnEntityCreated>,
		public EventSubscriber<CommonEvents::OnEntityDestroyed>,
		//public EventSubscriber<CommonEvents::OnComponentAdded<TransformComponent>>,
		public EventSubscriber<CustomEvent>,
		public EventSubscriber<TickEvent>
	{
	public:
		virtual ~TestSystem() {}

		void init(World* world) override
		{
			world->subscribe<CommonEvents::OnEntityCreated>(this);
			world->subscribe<CommonEvents::OnEntityDestroyed>(this);
			//world->subscribe<CommonEvents::OnComponentAdded<TransformComponent>>(this);
			world->subscribe<CustomEvent>(this);
			world->subscribe<TickEvent>(this);
		};

		void release(World* world) override
		{
			world->unsubscribe<CommonEvents::OnEntityCreated>(this);
			world->unsubscribe<CommonEvents::OnEntityDestroyed>(this);
			//world->subscribe<CommonEvents::OnComponentAdded<TransformComponent>>(this);
			world->unsubscribe<CustomEvent>(this);
			world->unsubscribe<TickEvent>(this);
		}

		virtual void Tick(World* world, float deltaTime) override
		{
			world->emit<TickEvent>({ Singleton<Timer>::GetInstance()->GetPlayTime() });
		}

		virtual void receive(class World* world, const CommonEvents::OnEntityCreated& event) override
		{
			OutputDebugString(L"Entity 생성 \n");
		}

		virtual void receive(class World* world, const CommonEvents::OnEntityDestroyed& event) override
		{
			OutputDebugString(L"Entity 파괴 \n");
		}

		/*virtual void receive(class World* world, const CommonEvents::OnComponentAdded<TransformComponent>& event) override
		{
			OutputDebugString(L"A Transform component was removed!");
		}*/

		virtual void receive(class World* world, const CustomEvent& event) override
		{
			OutputDebugString(L"Custom Event Value : ");
			OutputDebugString(L"\n");
			OutputDebugString((L"Int Param : " + std::to_wstring(event.SomeNumber)).c_str());
			OutputDebugString(L"\n");
			OutputDebugString((L"Boolean Param : " + std::to_wstring(event.SomeBoolean)).c_str());
			OutputDebugString(L"\n");
		}

		virtual void receive(class World* world, const TickEvent& event) override
		{
			OutputDebugString((std::to_wstring(event.fCurTime) + L" : DEBUG_TICK_EVENT\n").c_str());
		}
	};
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	WindowsClient sampleWindow;
	sampleWindow.Create(hInstance, L"Window Name", 1600, 900);

	SampleCore core;
	core.SetWindowHandle(sampleWindow.GetHandle());
	core.CoreInitialize();
	bool isRun = true;
	while (isRun)
	{
		if (!sampleWindow.Run())
		{
			isRun = false;
		}
		else
		{
			core.CoreFrame();
			core.CoreRender();
		}
	}

	core.CoreRelease();

	return 1;
}

SampleCore::SampleCore()
{
}

SampleCore::~SampleCore()
{
}

bool SampleCore::Initialize()
{
	EditorCore::Initialize();
	FBXLoader::GetInstance()->Initialize();

	MAIN_PICKER.optPickingMode = PMOD_LAND;

	/////////////////////////////
	MainCameraSystem = new CameraSystem;
	MainCameraActor = new Actor;
	MainCamera = MainCameraActor->AddComponent<Camera>();

	MainCamera->CreateViewMatrix(Vector3(0.0f, 50.0f, -70.0f), Vector3(150.0f, 20.0f, 50.0f), Vector3(0.0f, 1.0, 0.0f));
	MainCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));
	MainCamera->Roll += 30.0f;
	MainCameraSystem->MainCamera = MainCamera;
	TheWorld.AddEntity(MainCameraActor);

	MainCameraSystem->MainCamera = MainCamera;

	///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////DEBUG CAM/////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	DebugCameraActor = new Actor;
	DebugCamera = DebugCameraActor->AddComponent<Camera>();
	//auto DebugTransform = DebugCameraActor->GetComponent<TransformComponent>();
	//DebugTransform->Translation = Vector3(0.0f, 25.0f, -50.0f);

	DebugCamera->CreateViewMatrix(Vector3(0.0f, 25.0f, -50.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0, 0.0f));
	DebugCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));
	DebugCamera->Roll += 30.0f;
	TheWorld.AddEntity(DebugCameraActor);


	///////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////File Browser//////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	FileDlg.SetTypeFilters({ ".fbx" ,".FBX" });

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

bool SampleCore::Frame()
{

	MAIN_PICKER.setMatrix(nullptr, &(MainCamera->View), &(MainCamera->Projection));

	EditorCore::Frame();

	// ImGui Frame()
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////CAMERA SETTING////////////////////////////////////////////
	// ///////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// 카메라 전환 예시용. 필요에 따라 수정 바람.
		KeyState btnZ = Input::GetInstance()->getKey('Z');
		if (btnZ == KeyState::Hold || btnZ == KeyState::Down)
		{
			if (PlayerCharacter != nullptr)
				MainCameraSystem->MainCamera = PlayerCharacter->GetComponent<Camera>();
		}
		KeyState btnX = Input::GetInstance()->getKey('X');
		if (btnX == KeyState::Hold || btnX == KeyState::Down)
		{
			MainCameraSystem->MainCamera = DebugCamera;
		}
		KeyState btnC = Input::GetInstance()->getKey('C');
		if (btnC == KeyState::Hold || btnC == KeyState::Down)
		{
			MainCameraSystem->MainCamera = MainCamera;
		}

		// Debug Cam control
		//auto DebugTransform = DebugCameraActor->GetComponent<TransformComponent>();
		static float m_fSpeed = 10.f;
		if (MainCameraSystem->MainCamera == DebugCamera)
		{
			float dt = Timer::GetInstance()->SecondPerFrame;
			KeyState btnSpace = Input::GetInstance()->getKey(VK_SPACE);
			if (btnSpace == KeyState::Hold || btnSpace == KeyState::Down)
			{
				m_fSpeed += dt * 100.0f;
			}
			else
			{
				m_fSpeed -= dt * 100.0f;
			}
			m_fSpeed = max(10.0f, m_fSpeed);
			m_fSpeed = min(100.0f, m_fSpeed);

			KeyState btnRB = Input::GetInstance()->getKey(VK_RBUTTON);
			if (btnRB == KeyState::Hold || btnRB == KeyState::Down)
			{
				DebugCamera->Pitch += Input::GetInstance()->m_ptOffset.x * 0.02f;
				//DebugCamera->Yaw += Input::GetInstance()->m_ptOffset.y * 0.02f;
				DebugCamera->Roll += Input::GetInstance()->m_ptOffset.y * 0.02f;

				KeyState btnW = Input::GetInstance()->getKey('W');
				if (btnW == KeyState::Hold || btnW == KeyState::Down)
				{
					Vector3 v = DebugCamera->Look * m_fSpeed * dt;
					DebugCamera->Pos += v;
				}

				KeyState btnS = Input::GetInstance()->getKey('S');
				if (btnS == KeyState::Hold || btnS == KeyState::Down)
				{
					Vector3 v = DebugCamera->Look * -m_fSpeed * dt;
					DebugCamera->Pos += v;
				}

				KeyState btnA = Input::GetInstance()->getKey('A');
				if (btnA == KeyState::Hold || btnA == KeyState::Down)
				{
					Vector3 v = DebugCamera->Right * -m_fSpeed * dt;
					DebugCamera->Pos += v;
				}

				KeyState btnD = Input::GetInstance()->getKey('D');
				if (btnD == KeyState::Hold || btnD == KeyState::Down)
				{
					Vector3 v = DebugCamera->Right * m_fSpeed * dt;
					DebugCamera->Pos += v;
				}
			}
		}

		// 얘는 일단 주기적으로 업데이트 해줘야함.
		MainRenderSystem->MainCamera = MainCameraSystem->MainCamera;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////IMGUI MENUBAR////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool fbxListOpen = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("FBX List"))
			{
				if (fbxListOpen != true)
				{
					fbxListOpen = true;
				}
			}
			if (ImGui::MenuItem("Reset"))
			{
				PlayerCharacter->RemoveComponent<SkeletalMeshComponent>();
				PlayerCharacter->RemoveComponent<AnimationComponent>();
				PlayerCharacter->RemoveComponent<SocketComponent>();
				PlayerCharacter->RemoveComponent<StaticMeshComponent>();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////FBX INFO////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	if (fbxListOpen)
	{
		if (ImGui::Begin("FBX List##1", &fbxListOpen))
		{
			if (ImGui::ListBoxHeader("##FBX List2", ImVec2(0, 250)))
			{
				for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
				{

					int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
					std::string fbxName(size_needed, 0);
					WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

					ImGui::Selectable(fbxName.c_str());
				}
				ImGui::ListBoxFooter();
			}

			if (ImGui::Button("Load FBX"))
			{
				FileDlg.Open();
			}
		}
		ImGui::End();
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////FILE BROWSER/////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	FileDlg.Display();

	if (FileDlg.HasSelected())
	{
		FilePath = FileDlg.GetSelected().wstring();
		//FileName = FileDlg.GetSelected().filename().string();
		FileDlg.ClearSelected();

		// Fbx Load
		FBXLoader::GetInstance()->Load(FilePath);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////IMGUI INFO///////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	if (PlayerCharacter != nullptr)
	{
		ImGui::SetNextWindowSize(ImVec2(600, 850));
		if (ImGui::Begin("Character Information"))
		{
			static int page = 0;
			const char* tabs[] = { "Skeletal Mesh","Animation","Transform", "Socket", "Static Mesh" , "Bounding Box" };
			ImGui::BeginGroup();
			for (int i = 0; i < ARRAYSIZE(tabs); i++)
			{
				ImGui::SameLine();
				if (ImGui::Button(tabs[i], ImVec2(590 / ARRAYSIZE(tabs) - 10, 25)))
					page = i;
			}
			ImGui::EndGroup();

			switch (page)
			{
			case 0:	/////////////////////////////// SKELETAL MESH /////////////////////////////////

				if (PlayerCharacter->has<SkeletalMeshComponent>()) 
				{
					auto mesh = PlayerCharacter->GetComponent< SkeletalMeshComponent>();
					ImGui::Text("Skeletal Mesh Component : ");	ImGui::SameLine();

					int size_needed = WideCharToMultiByte(CP_UTF8, 0, &mesh->FBXName[0], (int)mesh->FBXName.size(), NULL, 0, NULL, NULL);
					std::string fbxName(size_needed, 0);
					WideCharToMultiByte(CP_UTF8, 0, &mesh->FBXName[0], (int)mesh->FBXName.size(), &fbxName[0], size_needed, NULL, NULL);

					ImGui::Text(fbxName.c_str());

					if (ImGui::TreeNode("Bone List"))
					{
						if (ImGui::ListBoxHeader("##Selectable List", ImVec2(500, 500)))
						{
							for (auto iter = mesh->BindPoseMap.begin(); iter != mesh->BindPoseMap.end(); iter++)
							{
								ImGui::Selectable(iter->first.c_str());
							}
							ImGui::ListBoxFooter();
						}
						ImGui::TreePop();
					}
					if (ImGui::Button("Remove Component##SKMesh"))
					{
						PlayerCharacter->RemoveComponent<SkeletalMeshComponent>();
					}
				}
				else
				{

					ImGui::Text("No Component ... Add SKMesh");
					if (ImGui::BeginListBox("##FBX List3", ImVec2(500, 250)))
					{
						static std::wstring SKData;

						for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
						{

							int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
							std::string fbxName(size_needed, 0);
							WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

							const bool is_selected = (SKData == iter->first);
							if (ImGui::Selectable(fbxName.c_str(), is_selected))
							{
								SKData = iter->first;
							}
						}
						ImGui::EndListBox();
						if (ImGui::Button("Add Component##SKMesh"))
						{

							auto playerCharMeshComp = PlayerCharacter->AddComponent<SkeletalMeshComponent>();

							FBXLoader::GetInstance()->GenerateSkeletalMeshFromFileData(SKData, playerCharMeshComp);

						}
					}
				}
				break;

			case 1:  /////////////////////////////// Animation /////////////////////////////////

				if (PlayerCharacter->has<AnimationComponent>()) 
				{
					auto anim = PlayerCharacter->GetComponent<AnimationComponent>();
					ImGui::Text("Animation Clip Name : ");	ImGui::SameLine();

					int size_needed = WideCharToMultiByte(CP_UTF8, 0, &anim->CurrentClipName[0], (int)anim->CurrentClipName.size(), NULL, 0, NULL, NULL);
					std::string clipName(size_needed, 0);
					WideCharToMultiByte(CP_UTF8, 0, &anim->CurrentClipName[0], (int)anim->CurrentClipName.size(), &clipName[0], size_needed, NULL, NULL);

					ImGui::Text(clipName.c_str());

					static bool loop = true;
					unsigned int min = 0;
					unsigned int max = 0;
					if (anim->CurrentClip != nullptr)
					{
						min = anim->CurrentClip->StartFrame;
						max = anim->CurrentClip->EndFrame;
					}
					ImGui::Text("Current Frame : %f", anim->m_currentAnimationFrame);							// 현재 프레임
					ImGui::Text("Start : %d", min);					// 시작 프레임
					ImGui::SameLine();
					ImGui::Text("End : %d", max);						// 끝 프레임

					if (ImGui::Button("Play"))
					{
						if (anim->CurrentClip != nullptr)
						{
							anim->CurrentClip->FrameSpeed = 30.0f;
						}

					}
					ImGui::SameLine();
					if (ImGui::Button("Pause"))
					{
						if (anim->CurrentClip != nullptr)
						{
							anim->CurrentClip->FrameSpeed = 0.0f;
						}

					}
					ImGui::SameLine();
					if (ImGui::Button("Replay"))
					{
						if (anim->CurrentClip != nullptr)
						{
							anim->m_currentAnimationFrame = 0;
							anim->CurrentClip->FrameSpeed = 30.0f;
						}

					}
					ImGui::SameLine();
					ImGui::Checkbox("Loop", &loop);														// 루프 버튼
					if (anim->CurrentClip != nullptr)
					{
						anim->CurrentClip->LoopState = loop;
					}

					// 프레임 슬라이더
					if (ImGui::DragFloat("##currentFrame", &anim->m_currentAnimationFrame, 0.1f, min, max))
					{
						if (anim->CurrentClip != nullptr)
						{
							anim->CurrentClip->FrameSpeed = 0.0f;
						}
					}ImGui::SameLine();
					ImGui::Text("Frame");	ImGui::SameLine();
					ImGui::InputFloat("Frame##inputfloat", &anim->m_currentAnimationFrame);

					if (ImGui::TreeNode("Clip List"))
					{
						if (ImGui::ListBoxHeader("##Selectable List", ImVec2(0, 500)))
						{
							static std::wstring currentClip;

							for (auto iter = anim->ClipList.begin(); iter != anim->ClipList.end(); iter++)
							{
								int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
								std::string clipName(size_needed, 0);
								WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &clipName[0], size_needed, NULL, NULL);
								const bool is_selected = (currentClip == iter->first);
								if (ImGui::Selectable(clipName.c_str(), is_selected))
								{
									currentClip = iter->first;
								}
							}
							ImGui::ListBoxFooter(); ImGui::SameLine();
							if (ImGui::Button("Play Clip##AnimClip"))
							{
								anim->m_currentAnimationFrame = 0.0f;

								anim->SetClipByName(currentClip);
							}
						}					
						ImGui::TreePop();
					}
					if (ImGui::TreeNode("FBX List##animfbx"))
					{
						if (ImGui::BeginListBox("##FBX List3", ImVec2(500, 250)))
						{
							static std::wstring ClipData;

							for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
							{

								int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
								std::string fbxName(size_needed, 0);
								WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

								const bool is_selected = (ClipData == iter->first);
								if (ImGui::Selectable(fbxName.c_str(), is_selected))
								{
									ClipData = iter->first;
								}
							}
							ImGui::EndListBox();
							if (ImGui::Button("Add Clip##AnimClip"))
							{

								FBXLoader::GetInstance()->GenerateAnimationFromFileData(ClipData, anim);
							}
						}
						ImGui::TreePop();
					}
					if (ImGui::Button("Remove Component##AnimComp"))
					{
						PlayerCharacter->RemoveComponent<AnimationComponent>();
					}
					
				}
				else
				{
					ImGui::Text("No Component ... Add AnimComponent");
					if (ImGui::BeginListBox("##FBX List3", ImVec2(500, 250)))
					{
						static std::wstring AnimData;

						for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
						{

							int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
							std::string fbxName(size_needed, 0);
							WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

							const bool is_selected = (AnimData == iter->first);
							if (ImGui::Selectable(fbxName.c_str(), is_selected))
							{
								AnimData = iter->first;
							}
						}
						ImGui::EndListBox();

						if (ImGui::Button("Add Component##AnimComp"))
						{
							auto playerCharAnimComp = PlayerCharacter->AddComponent<AnimationComponent>();

							FBXLoader::GetInstance()->GenerateAnimationFromFileData(AnimData, playerCharAnimComp);
						}
					}
				}
				break;

			case 2:  /////////////////////////////// Transform /////////////////////////////////

				if (PlayerCharacter->has<TransformComponent>()) {
					auto transform = PlayerCharacter->GetComponent<TransformComponent>();

					if (ImGui::TreeNode("Translation"))
					{
						ImGui::Text("X");	ImGui::SameLine();
						static float bufferX = 0;
						static float bufferOldX = transform->Translation.x;
						ImGui::InputFloat("##inputX", &bufferX); ImGui::SameLine();
						if (ImGui::Button("Apply##X"))
						{
							bufferOldX = transform->Translation.x;
							transform->Translation.x = bufferX;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##X"))
						{
							bufferX = bufferOldX;
							transform->Translation.x = bufferX;
						}


						ImGui::Text("Y");	ImGui::SameLine();
						static float bufferY = 0;
						static float bufferOldY = transform->Translation.y;
						ImGui::InputFloat("##inputY", &bufferY); ImGui::SameLine();
						if (ImGui::Button("Apply##Y"))
						{
							bufferOldY = transform->Translation.y;
							transform->Translation.y = bufferY;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Y"))
						{
							bufferY = bufferOldY;
							transform->Translation.y = bufferY;
						}

						ImGui::Text("Z");	ImGui::SameLine();
						static float bufferZ = 0;
						static float bufferOldZ = transform->Translation.z;
						ImGui::InputFloat("##inputZ", &bufferZ); ImGui::SameLine();
						if (ImGui::Button("Apply##Z"))
						{
							bufferOldZ = transform->Translation.z;
							transform->Translation.z = bufferZ;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Z"))
						{
							bufferZ = bufferOldZ;
							transform->Translation.z = bufferZ;
						}

						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Rotation"))
					{
						ImGui::Text("Pitch");	ImGui::SameLine();
						static float bufferYaw = 0;
						static float bufferOldYaw = transform->Rotation.x;
						ImGui::InputFloat("##inputYaw", &bufferYaw); ImGui::SameLine();
						if (ImGui::Button("Apply##Yaw"))
						{
							bufferOldYaw = transform->Rotation.x;
							transform->Rotation.x = bufferYaw;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Yaw"))
						{
							bufferYaw = bufferOldYaw;
							transform->Rotation.x = bufferYaw;
						}


						ImGui::Text("Yaw");	ImGui::SameLine();
						static float bufferPitch = 0;
						static float bufferOldPitch = transform->Rotation.y;
						ImGui::InputFloat("##inputPitch", &bufferPitch); ImGui::SameLine();
						if (ImGui::Button("Apply##Pitch"))
						{
							bufferOldPitch = transform->Rotation.y;
							transform->Rotation.y = bufferPitch;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Pitch"))
						{
							bufferPitch = bufferOldPitch;
							transform->Rotation.y = bufferPitch;
						}

						ImGui::Text("Roll");	ImGui::SameLine();
						static float bufferRoll = 0;
						static float bufferOldRoll = transform->Rotation.z;
						ImGui::InputFloat("##inputRoll", &bufferRoll); ImGui::SameLine();
						if (ImGui::Button("Apply##Roll"))
						{
							bufferOldRoll = transform->Rotation.z;
							transform->Rotation.z = bufferRoll;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Roll"))
						{
							bufferRoll = bufferOldRoll;
							transform->Rotation.z = bufferRoll;
						}

						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Scale"))
					{
						ImGui::Text("ScaleX");	ImGui::SameLine();
						static float bufferScaleX = 0;
						static float bufferOldScaleX = transform->Scale.x;
						ImGui::InputFloat("##inputScaleX", &bufferScaleX); ImGui::SameLine();
						if (ImGui::Button("Apply##ScaleX"))
						{
							bufferOldScaleX = transform->Scale.x;
							transform->Scale.x = bufferScaleX;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##ScaleX"))
						{
							bufferScaleX = bufferOldScaleX;
							transform->Scale.x = bufferScaleX;
						}


						ImGui::Text("ScaleY");	ImGui::SameLine();
						static float bufferScaleY = 0;
						static float bufferOldScaleY = transform->Scale.y;
						ImGui::InputFloat("##inputScaleY", &bufferScaleY); ImGui::SameLine();
						if (ImGui::Button("Apply##ScaleY"))
						{
							bufferOldScaleY = transform->Scale.y;
							transform->Scale.y = bufferScaleY;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##ScaleY"))
						{
							bufferScaleY = bufferOldScaleY;
							transform->Scale.y = bufferScaleY;
						}

						ImGui::Text("ScaleZ");	ImGui::SameLine();
						static float bufferScaleZ = 0;
						static float bufferOldScaleZ = transform->Scale.z;
						ImGui::InputFloat("##inputScaleZ", &bufferScaleZ); ImGui::SameLine();
						if (ImGui::Button("Apply##ScaleZ"))
						{
							bufferOldScaleZ = transform->Scale.z;
							transform->Scale.z = bufferScaleZ;
						} ImGui::SameLine();
						if (ImGui::Button("Undo##ScaleZ"))
						{
							bufferScaleZ = bufferOldScaleZ;
							transform->Scale.z = bufferScaleZ;
						}
						ImGui::TreePop();
					}
				}
				else
				{
					ImGui::Text("No Component");
				}
				break;
			case 3://///////////////////////////// SOCKET /////////////////////////////////

				if (PlayerCharacter->has<SocketComponent>()) {
					auto socket = PlayerCharacter->GetComponent<SocketComponent>();

					if (ImGui::TreeNode("Socket Info"))
					{
						ImGui::Text("Linked Bone : "); ImGui::SameLine();
						ImGui::Text(socket->BoneName.c_str());

						Vector3 T = socket->GetTranslation();
						ImGui::Text("Translation");
						ImGui::Text("X : %f / Y : %f / Z : %f", T.x, T.y, T.z);

						Vector3 R = socket->GetRotation();
						ImGui::Text("Rotation");
						ImGui::Text("X : %f / Y : %f / Z : %f", R.x, R.y, R.z);

						Vector3 S = socket->GetScale();
						ImGui::Text("Scale");
						ImGui::Text("X : %f / Y : %f / Z : %f", S.x, S.y, S.z);


						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Set Socket")) //&& PlayerCharacter->has<SkeletalMeshComponent>())
					{
						auto mesh = PlayerCharacter->GetComponent<SkeletalMeshComponent>();
						if (ImGui::TreeNode("Bone List"))
						{
							static std::string BoneName;

							if (ImGui::ListBoxHeader("##BoneList2", ImVec2(500, 500)))
							{
								for (auto iter = mesh->BindPoseMap.begin(); iter != mesh->BindPoseMap.end(); iter++)
								{
									const bool is_selected = (BoneName == iter->first);
									if (ImGui::Selectable(iter->first.c_str(), is_selected))
									{
										BoneName = iter->first;
									}
								}
								ImGui::ListBoxFooter();
							}
							ImGui::TreePop();

							if (ImGui::Button("Attach ##SocketComp"))
							{
								//auto socketComp = PlayerCharacter->AddComponent<SocketComponent>();
								socket->Attach(*mesh, BoneName);
							}
						}

						/*auto skmesh = PlayerCharacter->GetComponent<SkeletalMeshComponent>();
						ImGui::Text("Bone Name");	ImGui::SameLine();
						static char bufferBone[128] = "";
						static char bufferOldBone[128];
						strcpy_s(bufferOldBone, sizeof(bufferOldBone), socket->BoneName.c_str());

						ImGui::InputText("##inputBone", bufferBone, IM_ARRAYSIZE(bufferBone)); ImGui::SameLine();
						if (ImGui::Button("Apply##Bone"))
						{
							strcpy_s(bufferOldBone, sizeof(bufferOldBone), socket->BoneName.c_str());
							socket->Attach(*skmesh, bufferBone);
						} ImGui::SameLine();
						if (ImGui::Button("Undo##Bone"))
						{
							strcpy_s(bufferBone, sizeof(bufferBone), bufferOldBone);
							socket->Attach(*skmesh, bufferBone);
						}*/
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Set Offset"))
					{
						static Vector3 T = socket->GetTranslation();
						ImGui::Text("Translation");
						ImGui::DragFloat("##socketTx", &T.x, 0.1f); ImGui::SameLine();
						ImGui::Text("X");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputTx", &T.x);

						ImGui::DragFloat("##socketTy", &T.y, 0.1f); ImGui::SameLine();
						ImGui::Text("Y");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputTy", &T.y);

						ImGui::DragFloat("##socketTz", &T.z, 0.1f); ImGui::SameLine();
						ImGui::Text("Z");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputTz", &T.z);

						static Vector3 R = socket->GetRotation();
						ImGui::Text("Rotation");
						ImGui::DragFloat("##socketRx", &R.x, 0.1f, -360, 360); ImGui::SameLine();
						ImGui::Text("X");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputRx", &R.x);

						ImGui::DragFloat("##socketRy", &R.y, 0.1f, -360, 360); ImGui::SameLine();
						ImGui::Text("Y");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputRy", &R.y);

						ImGui::DragFloat("##socketRz", &R.z, 0.1f, -360, 360); ImGui::SameLine();
						ImGui::Text("Z");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputRz", &R.z);

						static Vector3 S = socket->GetScale();
						ImGui::Text("Scale");
						ImGui::DragFloat("##socketSx", &S.x, 0.1f, 0.01, 100); ImGui::SameLine();
						ImGui::Text("X");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputSx", &S.x);

						ImGui::DragFloat("##socketSy", &S.y, 0.1f, 0.01, 100); ImGui::SameLine();
						ImGui::Text("Y");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputSy", &S.y);

						ImGui::DragFloat("##socketSz", &S.z, 0.1f, 0.01, 100); ImGui::SameLine();
						ImGui::Text("Z");	ImGui::SameLine();
						ImGui::InputFloat("Frame##inputSz", &S.z);

						static bool adjust = false;
						ImGui::Checkbox("Adjust Offset", &adjust);
						if (adjust == true)
						{
							socket->SetOffset(T, R, S);
						}ImGui::SameLine();
						if (ImGui::Button("Reset"))
						{
							socket->SetOffset({ 0,0,0 }, { 0,0,0 }, { 1,1,1 });
							adjust = false;
						}

						ImGui::TreePop();
					}
					if (ImGui::Button("Remove Component##SocketComp"))
					{
						PlayerCharacter->RemoveComponent<SocketComponent>();
					}
				}
				else
				{
					if (PlayerCharacter->has<SkeletalMeshComponent>())
					{
						auto mesh = PlayerCharacter->GetComponent<SkeletalMeshComponent>();

						ImGui::Text("No Component...Add Socket");

						if (ImGui::TreeNode("Bone List"))
						{
							static std::string BoneName;

							if (ImGui::ListBoxHeader("##BoneList2", ImVec2(500, 500)))
							{
								for (auto iter = mesh->BindPoseMap.begin(); iter != mesh->BindPoseMap.end(); iter++)
								{
									const bool is_selected = (BoneName == iter->first);
									if (ImGui::Selectable(iter->first.c_str(), is_selected))
									{
										BoneName = iter->first;
									}
								}
								ImGui::ListBoxFooter();
							}
							ImGui::TreePop();

							if (ImGui::Button("Add Component##SocketComp"))
							{
								auto socketComp = PlayerCharacter->AddComponent<SocketComponent>();
								socketComp->Attach(*mesh, BoneName);
							}
						}
					}
					else
					{
						ImGui::Text("No SKMesh Component...Add Skeletal Mesh First");
					}
				}
				break;

			case 4:	/////////////////////////////// STATIC MESH /////////////////////////////////


				if (PlayerCharacter->has<StaticMeshComponent>()) 
				{

					auto mesh = PlayerCharacter->GetComponent< StaticMeshComponent>();
					ImGui::Text("Static Mesh Component : ");	ImGui::SameLine();

					int size_needed = WideCharToMultiByte(CP_UTF8, 0, &mesh->FBXName[0], (int)mesh->FBXName.size(), NULL, 0, NULL, NULL);
					std::string fbxName(size_needed, 0);
					WideCharToMultiByte(CP_UTF8, 0, &mesh->FBXName[0], (int)mesh->FBXName.size(), &fbxName[0], size_needed, NULL, NULL);

					ImGui::Text(fbxName.c_str());

					static std::wstring FBXlist;

					if (ImGui::BeginListBox("##FBX List3", ImVec2(500, 250)))
					{

						for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
						{

							int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
							std::string fbxName(size_needed, 0);
							WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

							const bool is_selected = (FBXlist == iter->first);
							if (ImGui::Selectable(fbxName.c_str(), is_selected))
							{
								FBXlist = iter->first;
							}
						}
						ImGui::EndListBox();

						if (ImGui::Button("Change Mesh##StaticMesh"))
						{
							PlayerCharacter->RemoveComponent<StaticMeshComponent>();
							auto weaponMesh = PlayerCharacter->AddComponent<StaticMeshComponent>();
							FBXLoader::GetInstance()->GenerateStaticMeshFromFileData(FBXlist, weaponMesh);
						}
						ImGui::SameLine();
						if (ImGui::Button("Remove Mesh##StaticMesh"))
						{
							PlayerCharacter->RemoveComponent<StaticMeshComponent>();
						}
					}
				}
				else
				{
					ImGui::Text("No Component...Add Static Mesh");
					static std::wstring FBXlist;
					if (ImGui::BeginListBox("##FBX List3", ImVec2(500, 250)))
					{

						for (auto iter = FBXLoader::GetInstance()->GetList()->begin(); iter != FBXLoader::GetInstance()->GetList()->end(); iter++)
						{

							int size_needed = WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), NULL, 0, NULL, NULL);
							std::string fbxName(size_needed, 0);
							WideCharToMultiByte(CP_UTF8, 0, &iter->first[0], (int)iter->first.size(), &fbxName[0], size_needed, NULL, NULL);

							const bool is_selected = (FBXlist == iter->first);
							if (ImGui::Selectable(fbxName.c_str(), is_selected))
							{
								FBXlist = iter->first;
							}
						}
						ImGui::EndListBox();

						if (ImGui::Button("Add Component##StaticMesh"))
						{
							auto weaponMesh = PlayerCharacter->AddComponent<StaticMeshComponent>();
							FBXLoader::GetInstance()->GenerateStaticMeshFromFileData(FBXlist, weaponMesh);
						}
					}
				}
				break;


			case 5:	/////////////////////////////// BOUNDING BOX /////////////////////////////////

				if (PlayerCharacter->has<BoundingBoxComponent>()) {
					auto box = PlayerCharacter->GetComponent< BoundingBoxComponent>();
					ImGui::Text("Bounding Box Component");

					static Vector3 OBBE = box->InitOBB.Extents;
					ImGui::Text("Extents");
					ImGui::DragFloat("Extents##OBBEx", &OBBE.x, 0.05f, 0, 1000); ImGui::SameLine();
					ImGui::Text("X");	ImGui::SameLine();
					ImGui::InputFloat("##OBBEx", &OBBE.x);

					ImGui::DragFloat("Extents##OBBEy", &OBBE.y, 0.05f, 0, 1000); ImGui::SameLine();
					ImGui::Text("Y");	ImGui::SameLine();
					ImGui::InputFloat("##OBBEy", &OBBE.y);

					ImGui::DragFloat("Extents##OBBEz", &OBBE.z, 0.05f, 0, 1000); ImGui::SameLine();
					ImGui::Text("Z");	ImGui::SameLine();
					ImGui::InputFloat("##OBBEz", &OBBE.z);

					ImGui::Text("Center");
					static Vector3 OBBC = box->InitOBB.Center;
					ImGui::DragFloat("Center##OBBCx", &OBBC.x, 0.05f); ImGui::SameLine();
					ImGui::Text("X");	ImGui::SameLine();
					ImGui::InputFloat("##OBBCx", &OBBC.x);

					ImGui::DragFloat("Center##OBBCy", &OBBC.y, 0.05f); ImGui::SameLine();
					ImGui::Text("Y");	ImGui::SameLine();
					ImGui::InputFloat("##OBBCy", &OBBC.y);

					ImGui::DragFloat("Center##OBBCz", &OBBC.z, 0.05f); ImGui::SameLine();
					ImGui::Text("Z");	ImGui::SameLine();
					ImGui::InputFloat("##OBBCz", &OBBC.z);

					box->InitOBB.Extents = OBBE;
					box->InitOBB.Center = OBBC;

#ifdef _DEBUG
					box->pDebugObj->m_vertices.clear();
					box->pDebugObj->setVertices(box->InitOBB);

					D3D11_MAPPED_SUBRESOURCE ms;
					ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));

					HRESULT hr = DXDevice::g_pImmediateContext->Map(box->pDebugObj->m_pVBuf.Get(),
						0,
						D3D11_MAP_WRITE_DISCARD,
						NULL,
						&ms);

					if (SUCCEEDED(hr))
					{
						int size = box->pDebugObj->m_vertices.size() * sizeof(LineVertex);
						memcpy(ms.pData, &box->pDebugObj->m_vertices.at(0), size);

						DXDevice::g_pImmediateContext->Unmap(box->pDebugObj->m_pVBuf.Get(), NULL);
					}

#endif //_DEBUG
				}
				else
				{
					ImGui::Text("No Component");
				}
				break;
				}




			}ImGui::End();
		}



		PickedCharacter = (Character*)MAIN_PICKER.lastSelect.pTarget;











		TheWorld.CleanUp();


		return true;
	}

	bool SampleCore::Render()
	{
		EditorCore::Render();

		float dt = Timer::GetInstance()->SecondPerFrame;
		TheWorld.Tick(dt);

		Input::GetInstance()->cleanUpWheelState();

		return true;
	}

	bool SampleCore::Release()
	{

		BV_MGR.release();

		FBXLoader::GetInstance()->Release();
		return EditorCore::Release();

	}

	void SampleCore::Init_Map()
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

	void SampleCore::Init_Chara()
	{
		PlayerCharacter = new Character;
		
		////auto playerCharMovementComp = PlayerCharacter->GetComponent<MovementComponent>();
		////playerCharMovementComp->Speed = 25.0f;
		////PlayerCharacter->MoveTo(Vector3(-20.0f, 0.0f, 0.0f));


		//////////////// Bounding Box Add /////////////////
		//auto playerOBBComp = PlayerCharacter->AddComponent<BoundingBoxComponent>(Vector3(0.75f, 1.1f, 0.75f), Vector3(0.0f, 1.1f, 0.0f));

		// 플레이어용 카메라 및 카메라 암 설정.
		auto playerCamera = PlayerCharacter->AddComponent<Camera>();
		auto playerCameraArm = PlayerCharacter->AddComponent<CameraArmComponent>();
		playerCameraArm->Distance = 100.0f;
		playerCameraArm->Roll = 35.0f;
		playerCameraArm->Pitch = 180.0f - 40.0f;
		playerCamera->CreateViewMatrix(Vector3(0.0f, 25.0f, -100.0f), Vector3(0.0f, 0.0f, 00.0f), Vector3(0.0f, 1.0, 0.0f));
		playerCamera->CreateProjectionMatrix(1.0f, 10000.0f, PI * 0.25, (DXDevice::g_ViewPort.Width) / (DXDevice::g_ViewPort.Height));



		TheWorld.AddEntity(PlayerCharacter);



	}

