#pragma once
#include "Entity.hpp"
#include "MovementComponent.h"
#include "TransformComponent.h"
#include "LightComponent.h"

class Light : public ECS::Entity
{
public:


public:
	Light();
	virtual ~Light();
};

inline Light::Light()
{
	this->AddComponent<TransformComponent>();
	this->Name = L"Light" + std::to_wstring(this->ID);
}

inline Light::~Light()
{

}