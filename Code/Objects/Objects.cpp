#include "Objects.h"
#include <World.h>
#include <Objects/MeshObject.h>
#include <Objects/SoundObject.h>
#include <Objects/InstancedMeshObject.h>
#include <Objects/PlayerObject.h>
#include <Objects/Bomb.h>
#include <Objects/WallObject.h>
#include <Objects/BombPickup.h>
#include <Objects/Orb.h>

template<typename T>
inline T* Objects::SpawnObject(Transform ObjectTransform)
{
	T* NewObject = new T();
	NewObject->Start(NewObject->GetObjectDescription().Name, ObjectTransform);
	NewObject->CurrentScene = World::CurrentScene;
	return NewObject;
}
bool Objects::DestroyObject(WorldObject* Object)
{
	if (Object)
	{
		for (Component* LoopComponent : Object->GetComponents())
		{
			LoopComponent->Destroy();
			delete LoopComponent;
		}
		for (int i = 0; i < Objects::AllObjects.size(); i++)
		{
			if (Object)
			{
				if (Objects::AllObjects.at(i) == Object)
				{
					Objects::AllObjects.erase(Objects::AllObjects.begin() + i);
				}
			}
		}
		Object->Destroy();
		delete Object;
		return true;
	}
	return false;
}
WorldObject* Objects::SpawnObjectFromID(uint32_t ID, Transform ObjectTransform)
{
	switch (ID)
	{
	case 0:
		return (WorldObject*)SpawnObject<WorldObject>(ObjectTransform);
	case 1:
		return (WorldObject*)SpawnObject<MeshObject>(ObjectTransform);
	case 2:
		return (WorldObject*)SpawnObject<SoundObject>(ObjectTransform);
	case 3:
		return (WorldObject*)SpawnObject<InstancedMeshObject>(ObjectTransform);
	case 4:
		return (WorldObject*)SpawnObject<PlayerObject>(ObjectTransform);
	case 5:
		return (WorldObject*)SpawnObject<Bomb>(ObjectTransform);
	case 6:
		return (WorldObject*)SpawnObject<WallObject>(ObjectTransform);
	case 7:
		return (WorldObject*)SpawnObject<BombPickup>(ObjectTransform);
	case 8:
		return (WorldObject*)SpawnObject<Orb>(ObjectTransform);
	default:
		throw "Attempted to spawn unknown object";
	}
}


namespace Objects
{
	const std::vector<ObjectDescription> EditorObjects
	{
		ObjectDescription("Mesh Object", 1),
		ObjectDescription("Sound Object", 2),
		ObjectDescription("Instanced Mesh", 3),
		ObjectDescription("Player Object", 4),
		ObjectDescription("Bomb", 5),
		ObjectDescription("Wall Object", 6),
		ObjectDescription("Bomb Pickup", 7),
		ObjectDescription("Orb", 8)
	};
}