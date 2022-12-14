#pragma once
#include "Objects/WorldObject.h"
#include <Objects/Components/CollisionComponent.h>
#include <Objects/Components/MeshComponent.h>
#include "Rendering/Mesh/Model.h"
#include <GENERATED/GENERATED_MeshObject.h>
class MeshObject : public WorldObject
{
public:
	MESHOBJECT_GENERATED()

	virtual void Destroy();
	virtual void Tick();
	virtual void Begin();
	void LoadFromFile(std::string Filename);
	virtual void OnPropertySet() override;

	virtual std::string Serialize() override;
	virtual void Deserialize(std::string SerializedObject) override;
	bool HasCollision = true;
protected:
	MeshComponent* Mesh = nullptr;
	CollisionComponent* MeshCollision = nullptr;
	Vector3 CollisionScale = Vector3(1);
	std::string Filename;
	std::string MaterialOverride;
};