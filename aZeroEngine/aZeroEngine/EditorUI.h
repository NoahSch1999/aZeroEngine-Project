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
				graphics.materialManager.CreateMaterial<T>(graphics.device, newMatTempName);
				T* tempNew = graphics.materialManager.GetMaterial<T>(newMatTempName);

				//Texture2D* t = graphics.textureCache.GetResource("defaultDiffuse.png");
				//if (t)
				//	tempNew->GetInfoPtr().albedoMapIndex = t->GetHandle().GetHeapIndex();

				//tempNew->Update(graphics.resourceEngine, graphics.frameIndex);
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

				graphics.materialManager.LoadMaterial<T>(graphics.device, matNameWithoutExt);
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
				{
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
					}
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

	std::string selectedEntityStr = "";
	std::string lastSelectedEntityStr = "";
	int selectedEntityID = -1;
	int lastSelectedEntityID = -1;

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

	bool newScene = false;
	std::string sceneName = "";

	bool closeScene = false;

	EditorUI(const std::string& _name, Graphics& _graphics, AppWindow& _window)
		:UserInterface(_name, _graphics.device, _graphics.descriptorManager.GetResourceHeap(), _graphics.descriptorManager.GetPassDescriptor(), _window.GetHandle()), graphics(_graphics), window(_window)
	{
		
		Matrix worldMat = Matrix::CreateTranslation(0, 0, 2);
		Matrix view = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
		Matrix proj = Matrix::CreatePerspectiveFieldOfView(3.14f * 0.2f, (float)600 / (float)600, 0.1f, 100.f);

		tempMatPrev.world = worldMat;
		tempMatPrev.view = view;
		tempMatPrev.proj = proj;

		_graphics.resourceEngine.CreateResource(_graphics.device, cbMaterialPrev, (void*)&tempMatPrev, sizeof(temp), true, true);

		_graphics.resourceEngine.CreateResource(_graphics.device, matPrevRTV, graphics.descriptorManager.GetRTVDescriptor(),
			graphics.descriptorManager.GetPassDescriptor(), 600, 600, DXGI_FORMAT_B8G8R8A8_UNORM, Vector4(0.5f, 0.5f, 0.5f, 1.f));

		_graphics.resourceEngine.CreateResource(_graphics.device, matPrevDSV, graphics.descriptorManager.GetDSVDescriptor(), 600, 600);

		matPrevRTV.GetMainResource()->SetName(L"RTV Material Preview");
		matPrevDSV.GetMainResource()->SetName(L"DSV Material Preview");

		RootParameters params;
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // 0
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 100, D3D12_SHADER_VISIBILITY_PIXEL, 0, 1); // Textures 1
		params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL); // Material 2
		params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL); // Sampler 3

		rootMatPrev.Init(_graphics.device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

		psoMatPrev.Init(_graphics.device, &rootMatPrev, _graphics.renderSystem->layout, _graphics.renderSystem->solidRaster, 3, DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_D24_UNORM_S8_UINT, L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_MaterialPrev.cso",
			L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_MaterialPrev.cso", L"", L"", L"");

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
	}

	virtual ~EditorUI() { printf("Editor Destroyed\n"); }

	void BeginFrame()
	{
		UserInterface::BeginFrame();
		
	}

	virtual void Update() override;


};