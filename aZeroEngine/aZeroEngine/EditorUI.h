#pragma once
#include "UserInterface.h"

#include "Graphics.h"
#include "AppWindow.h"
#include "Scene.h"
#include <filesystem>

class EditorUI : public UserInterface
{
private:
	void ShowSettings();
	void ShowMaterialEditor();
	void ShowEntityEditor();
	void ShowSceneManager();
	void ShowLoadedVertexBuffers();
	void ShowLoadedTextures();
	void ShowGlobalLightingManager();
	void ShowDebugTextures();

	void PBRMatEditing();
	void PhongMatEditing();

	template<typename T>
	void HandleListSelection(const std::vector<T>& _vec, const std::string& _listText, std::string& _selectedStr, int& _selectedID, int _textImGuiID)
	{
		if (ImGui::BeginListBox(_listText.c_str()))
		{
			for (const auto& mat : _vec)
			{
				bool selected = false;
				if (mat.GetName() == _selectedStr)
					selected = true;

				if (ImGui::Selectable(mat.GetName().c_str(), selected))
				{
					_selectedStr = mat.GetName();
					_selectedID = graphics.materialManager.GetReferenceID<T>(mat.GetName());
					break;
				}
			}
			ImGui::EndListBox();
		}

		std::string selMat = "Selected Material: ";
		if (_selectedID != -1)
		{
			selMat += graphics.materialManager.GetMaterial<T>(_selectedID)->GetName();
		}
		else
		{
			selMat += "NULL";
		}
		ImGui::Text(selMat.c_str());
	};

	template<typename T>
	void CreateMaterial(const std::string& _btnText)
	{
		static char matNameBuffer[64] = "";
		ImGui::InputText("Material Name (MAX 64 CHARACTERS)", matNameBuffer, 64);
		if (ImGui::Button(_btnText.c_str()) && matNameBuffer[0] != '\0')
		{
			const std::string newMatTempName(matNameBuffer);
			if (!graphics.materialManager.Exists<T>(newMatTempName))
			{
				graphics.materialManager.CreateMaterial<T>(newMatTempName);
				T* tempNew = graphics.materialManager.GetMaterial<T>(newMatTempName);
			}
			ZeroMemory(matNameBuffer, ARRAYSIZE(matNameBuffer));
		}
	}

	template<typename T>
	void SaveMaterial(const std::string& _btnText, int _selectedMaterialID, const std::string& _defaultMat)
	{
		if (_selectedMaterialID != -1)
		{
			T* selectedMaterial = graphics.materialManager.GetMaterial<T>(_selectedMaterialID);
			if (selectedMaterial->GetName() == _defaultMat)
				return;

			if (ImGui::Button(_btnText.c_str()))
			{
				selectedMaterial->Save("../materials/", graphics.textureCache);
			}
		}
	}

	template<typename T>
	void LoadMaterial(const std::string& _btnText, std::string& _selectedMaterialStr, int& _selectedMaterialID, const std::string& _ext)
	{
		if (ImGui::Button(_btnText.c_str()))
		{
			std::string matNameWithExt = "";
			if (Helper::OpenFileDialogForExtension(_ext, matNameWithExt))
			{
				std::string::size_type const p(matNameWithExt.find_last_of('.'));
				std::string matNameWithoutExt = matNameWithExt.substr(0, p);

				graphics.materialManager.LoadMaterial<T>(matNameWithoutExt);
				T* mat = graphics.materialManager.GetMaterial<T>(matNameWithoutExt);
				_selectedMaterialID = graphics.materialManager.GetReferenceID<T>(matNameWithoutExt);
				_selectedMaterialStr = mat->GetName();
			}
			//ZeroMemory(matNameBuffer, ARRAYSIZE(matNameBuffer));
		}
	}

	template<typename T>
	void DeleteMaterial(const std::string& _btnText, std::string& _selectedMaterialStr, int& _selectedMaterialID, const std::string& _defaultMaterial)
	{
		if (_selectedMaterialID > 0)
		{
			if (ImGui::Button(_btnText.c_str()))
			{
				if (graphics.scene != nullptr)
				{/*
					for (auto& ent : graphics.scene->entities.GetObjects())
					{
						MaterialComponent* matComp = graphics.ecs.GetComponentManager().GetComponent<MaterialComponent>(ent);
						if (matComp != nullptr)
						{
							if (matComp->type == MATERIALTYPE::PBR)
							{
								if (matComp->materialID == _selectedMaterialID)
									matComp->materialID = graphics.materialManager.GetReferenceID<T>(_defaultMaterial);
							}
						}
					}*/
				}
				graphics.materialManager.RemoveMaterial<T>(_selectedMaterialID);
				_selectedMaterialID = -1;
				_selectedMaterialStr = "";
			}
		}
	}
protected:
	Graphics& graphics;
	AppWindow& window;

public:
	void DrawMaterialPreview();
	void ShowPerformanceData();

