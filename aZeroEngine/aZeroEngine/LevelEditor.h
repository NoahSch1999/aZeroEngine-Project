#pragma once
#include "Engine.h"
#include "LevelEditorUI.h"

class LevelEditor
{
private:
	std::unique_ptr<aZero::Engine> engine;
	std::shared_ptr<LevelEditorUI> ui;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Camera> cameraTwo;

public:

	LevelEditor(HINSTANCE _appInstance, UINT _width, UINT _height)
	{
		engine = std::make_unique<aZero::Engine>(_appInstance, _width, _height);
		ui = std::make_shared<LevelEditorUI>("LevelEditorUI", engine.get());
		engine->AttachUI(ui);

		camera = std::make_shared<Camera>(engine->GetResourceEngine(), 0.4f * 3.14f, engine->GetWindowApsectRatio());

		ui->SetCamera(camera);
		engine->SetCamera(camera);
	}

	~LevelEditor()
	{
	}

	void Run()
	{
		Timer timer;
		timer.StartCountDown();

		std::shared_ptr<Scene> testScene = nullptr;

		while (!WINDOWQUIT)
		{
			timer.Update();

			engine->BeginFrame();

			static bool editorMode = false;
			if (InputManager::KeyDown(VK_ESCAPE))
			{
				editorMode = !editorMode;
				ui->ToggleEditorMode();
				engine->DisplayCursor(editorMode);
			//	engine->ConfineCursor(!editorMode);
				if(camera)
					camera->ToggleActive(!editorMode);
			}

			if (camera)
			{
				if (camera->Active())
				{
					Vector2 clientDimensions = engine->GetClientWindowSize();
					camera->Update(timer.deltaTime, (float)clientDimensions.x / (float)clientDimensions.y);
				}
			}

			/*if (!ImGui::IsAnyItemHovered())
			{
				if (InputManager::MouseBtnDown(LEFT))
				{
					std::optional<Vector2> mousePos = engine->GetMouseWindowPosition();
					if (mousePos)
					{
						ui->SetSelectedEntity(engine->GetPickingEntityID(mousePos.value().x, mousePos.value().y));
					}
				}
			}*/

			if (InputManager::KeyHeld('R') && InputManager::KeyHeld(VK_CONTROL))
			{
				camera.reset();
				camera = std::make_shared<Camera>(engine->GetResourceEngine(), 0.4f * 3.14f, engine->GetWindowApsectRatio());
				camera->GetPosition().z = -2.f;
				Vector2 clientDimensions = engine->GetClientWindowSize();
				camera->Update((float)clientDimensions.x / (float)clientDimensions.y);
				engine->SetCamera(camera);
				ui->SetCamera(camera);
			}

			engine->Render();
			engine->EndFrame();
			InputManager::EndFrame();
		}
	}
};