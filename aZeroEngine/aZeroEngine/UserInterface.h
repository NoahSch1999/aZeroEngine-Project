#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_win32.h"
#include "../ImGui/backends/imgui_impl_dx12.h"
#include <dxgi1_4.h>
#include <tchar.h>
#include "Scene.h"
#include <Psapi.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#include "Graphics.h"


using Task = std::function<void()>;

class UserInterface
{
private:
	DescriptorHandle heapHandle;
	ImGuiIO io;
protected:
	Graphics* graphics;
	ImVec4 clearColor;

public:
	UserInterface(Graphics* _graphics, HWND _wHandle)
	{
		graphics = _graphics;
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(_wHandle);
		heapHandle = _graphics->resourceManager.GetPassDescriptor();
		ImGui_ImplDX12_Init(_graphics->device, 3, DXGI_FORMAT_B8G8R8A8_UNORM, _graphics->resourceManager.GetResourceHeap(), heapHandle.GetCPUHandle(), heapHandle.GetGPUHandle());

		clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	}

	void BeginFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void Render(CommandList* _cmdList)
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _cmdList->graphic);
	}

};

class EditorUI : public UserInterface
{
private:

public:

	

	std::string selectedEntityStr = "";
	std::string lastSelectedEntityStr = "";
	int selectedEntityID = -1;

	EditorUI(Graphics* _graphics, HWND _wHandle)
		:UserInterface(_graphics, _wHandle)
	{

	}

	void Update()
	{
		ImGui::Begin("Entity Editor");

		// Available entities && entity selection
		ImGui::ListBoxHeader("Entities");
		for (auto [name, index] : graphics->scene->entities.GetStringToIndexMap())
		{
			if (ImGui::Selectable(name.c_str()))
			{
				selectedEntityStr = name;
				selectedEntityID = index;
				break;
			}
		}
		ImGui::ListBoxFooter();

		ImGui::Text("Selected Entity: ");
		ImGui::SameLine();
		ImGui::Text(selectedEntityStr.c_str());
		//

		// Create new entity
		static int counter = 0;
		if (ImGui::Button("Create Entity"))
		{
			graphics->WaitForGPU();
			Entity& ent = graphics->scene->CreateEntity(graphics->device, &graphics->directCmdList);
			Mesh m;
			m.vbIndex = graphics->scene->GetVertexBufferCache()->GetBufferIndex("goblin");
			graphics->scene->AddComponentToEntity<Mesh>(ent, m);
			MaterialComponent mat;
			mat.materialID = graphics->scene->GetMaterialManager()->GetReferenceID<PhongMaterial>("otherMaterial");
			graphics->scene->AddComponentToEntity<MaterialComponent>(ent, mat);
			graphics->scene->GetComponentForEntity<Transform>(ent)->Update(&graphics->directCmdList, Matrix::CreateTranslation(0, counter, 0), graphics->frameIndex);
			graphics->renderSystem->BindFast(ent);
			counter++;
		}
		//

		// Show Components for the selected entity
		if (selectedEntityID != -1)
		{
			const char* comps[] = { "Transform", "Mesh", "Material" };
			if (ImGui::TreeNode("Components"))
			{
				for (int i = 0; i < 3; i++)
				{
					if (ImGui::TreeNode((void*)(intptr_t)i, comps[i]))
					{
						switch (i)
						{
						case 0:
						{
							graphics->WaitForGPU();

							Transform* tf = graphics->scene->GetComponentForEntity<Transform>(graphics->scene->entities.Get(selectedEntityID));
							Vector3 translation;
							Quaternion rotQuat;
							Vector3 rotation;
							Vector3 scale;
							tf->worldMatrix.Decompose(scale, rotQuat, translation);

							rotation = rotQuat.ToEuler();

							static float pos[3]{ translation.x, translation.y, translation.z };
							if (ImGui::InputFloat3("Translation", pos))
							{
								translation.x = pos[0];
								translation.y = pos[1];
								translation.z = pos[2];
								tf->worldMatrix = (Matrix::CreateRotationX(rotation.x) * Matrix::CreateRotationY(rotation.y) * Matrix::CreateRotationZ(rotation.z)) * Matrix::CreateScale(scale) * Matrix::CreateTranslation(translation);
								tf->Update(&graphics->directCmdList, graphics->frameIndex);
							}

							static float rot[3]{ rotation.x, rotation.y, rotation.z };
							if (ImGui::InputFloat3("Rotation", rot))
							{
								tf->worldMatrix = (Matrix::CreateRotationX(rot[0]) * Matrix::CreateRotationY(rot[1]) * Matrix::CreateRotationZ(rot[2])) * Matrix::CreateScale(scale) * Matrix::CreateTranslation(translation);
								tf->Update(&graphics->directCmdList, graphics->frameIndex);
							}

							static float sca[3]{ scale.x, scale.y, scale.z };
							if (ImGui::InputFloat3("Scale", sca))
							{
								scale.x = sca[0];
								scale.y = sca[1];
								scale.z = sca[2];
								tf->worldMatrix = (Matrix::CreateRotationX(rot[0]) * Matrix::CreateRotationY(rot[1]) * Matrix::CreateRotationZ(rot[2])) * Matrix::CreateScale(scale) * Matrix::CreateTranslation(translation);
								tf->Update(&graphics->directCmdList, graphics->frameIndex);
							}

							
							
							break;
						}
						}

						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		//


		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::ShowDemoWindow();

		ImGui::End();

		ImGui::Begin("Performance Data");
		static int f = 0;
		const int num = 50000;

		if (f >= num)
			f = 0;

		float averageFPS = ImGui::GetIO().Framerate;
		static float values[num];

		values[f] = averageFPS;

		char overlay[32];
		sprintf(overlay, "FPS: %f", averageFPS);
		ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), 0, overlay, -1.0f, 5000.0f, ImVec2(0, 80.0f));
		
		{
			ImGui::Text("Camera Transform");
			std::string p = "Position: ";
			Vector3 po = graphics->renderSystem->camera.position;
			p += std::to_string(po.x) + " " + std::to_string(po.y) + " " + std::to_string(po.z);
			ImGui::Text(p.c_str());
			
		}
		//ImGui::tex

		/*MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;

		DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;*/

		/*PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

		SIZE_T physMemUsedByMe = pmc.WorkingSetSize;


		std::cout << physMemUsedByMe << std::endl;*/

		ImGui::End();

		lastSelectedEntityStr = selectedEntityStr;
		f++;
	}
};