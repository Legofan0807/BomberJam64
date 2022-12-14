#include "OrbGate.h"
#include <Engine/Save.h>
#include <Engine/Log.h>
#include <World/Stats.h>

std::vector<std::string> Textures =
{
	"Gates_X10",
	"Gates_X20",
	"Gates_X30",
	"Gates_X50"
};

std::vector<unsigned int> RequiredAmounts =
{
	10,
	20,
	40,
	50
};

void OrbGate::Begin()
{
	GateMeshComponent = new MeshComponent();
	Attach(GateMeshComponent);
	GateMeshComponent->Load("OrbGate");
	GateMeshComponent->SetUniform("u_diffuse", U_VEC3, Vector3(1, 1, 0.8).ToString(), 0);

	Properties.push_back(Object::Property("Gate Index", T_INT, &TextureIndex));
}

void OrbGate::OnPropertySet()
{
	GateMeshComponent->SetUniform("u_texture", U_TEXTURE, Textures[TextureIndex], 0);
	GateMeshComponent->SetUniform("u_usetexture", U_INT, std::to_string(1), 0);
	SaveGame MainSaveGame = SaveGame("Main");

	if (!IsInEditor)
	{
		if (!MainSaveGame.SaveGameIsNew())
		{
			if (std::stoi(MainSaveGame.GetPropterty("OrbsCollected").Value) >= RequiredAmounts[TextureIndex])
			{
				Objects::DestroyObject(this);
				return;
			}
		}
	}
	GateCollisionComponent = new CollisionComponent();
	Attach(GateCollisionComponent);
	GateCollisionComponent->Init(GateMeshComponent->GetMeshData().Vertices, GateMeshComponent->GetMeshData().Indices);
}