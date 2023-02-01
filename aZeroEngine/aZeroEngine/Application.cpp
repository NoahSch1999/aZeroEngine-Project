#include "Application.h"
#include <iostream>
#include <chrono>

Application::Application(HINSTANCE _instance, int _width, int _height)
{
	Initialize(_instance, _width, _height);
}

void Application::Initialize(HINSTANCE _instance, int _width, int _height)
{
	window = new AppWindow(WndProc, _instance, _width, _height, L"../sprites/snowflake.ico", L"../sprites/snowflakeRed.ico");
	graphics = new Graphics(window, _instance);
	input = new Input(_instance, window->windowHandle);
	sc = graphics->swapChain;
	win = window;
	init = true;
}

void Application::Run()
{
	performanceTimer.StartCountDown();
	int lastSecond = 0;
	int currentSecond = 0;
	int totalFrames = 0;

	//input->mouseDevice->Unacquire();
	//std::cout << ShowCursor(false) << "\n";

	while (true)
	{
		performanceTimer.Update();

		if (!window->Update())
			break;

		if (input->KeyDown(DIK_ESCAPE))
		{
			break;
		}

		input->Update();

		static bool editorMode = false;

		if (input->KeyDown(DIK_E))
		{
			PointLight pt;
			pt.color = Vector3(1.f, 0.1, 0.1);
			pt.position = Vector4(0, 2, 0, 1);
			pt.strenght = 5;
			int index = 1;
			graphics->lManager->AddLight<PointLight>(pt, index);

			if (editorMode)
			{
				//graphics->vbCache->FreeReferenceID(graphics->testMeshID);
				//graphics->testMeshID = graphics->vbCache->GetReferenceID("cube");
				//input->mouseDevice->Unacquire();
				//std::cout << ShowCursor(false) << "\n";
				//graphics->testIDMaterial = graphics->materialManager.GetMaterialIDByName<PhongMaterial>("testMaterial");
				graphics->materialManager.GetMaterial<PhongMaterial>("otherMaterial")->GetInfoPtr()->diffuseTextureID =
					graphics->textureCache->GetResource("brickAlbedo.png")->GetHandle().GetHeapIndex();
			}
			else
			{
				//graphics->vbCache->FreeReferenceID(graphics->testMeshID);
				//graphics->testMeshID = graphics->vbCache->GetReferenceID("sphere");
				//input->mouseDevice->Unacquire();
				//std::cout << ShowCursor(true) << "\n";
				//graphics->testIDMaterial = graphics->materialManager.GetMaterialIDByName<PhongMaterial>("otherMaterial");
				graphics->materialManager.GetMaterial<PhongMaterial>("otherMaterial")->GetInfoPtr()->diffuseTextureID =
					graphics->textureCache->GetResource("sadcat.png")->GetHandle().GetHeapIndex();
			}

			graphics->materialManager.GetMaterial<PhongMaterial>("otherMaterial")->Update(&graphics->directCmdList, graphics->frameIndex);
			editorMode = !editorMode;
		}

		if (!editorMode)
		{
			graphics->camera->Update(performanceTimer.deltaTime, *input, window->width, window->height, &graphics->directCmdList, graphics->frameIndex);


			//memcpy((char*)graphics->lManager->directionalLights.mappedBuffer, &temp, sizeof(temp));

			//PhongMaterialInformation* ptr = graphics->materialManager.GetMaterial<PhongMaterial>("testMaterial")->GetInfoPtr();
			//ptr->ambientAbsorbation = Vector3(0, 0, 1);
			//ptr->specularAbsorbation = Vector3(0, 1, 1);
			//ptr->specularShine = 0.5f;
			//graphics->materialManager.GetMaterial<PhongMaterial>("testMaterial")->Update(graphics->frameIndex);
		}
		else
		{
			graphics->camera->Update(performanceTimer.deltaTime, *input, window->width, window->height, &graphics->directCmdList, graphics->frameIndex);


			//memcpy((char*)graphics->lManager->directionalLights.mappedBuffer, &temp, sizeof(temp));

			//PhongMaterialInformation* ptr = graphics->materialManager.GetMaterial<PhongMaterial>("testMaterial")->GetInfoPtr();
			//ptr->ambientAbsorbation = Vector3(1, 0, 1);
			//ptr->specularAbsorbation = Vector3(1, 1, 0);
			//ptr->specularShine = 0.5f;
			//graphics->materialManager.GetMaterial<PhongMaterial>("testMaterial")->Update(graphics->frameIndex);
		}

		if (input->KeyDown(DIK_V))
		{
			graphics->scene->Save("C:/Users/Noah Schierenbeck/Desktop/Test", "Level1");
		}
		if (input->KeyDown(DIK_B))
		{
			graphics->scene->Load(graphics->device, &graphics->directCmdList, graphics->frameIndex, "C:/Users/Noah Schierenbeck/Desktop/Test", "Level1");
		}

		static bool t = false;
		if (!t)
		{
			if (input->KeyDown(DIK_N))
			{
				int id = graphics->materialManager.GetReferenceID<PhongMaterial>("otherMaterial");
				graphics->scene->GetComponentForEntity<MaterialComponent>(graphics->scene->entities.at("Entity_0"))->materialID = id;
				t = !t;
			}
		}
		else
		{
			if (input->KeyDown(DIK_N))
			{
				int id = graphics->materialManager.GetReferenceID<PhongMaterial>("defaultMaterial");
				graphics->scene->GetComponentForEntity<MaterialComponent>(graphics->scene->entities.at("Entity_0"))->materialID = id;
				t = !t;
			}
		}
		

		// Rendering
		graphics->Begin();

		graphics->Render(window);

		graphics->Present();
		
	}
}

Application::~Application()
{
	delete window;
	delete graphics;
}
