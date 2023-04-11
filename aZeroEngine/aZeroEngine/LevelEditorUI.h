#pragma once
#include "UserInterface.h"
#include "Engine.h"

class LevelEditorUI : public UserInterface
{
private:
	int selEntityID = -1;
	int selPBRMatID = -1;
	int selMeshID = -1;
	int selTexture2DID = -1;

	std::string selPBRMatName = "";
	std::string selMeshName = "";
	std::string selTexture2DName = "";

	bool editingPLight = false;
	bool editorMode = false;

	std::weak_ptr<Camera> camera;
	std::shared_ptr<Scene> currentScene = nullptr;
	aZero::Engine* engine = nullptr;

private:
	void SetupVisuals();

	void ShowSceneWindow();
	void ShowEntityWindow();
	void ShowMaterialWindow();
	void ShowResourceWindow();

	void DrawEntityComponents(Entity& _entity);
	std::optional<std::string> LoadMeshFromDirectory();
	std::optional<std::string> LoadPBRMaterialFromDirectory();

public:

	LevelEditorUI(const std::string& _name, aZero::Engine* _engine)
		:UserInterface(_name), engine(_engine) 
	{
		SetupVisuals();
	}

	virtual ~LevelEditorUI() { currentScene.reset(); }

	virtual void BeginFrame() override { ImGuizmo::Enable(editorMode); }

	virtual void Update() override;

	void ToggleEditorMode() { editorMode = !editorMode; }
	void SetCamera(std::shared_ptr<Camera> _camera) { camera = _camera; }
	void SetSelectedEntity(int _id) { selEntityID = _id; }
};