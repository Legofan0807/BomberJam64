#pragma once
#include <Objects/Components/MeshComponent.h>
#include <Math/Collision/Collision.h>


class CollisionComponent : public Component
{
public:
	Collision::HitResponse OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore = {});
	CollisionComponent() {}
	Transform RelativeTransform;
	virtual void Destroy() override;
	void Tick() override;
	void Init(std::vector<Vertex> Vertices, std::vector<int> Indices, Transform RelativeTransform = Transform());
	Collision::CollisionMesh CollMesh;
	Transform LastParentTransform, TestTransform;
};