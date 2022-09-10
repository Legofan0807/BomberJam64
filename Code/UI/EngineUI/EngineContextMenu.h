#pragma once
#include <UI/Default/UICanvas.h>
#include <WorldParameters.h>
#include <UI/Default/VectorInputField.h>
#include <UI/Default/TextRenderer.h>
#include <UI/Default/ScrollObject.h>
class EngineContextMenu : UICanvas
{
public:
	EngineContextMenu(TextRenderer* Renderer) : UICanvas()
	{
		this->Renderer = Renderer;
		TransformTextFields.push_back(new VectorInputField(Vector2(0.7, -0.5), Vector2(0.045f, 0.08), Vector3(1), 1, Renderer, this, Vector3()));
		UIElements.push_back(TransformTextFields.at(0));
		TransformTextFields.push_back(new VectorInputField(Vector2(0.7, -0.6), Vector2(0.045f, 0.08), Vector3(1), 2, Renderer, this, Vector3()));
		UIElements.push_back(TransformTextFields.at(1));
		TransformTextFields.push_back(new VectorInputField(Vector2(0.7, -0.7), Vector2(0.045f, 0.08), Vector3(1), 3, Renderer, this, Vector3()));
		UIElements.push_back(TransformTextFields.at(2));
		for (VectorInputField* t : TransformTextFields)
		{
			t->CurrentScrollObject = &ContextMenuScrollObject;
		}
	}
	void OnButtonClicked(int Index) override;
	void Render(Shader* Shader) override;
	virtual ~EngineContextMenu();
	void SetObject(WorldObject* Object);
	void Generate();
	WorldObject* CurrentObject = nullptr;
protected:
	Transform LastTransform;
	TextRenderer* Renderer;
	std::vector<Vector2> TextLocations;
	std::vector<UIBorder*> Properties;
	std::vector<VectorInputField*> TransformTextFields;
	ScrollObject ContextMenuScrollObject = ScrollObject(Vector2(1.f, -0.25f), Vector2(0.3, 0.75), 15);
	friend WorldObject;
};