#pragma once
#include "UserInterface.h"
#include "Engine.h"
#include "SelectionList.h"

class LevelEditorUI : public UserInterface
{
private:
	//int selEntityID = -1;
	int selPBRMatID = -1;
	int selMeshID = -1;
	int selTexture2DID = -1;

	std::string selPBRMatName = "";
	std::string selMeshName = "";
	std::string selTexture2DName = "";

	bool editingPLight = false;
	bool editorMode = false;

	std::weak_ptr<Camera> camera;
	aZero::Engine* engine = nullptr;

private:
	void ShowPerformanceData();
	void showApplicationInfo();

	void SetupVisuals();

	void ShowSceneWindow();
	
	void ShowEntityWindow();
	void ShowEntityHierarchy(const aZeroECS::Entity& _current);

	void ShowMaterialWindow();
	void ShowResourceWindow();

	void DrawEntityComponents(aZeroECS::Entity& _entity);
	std::optional<std::string> LoadMeshFromDirectory();
	std::optional<std::string> LoadPBRMaterialFromDirectory();

public:
	SelectionList selectionList;
	std::shared_ptr<Scene> currentScene = nullptr;

	LevelEditorUI(const std::string& _name, aZero::Engine* _engine)
		:UserInterface(_name), engine(_engine) 
	{
		SetupVisuals();
		//currentScene = engine->LoadScene("../scenes/", "NewScene");
	}

	virtual ~LevelEditorUI() { currentScene.reset(); }

	virtual void BeginFrame() override { ImGuizmo::Enable(editorMode); }

	virtual void Update() override;

	void ToggleEditorMode() { editorMode = !editorMode; }
	void SetCamera(std::shared_ptr<Camera> _camera) { camera = _camera; }
};