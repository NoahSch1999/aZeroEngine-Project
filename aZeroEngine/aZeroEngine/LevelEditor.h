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

		camera = std::make_shared<Camera>(engine->GetDevice(), engine->GetResourceTrashcan(), 0.4f * 3.14f, engine->GetWindowApsectRatio());

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

		/*engine->GetResourceEngine().GetCopy();
		ui->currentScene = engine->LoadScene("../scenes/", "NewScene");
		engine->GetResourceEngine().ExecuteCopy();*/

		engine->GetRenderSystem().lock()->uiSelectionList = &ui->selectionList;

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

				if(camera)
					camera->ToggleActive(!editorMode);
			}

			if (camera)
			{
				if (camera->Active())
				{
					Vector2 clientDimensions = engine->GetClientWindowSize();
					CopyContextHandle context = engine->GetCommandManager().GetCopyContext();
					camera->Update(timer.deltaTime, (float)clientDimensions.x / (float)clientDimensions.y, 
						context.GetList(), engine->GetFrameIndex());
					engine->GetCommandManager().ExecuteContext(context);
				}
			}

			if (InputManager::KeyHeld('R') && InputManager::KeyHeld(VK_CONTROL))
			{
				camera.reset();
				camera = std::make_shared<Camera>(engine->GetDevice(), engine->GetResourceTrashcan(), 0.4f * 3.14f, engine->GetWindowApsectRatio());
				camera->GetPosition().z = -2.f;
				Vector2 clientDimensions = engine->GetClientWindowSize();
				{
					CopyContextHandle context = engine->GetCommandManager().GetCopyContext();
					camera->Update(context.GetList(), engine->GetFrameIndex(), (float)clientDimensions.x / (float)clientDimensions.y);
					engine->GetCommandManager().ExecuteContext(context);
				}
				engine->SetCamera(camera);
				ui->SetCamera(camera);
			}

			engine->GetRenderSystem().lock()->currentScene = ui->currentScene.get();
			engine->Render();
			engine->EndFrame();
			InputManager::EndFrame();
		}
	}
};