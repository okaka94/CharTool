#include "RenderSystem.h"
//#include "Define.h"
#include "World.hpp"
#include "StaticMeshComponent.h"
#include "SkeletalMeshComponent.h"
#include "DXSamplerState.hpp"
#include "TransformComponent.h"
#include "Landscape.h"
//#include "DebugCamera.h"
#include "BoundingBoxComponent.h"
#include "BoundingSphereComponent.h"
#include "EffectInclude/EffectSystem.h"
#include "SocketComponent.h"

void RenderSystem::Tick(ECS::World* world, float time)
{
	ID3D11RasterizerState* pOldRSState;
	DXDevice::g_pImmediateContext->RSGetState(&pOldRSState);
	DXDevice::g_pImmediateContext->RSSetState(DXSamplerState::pDefaultRSSolid);
	DXDevice::g_pImmediateContext->OMSetBlendState(DXSamplerState::pBlendSamplerState, 0, -1);
	DXDevice::g_pImmediateContext->PSSetSamplers(0, 1, &DXSamplerState::pDefaultSamplerState);
	DXDevice::g_pImmediateContext->OMSetDepthStencilState(DXSamplerState::pDefaultDepthStencil, 0xff);

	for (auto& entity : world->GetEntities<LandscapeComponents, TransformComponent>())
	{
		auto landscape = entity->GetComponent<LandscapeComponents>();
		auto transform = entity->GetComponent<TransformComponent>();
		if ((landscape != nullptr) && (transform != nullptr))
		{
			landscape->SetCamera(MainCamera);
			landscape->UpdateTransformMatrix(*transform);
			landscape->Render();
		}
	}

	for (auto& entity : world->GetEntities<SkeletalMeshComponent>())
	{
		auto skeletalMesh = entity->GetComponent<SkeletalMeshComponent>();
		auto transform = entity->GetComponent<TransformComponent>();

		if ((skeletalMesh != nullptr) && (transform != nullptr))
		{
			skeletalMesh->UpdateTransformMatrix(*transform);
			skeletalMesh->Render();
		}

	}


	for (auto& entity : world->GetEntities<StaticMeshComponent, TransformComponent>())
	{
		auto staticMesh = entity->GetComponent<StaticMeshComponent>();
		auto transform = entity->GetComponent<TransformComponent>();
		if ((staticMesh != nullptr) && (transform != nullptr))
		{
			if (entity->has<SocketComponent>())
			{
				auto socket = entity->GetComponent<SocketComponent>();
				staticMesh->UpdateTransformData(socket->GetTransformMatrix());
				staticMesh->Render();
			}
			else
			{
				staticMesh->UpdateTransformMatrix(*transform);
				staticMesh->Render();
			}
		}
	}


//�ٿ�� ���� ���� �׽�Ʈ�� �ڵ�
#ifdef _DEBUG
	for (auto& entity : world->GetEntities<BoundingBoxComponent>())
	{
		auto BVolume = entity->GetComponent<BoundingBoxComponent>();

		if (BVolume != nullptr && MainCamera != nullptr)
		{
			if (BVolume->pDebugObj)
			{
				BVolume->pDebugObj->update(nullptr, &MainCamera->View, &MainCamera->Projection);
				BVolume->pDebugObj->render();
			}
		}
	}

	for (auto& entity : world->GetEntities<BoundingSphereComponent>())
	{
		auto BVolume = entity->GetComponent<BoundingSphereComponent>();

		if (BVolume != nullptr && MainCamera != nullptr)
		{
			if (BVolume->pDebugObj)
			{
				BVolume->pDebugObj->update(nullptr, &MainCamera->View, &MainCamera->Projection);
				BVolume->pDebugObj->render();
			}
		}
	}
#endif //_DEBUG


	DXDevice::g_pImmediateContext->GSSetShader(NULL, NULL, 0);

	DXDevice::g_pImmediateContext->RSSetState(pOldRSState);
	if (pOldRSState != nullptr)
	{
		pOldRSState->Release();
	}
}