	void ShowMaterialPreview()
	{
		if (selectedPhongMaterialID != -1)
		{
			ImGui::Begin("Material Preview");
			ImGui::Image((ImTextureID)matPrevRTV.GetSrvHandle().GetGPUHandle().ptr, ImVec2(600, 600));
			ImGui::End();

			/*D3D12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(matPrevRTV.GetMainResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			graphics.resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &x);*/
		}
	}

	int selectedEntityID = -1;

	std::string selectedPhongMaterialStr = "";
	int selectedPhongMaterialID = -1;

	std::string selectedPBRMaterialStr = "";
	int selectedPBRMaterialID = -1;

	std::string selectedMeshStr = "";
	int selectedMeshID = -1;
	std::string selectedTextureStr = "";
	int selectedTextureID = -1;

	bool editingPLight = false;
	bool editingDLight = false;
	ConstantBuffer cbMaterialPrev;
	bool showPreview = false;

	struct temp
	{
		Matrix world;
		Matrix view;
		Matrix proj;
	};
	temp tempMatPrev;

	RenderTarget matPrevRTV;
	DepthStencil matPrevDSV;
	PipelineState psoMatPrev;
	RootSignature rootMatPrev;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};

	InputLayout layout;
	RasterState solidRaster;

	EditorUI(const std::string& _name, Graphics& _graphics, AppWindow& _window)
		:UserInterface(_name/*, _graphics.device, _graphics.descriptorManager.GetResourceHeap(), _graphics.descriptorManager.GetResourceDescriptor(), _window.GetHandle()*/), graphics(_graphics), window(_window)
	{
		Matrix worldMat = Matrix::CreateTranslation(0, 0, 2);
		Matrix view = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
		Matrix proj = Matrix::CreatePerspectiveFieldOfView(3.14f * 0.2f, (float)600 / (float)600, 0.1f, 100.f);

		tempMatPrev.world = worldMat;
		tempMatPrev.view = view;
		tempMatPrev.proj = proj;

		_graphics.resourceEngine.CreateResource(cbMaterialPrev, (void*)&tempMatPrev, sizeof(temp), true, true);

		/*_graphics.resourceEngine.CreateResource(matPrevRTV, graphics.descriptorManager.GetRTVDescriptor(),
			graphics.descriptorManager.GetResourceDescriptor(), 600, 600, 4, DXGI_FORMAT_B8G8R8A8_UNORM, false, Vector4(0.5f, 0.5f, 0.5f, 1.f));

		_graphics.resourceEngine.CreateResource(matPrevDSV, graphics.descriptorManager.GetDSVDescriptor(), 600, 600);*/

		RootParameters params;
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // mesh transform
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL); // Material 1

		rootMatPrev.Init(_graphics.device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT 
			| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

		solidRaster = RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);
		psoMatPrev.Init(_graphics.device, &rootMatPrev, layout, solidRaster, 3, DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_D24_UNORM_S8_UINT, L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_MaterialPrev.cso",
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_MaterialPrev.cso", L"", L"", L"");

#ifdef _DEBUG
		psoMatPrev.GetPipelineState()->SetName(L"Material Preview PSO");
		rootMatPrev.GetSignature()->SetName(L"Material Preview Root Signature");
		matPrevRTV.GetGPUOnlyResource()->SetName(L"RTV Material Preview");
		matPrevDSV.GetGPUOnlyResource()->SetName(L"DSV Material Preview");
#endif // DEBUG

		// Viewport
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (FLOAT)600;
		viewport.Height = (FLOAT)600;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		// Scissor Rect
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = 600;
		scissorRect.bottom = 600;

		//ImGui::StyleColorsClassic();

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.22f, 0.36f, 0.51f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.47f, 0.79f, 1.00f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.29f, 0.67f, 1.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.67f, 1.00f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.29f, 0.67f, 1.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.29f, 0.67f, 1.00f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.29f, 0.67f, 1.00f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.60f, 0.96f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
		colors[ImGuiCol_Tab] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.56f, 0.80f, 1.00f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.40f, 1.00f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.07f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 7;
		style.FrameRounding = 4;
		style.TabRounding = 4;
	}

	virtual ~EditorUI() { printf("Editor Destroyed\n"); }

	void BeginFrame()
	{
		UserInterface::BeginFrame();
	}

	virtual void Update() override;


};