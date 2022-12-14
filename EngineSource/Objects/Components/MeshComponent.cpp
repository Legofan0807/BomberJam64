#include "MeshComponent.h"
#include <Rendering/Mesh/Model.h>
#include <World/Graphics.h>
#include <World/Assets.h>
#include <Rendering/Utility/Framebuffer.h>

void MeshComponent::Start()
{
	if (MeshModel)
	{
		MeshModel->ModelTransform = Parent->GetTransform() + RelativeTransform;
	}
}

void MeshComponent::Destroy()
{
	for (auto* f : Graphics::AllFramebuffers)
	{
		for (int i = 0; i < f->Renderables.size(); i++)
		{
			if (MeshModel == f->Renderables[i])
			{
				f->Renderables.erase(f->Renderables.begin() + i);
			}
		}
	}
	delete MeshModel;
}
void MeshComponent::Tick()
{
	if (AutomaticallyUpdateTransform)
	{
		MeshModel->ModelTransform = Parent->GetTransform() + RelativeTransform;
		MeshModel->UpdateTransform();
	}
}
void MeshComponent::Load(std::string File)
{
	MeshModel = new Model(Assets::GetAsset(File + ".jsm"));
	ModelPath = File;
	Graphics::MainFramebuffer->Renderables.push_back(MeshModel);
	MeshModel->UpdateTransform();
}

FrustumCulling::AABB MeshComponent::GetBoundingBox()
{
	return MeshModel->Size;
}

void MeshComponent::SetUniform(std::string Name, UniformType Type, std::string Content, uint8_t MeshSection)
{
	MeshModel->SetUniform(Uniform(Name, Type, (void*)Content.c_str()), MeshSection);
}

MeshData MeshComponent::GetMeshData()
{
	return MeshModel->ModelMeshData;
}

void MeshComponent::SetRelativeTransform(Transform NewRelativeTransform)
{
	if(Parent)
	RelativeTransform = NewRelativeTransform;
}

Transform& MeshComponent::GetRelativeTransform()
{
	return RelativeTransform;
}

void MeshComponent::SetVisibility(bool NewVisibility)
{
	MeshModel->Visible = NewVisibility;
}

void MeshComponent::UpdateTransform()
{
	if (!AutomaticallyUpdateTransform)
	{
		MeshModel->ModelTransform = Parent->GetTransform() + RelativeTransform;
		MeshModel->UpdateTransform();
	}
}
