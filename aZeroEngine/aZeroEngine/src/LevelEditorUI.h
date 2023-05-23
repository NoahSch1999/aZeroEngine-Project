#pragma once
#include "UserInterface.h"
#include "Engine.h"
#include "SelectionList.h"

class LevelEditorUI : public UserInterface
{
private:
	bool m_currentlyPlaying = false;

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
	void showApplicationSettings();

	void SetupVisuals();

	void ShowSceneWindow();
	
	void ShowEntityWindow();
	void ShowEntityHierarchy(const aZeroECS::Entity& _current);

	void ShowMaterialWindow();
	void showMeshes();
	void showFileTextures();

	void DrawEntityComponents(aZeroECS::Entity& _entity);
	
	int applyTexturePopup(int ID);

	std::optional<std::string> LoadModelDataFromDirectory();

	std::optional<std::string> LoadPBRMaterialFromDirectory();

public:
	SelectionList selectionList;
	std::shared_ptr<Scene> currentScene = nullptr;

	void play(bool startStop)
	{
		engine->GetPhysicSystem().lock()->simulatePhysics(startStop);

		if (!startStop)
		{
			GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
			const std::string name = currentScene->GetName();
			selectionList.Clear();
			currentScene = engine->LoadScene(context, "../scenes/", name);
			engine->GetCommandManager().executeContext(context);
		}
	}

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

	void materialDragDropViewport()
	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();

		if (payload)
		{
			if (InputManager::MouseBtnUp(LEFT))
			{
				if (payload->IsDataType("MatDragDrop"))
				{
					int matId = *static_cast<int*>(payload->Data);

					std::optional<DXM::Vector2> mousePos = engine->GetMouseWindowPosition();
					if (mousePos)
					{
						int pickingID = engine->GetPickingEntityID(mousePos.value().x, mousePos.value().y);
						if (pickingID != -1)
						{
							MaterialComponent* matComponent = currentScene->GetComponentForEntity<MaterialComponent>(currentScene->GetEntity(pickingID));
							if (matComponent)
							{
								matComponent->materialID = matId;
							}
						}
					}
				}
			}
		}
		
	}

	void EntitySelection()
	{
		if (InputManager::MouseBtnDown(LEFT))
		{
			if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			{
				if (!ImGuizmo::IsOver() || selectionList.Empty())
				{
					std::optional<DXM::Vector2> mousePos = engine->GetMouseWindowPosition();
					if (mousePos)
					{
						int pickingID = engine->GetPickingEntityID(mousePos.value().x, mousePos.value().y);

						if (pickingID == -1)
						{
							selectionList.Clear();
						}
						else
						{
							if (InputManager::KeyHeld(VK_SHIFT))
							{
								if (selectionList.Selected(pickingID))
									selectionList.Remove(pickingID);
								else
									selectionList.Add(pickingID);
							}
							else
							{
								selectionList.Clear();
								selectionList.Add(pickingID);
							}
						}
					}
				}
			}
		}
	}
};