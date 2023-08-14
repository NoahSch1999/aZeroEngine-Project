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

		{
			GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
			//ui->currentScene = engine->LoadScene(context, "../scenes/", "Epic Scene");
			ui->currentScene = engine->LoadScene(context, "../scenes/", "Megascans Scene");
			engine->GetCommandManager().executeContext(context);
			engine->GetCommandManager().flushCPU();
		}

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
				
				DXM::Vector2 clientDimensions = engine->getSwapChain()->getBackBufferDimensions();
				CopyContextHandle context = engine->GetCommandManager().getCopyContext();
				camera->Update(timer.deltaTime, (float)clientDimensions.x / (float)clientDimensions.y, 
					context.getList(), engine->GetFrameIndex(), !editorMode);
				engine->GetCommandManager().executeContext(context);
			}

			if (InputManager::MouseBtnDown(LEFT) && !editorMode)
			{
				static int count = 0;
				DXM::Vector3 forward = camera->GetForward() * 2.f;
				const std::string name = "EntityShoot" + std::to_string(count);
				aZeroECS::Entity& entity = ui->currentScene->CreateEntity(name);
				ui->currentScene->GetComponentForEntity<Transform>(entity)->GetTranslation() = camera->GetPosition();
				ui->currentScene->GetComponentForEntity<Transform>(entity)->GetScale() = { 0.01, 0.01, 0.01 };
				ui->currentScene->AddComponentToEntity<Mesh>(entity, Mesh(engine->GetModelCache().GetID("defaultSphere")));
				ui->currentScene->AddComponentToEntity<MaterialComponent>(entity, MaterialComponent(engine->GetMaterialManager().GetReferenceID<PBRMaterial>("Rock Mat")));
				ui->currentScene->AddComponentToEntity<RigidBody>(entity);
				RigidBody* rbComp = ui->currentScene->GetComponentForEntity<RigidBody>(entity);
				rbComp->m_body->applyLocalForceAtCenterOfMass({ forward.x, forward.y, forward.z });
				engine->GetPhysicSystem().lock()->addSphereCollider(*rbComp, "Collider");
				
				count++;
			}

			if (InputManager::KeyHeld('R') && InputManager::KeyHeld(VK_CONTROL))
			{
				camera.reset();
				camera = std::make_shared<Camera>(engine->GetDevice(), engine->GetResourceTrashcan(), 0.4f * 3.14f, engine->GetWindowApsectRatio());
				camera->GetPosition().z = -2.f;
				DXM::Vector2 clientDimensions = engine->GetClientWindowSize();
				{
					CopyContextHandle context = engine->GetCommandManager().getCopyContext();
					camera->Update(context.getList(), engine->GetFrameIndex(), (float)clientDimensions.x / (float)clientDimensions.y);
					engine->GetCommandManager().executeContext(context);
				}
				engine->SetCamera(camera);
				ui->SetCamera(camera);
			}

			if (!ui->selectionList.Empty())
			{
				if (InputManager::KeyDown('D') && InputManager::KeyHeld(VK_CONTROL))
				{
					aZeroECS::Entity& srcCopyEntity = ui->currentScene->GetEntity(ui->selectionList.GetRoot());
					aZeroECS::Entity& newEntity = ui->currentScene->DuplicateEntity(srcCopyEntity);
					ui->selectionList.Clear();
					if (engine->GetComponentManager().HasComponent<Mesh>(newEntity))
					{
						ui->selectionList.Add(newEntity.m_id);
					}
				}
			}

			engine->GetRenderSystem().lock()->currentScene = ui->currentScene.get();
			engine->Render();

			ui->EntitySelection();
			ui->materialDragDropViewport();

			engine->EndFrame();

			InputManager::EndFrame();
		}
	}
};