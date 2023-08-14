#include "LevelEditorUI.h"
#include <filesystem>
#include "aZeroModelParsing/ModelParser.h"

void LevelEditorUI::ShowPerformanceData()
{
	ImGui::Begin("Performance Data");
	static int currentFramesPerfFPS = 0;
	const int targetFramesPerfFPS = 50000;

	if (currentFramesPerfFPS >= targetFramesPerfFPS)
		currentFramesPerfFPS = 0;

	float averageFPS = ImGui::GetIO().Framerate;
	static float values[targetFramesPerfFPS];

	values[currentFramesPerfFPS] = averageFPS;
	currentFramesPerfFPS++;

	char overlay[32];
	sprintf(overlay, "FPS: %f", averageFPS);
	ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), 0, overlay, -1.0f, 5000.0f, ImVec2(0, 80.0f));

	ImGui::Text("Average MS and FPS: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	if (ImGui::Button("TESTSTTS"))
	{
		
		printf("h");
	}

	ImGui::End();
}

void LevelEditorUI::showApplicationInfo()
{
	ImGui::Begin("Application Info");

	if (currentScene)
	{
		const std::string numEntities("Entity Count: " + std::to_string(currentScene->GetEntityVector().size()));
		ImGui::Text(numEntities.c_str());
	}

	const std::string numTransformComponents("Transform Components: " + std::to_string(engine->GetComponentManager().GetComponentArray<Transform>().numComponents()));
	ImGui::Text(numTransformComponents.c_str());

	const std::string numMeshComponents("Mesh Components: " + std::to_string(engine->GetComponentManager().GetComponentArray<Mesh>().numComponents()));
	ImGui::Text(numMeshComponents.c_str());

	const std::string numMaterialComponents("Material Components: " + std::to_string(engine->GetComponentManager().GetComponentArray<MaterialComponent>().numComponents()));
	ImGui::Text(numMaterialComponents.c_str());

	std::shared_ptr<PhysicSystem> pSystem = engine->GetPhysicSystem().lock();
	const std::string numRigidBodies("RigidBody Components: " + std::to_string(pSystem->numEntitiesBound()));
	ImGui::Text(numRigidBodies.c_str());

	const std::string numPointLightComponents("Point Light Components: " + std::to_string(engine->GetComponentManager().GetComponentArray<PointLightComponent>().numComponents()));
	ImGui::Text(numPointLightComponents.c_str());
	
	const std::string numBoundToLight("Light System Entity Count: " + std::to_string(engine->GetLightSystem().lock()->numEntitiesBound()));
	ImGui::Text(numBoundToLight.c_str());

	std::shared_ptr<LightManager> lManager = engine->GetLightSystem().lock()->GetLightManager();
	const std::string numInLightManager("Light Manager Num Lights: " + std::to_string(lManager->numLightsData.numPointLights) + " / " + std::to_string(lManager->pLightList.maxElements));
	ImGui::Text(numInLightManager.c_str());

	const std::string numTextures("Texture Count: " + std::to_string(engine->GetTexture2DCache().GetAllResources().size()));
	ImGui::Text(numTextures.c_str());

	const std::string numModels("Model Count: " + std::to_string(engine->GetModelCache().GetAllResources().size()));
	ImGui::Text(numModels.c_str());

	DescriptorManager& dManager = engine->GetDescriptorManager();
	const std::string numResourceDescriptors("Resource Descriptors Used: " + std::to_string(dManager.getResourceShaderDescriptorHeap().getNumberOfUsedDescriptors()) + " / " + std::to_string(dManager.getResourceShaderDescriptorHeap().getMaxDescriptors()));
	ImGui::Text(numResourceDescriptors.c_str());

	const std::string numSamplerDescriptors("Sampler Descriptors Used: " + std::to_string(dManager.getSamplerShaderDescriptorHeap().getNumberOfUsedDescriptors()) + " / " + std::to_string(dManager.getSamplerShaderDescriptorHeap().getMaxDescriptors()));
	ImGui::Text(numSamplerDescriptors.c_str());

	const std::string numRTVDescriptors("RTV Descriptors Used: " + std::to_string(dManager.getRTVDescriptorHeap().getNumberOfUsedDescriptors()) + " / " + std::to_string(dManager.getRTVDescriptorHeap().getMaxDescriptors()));
	ImGui::Text(numRTVDescriptors.c_str());

	const std::string numDSVDescriptors("DSV Descriptors Used: " + std::to_string(dManager.getDSVDescriptorHeap().getNumberOfUsedDescriptors()) + " / " + std::to_string(dManager.getDSVDescriptorHeap().getMaxDescriptors()));
	ImGui::Text(numDSVDescriptors.c_str());

	MaterialManager& mManager = engine->GetMaterialManager();
	const std::string numMaterials("Material Count: " + std::to_string(mManager.GetPBRMaterials().size()));
	ImGui::Text(numMaterials.c_str());

	ImGui::End();
}

void LevelEditorUI::showApplicationSettings()
{
	// Add to setting window
	if(ImGui::Begin("Settings"))
	{
		static bool lastPlaystop = false;
		ImGui::Checkbox("Play", &m_currentlyPlaying);

		if (m_currentlyPlaying != lastPlaystop)
		{
			this->play(m_currentlyPlaying);
		}

		lastPlaystop = m_currentlyPlaying;

		{
			std::shared_ptr<PhysicSystem> pSystem = engine->GetPhysicSystem().lock();
			float currentGravity = pSystem->getGravity();
			ImGui::InputFloat("Gravity", &currentGravity);
			pSystem->setGravity(currentGravity);
		}

		{
			std::shared_ptr<RendererSystem> rSystem = engine->GetRenderSystem().lock();

			ImGui::Checkbox("Enable Glow", &rSystem->m_renderSettings.m_enableGlow);

			ImGui::SliderInt("Glow Radius", &rSystem->m_renderSettings.m_glowRadius, 0, 25);

			std::shared_ptr<Camera> cam = camera.lock();

			ImGui::SliderFloat("Far Plane", &cam->farPlane, cam->nearPlane + 0.2f, 1000.f, "%.3f", 32);
			ImGui::SliderAngle("Fov", &cam->fov, 10.f, 180.f);

			ImGui::InputFloat("Gamma", &rSystem->m_renderSettings.m_gamma, 0.1f);
			ImGui::SliderFloat("HDR Exposure", &rSystem->m_renderSettings.m_hdrExposure, 0.001f, 10.f);

			ImGui::Checkbox("Enable Selection Outlines", &rSystem->m_renderSettings.m_drawSelectionOutlines);

			if (rSystem->m_renderSettings.m_drawSelectionOutlines)
			{
				ImGui::SliderInt("Outline Thickness", &rSystem->m_renderSettings.m_outlineThickness, 1, 20);
				ImGui::ColorEdit3("Main Selection Color", (float*)&rSystem->m_renderSettings.m_mainOutlineColor);
				ImGui::ColorEdit3("Secondary Selection Color", (float*)&rSystem->m_renderSettings.m_secondaryOutlineColor);
			}

			{
				ImGui::Text("MSAA Sample Count");
				int currentMSAACount = rSystem->m_renderSettings.m_msaaCount;
				ImGui::RadioButton("1##msaa", (int*)&rSystem->m_renderSettings.m_msaaCount, 1); ImGui::SameLine();
				ImGui::RadioButton("2##msaa", (int*)&rSystem->m_renderSettings.m_msaaCount, 2); ImGui::SameLine();
				ImGui::RadioButton("4##msaa", (int*)&rSystem->m_renderSettings.m_msaaCount, 4); ImGui::SameLine();
				ImGui::RadioButton("8##msaa", (int*)&rSystem->m_renderSettings.m_msaaCount, 8);

				if (currentMSAACount != rSystem->m_renderSettings.m_msaaCount)
					rSystem->notifyRenderSettingsUpdate();

				const char* resolutions[] = { "1920x1080", "1600x900", "1280x1024"};
				static int currentResolution = 0; 
				const char* comboValue = resolutions[currentResolution];  
				if (ImGui::BeginCombo("Resolution", comboValue))
				{
					for (int n = 0; n < IM_ARRAYSIZE(resolutions); n++)
					{
						const bool is_selected = (currentResolution == n);
						if (ImGui::Selectable(resolutions[n], is_selected))
						{
							currentResolution = n;
							switch (currentResolution)
							{
								case 0:
								{
									engine->resizeWindow(1920, 1080);
									break;
								}
								case 1:
								{
									engine->resizeWindow(1600, 900);
									break;
								}
								case 2:
								{
									engine->resizeWindow(1280, 1024);
									break;
								}
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				const char* textureFilterings[] = { "POINT", "LINEAR", "ANISOTROPIC_X8", "ANISOTROPIC_X16" };
				static int currentFiltering = rSystem->m_renderSettings.m_currentSampler; 
				const char* currentFilteringValue = textureFilterings[currentFiltering];  
				if (ImGui::BeginCombo("Texture Filtering", currentFilteringValue))
				{
					for (int n = 0; n < IM_ARRAYSIZE(textureFilterings); n++)
					{
						const bool is_selected = (currentFiltering == n);
						if (ImGui::Selectable(textureFilterings[n], is_selected))
						{
							currentFiltering = n;
							switch (currentFiltering)
							{
							case 0:
							{
								rSystem->m_renderSettings.m_currentSampler = RendererSystem::RenderSettings::SAMPLERTYPE::POINT;
								break;
							}
							case 1:
							{
								rSystem->m_renderSettings.m_currentSampler = RendererSystem::RenderSettings::SAMPLERTYPE::LINEAR;
								break;
							}
							case 2:
							{
								rSystem->m_renderSettings.m_currentSampler = RendererSystem::RenderSettings::SAMPLERTYPE::ANISOTROPIC_X8;
								break;
							}
							case 3:
							{
								rSystem->m_renderSettings.m_currentSampler = RendererSystem::RenderSettings::SAMPLERTYPE::ANISOTROPIC_X16;
								break;
							}
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
		}

		ImGui::End();
	}
}

void LevelEditorUI::SetupVisuals()
{
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
	//colors[ImGuiCol_DockingPreview] = ImVec4(0.09f, 0.57f, 0.98f, 1.00f);
	//colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
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

int LevelEditorUI::applyTexturePopup(int ID)
{
	const std::string btnName = "Apply Texture##" + std::to_string(ID);
	const std::string popupName = "ApplyTexturePopup##" + std::to_string(ID);
	const std::string listBoxName = "##" + std::to_string(ID);

	if (ImGui::Button(btnName.c_str()))
	{
		ImGui::OpenPopup(popupName.c_str());
	}

	int retValue = -1;
	if (ImGui::BeginPopup(popupName.c_str()))
	{
		if (ImGui::BeginListBox(listBoxName.c_str()))
		{
			for (Texture& texture : engine->GetTexture2DCache().GetAllResources())
			{
				const std::string name = engine->GetTexture2DCache().GetFileNameByHeapIndex(texture.getSRVHandle().getHeapIndex());
				if (ImGui::Selectable(name.c_str()))
				{
					retValue = texture.getSRVHandle().getHeapIndex();
					break;
				}
			}
			ImGui::EndListBox();
		}
		ImGui::EndPopup();
	}

	return retValue;
}

std::optional<std::string> LevelEditorUI::LoadModelDataFromDirectory()
{
	std::string nameWithExt = "";
	

	if (Helper::OpenFileDialogForExtension({".fbx", ".azModel"}, nameWithExt))
	{
		const std::string::size_type const nameLengthNoExt(nameWithExt.find_last_of('.'));
		std::string nameWithoutExt = nameWithExt.substr(0, nameLengthNoExt);
		const std::string extension(nameWithExt.substr(nameLengthNoExt, nameWithExt.length() - nameLengthNoExt));

		GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();

		if (extension == ".fbx")
		{
			aZeroFiles::ModelGenerateSettings settings;
			settings.m_sourceDirectory = "../meshes/";
			settings.m_sourceFileName = nameWithoutExt;
			settings.m_targetDirectory = "../meshes/";
			settings.m_targetFileName = nameWithoutExt;
			settings.m_fileExtension = ".fbx";
			aZeroFiles::GenerateAZModel(settings);
			engine->GetModelCache().LoadAZModel(engine->GetDevice(), context, engine->GetFrameIndex(), nameWithoutExt, "../meshes/");
		}
		else if (extension == ".azModel")
		{
			engine->GetModelCache().LoadAZModel(engine->GetDevice(), context, engine->GetFrameIndex(), nameWithoutExt, "../meshes/");
		}

		engine->GetCommandManager().executeContext(context);
		return nameWithoutExt;
	}
	return {};
}

std::optional<std::string> LevelEditorUI::LoadPBRMaterialFromDirectory()
{
	std::string matNameWithExt = "";

	if (Helper::OpenFileDialogForExtension({ ".azmpbr" }, matNameWithExt))
	{
		std::string::size_type const p(matNameWithExt.find_last_of('.'));
		std::string fileNameWithoutExt = matNameWithExt.substr(0, p);

		GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();

		engine->GetMaterialManager().LoadMaterial<PBRMaterial>(engine->GetDevice(), context,
			engine->GetFrameIndex(), fileNameWithoutExt);

		engine->GetCommandManager().executeContext(context);

		return fileNameWithoutExt;
	}
	return {};
}

void LevelEditorUI::Update()
{
	if (editorMode)
	{
		ShowPerformanceData();
		showApplicationInfo();
		ImGui::ShowDemoWindow();
	
		ShowSceneWindow();

		showApplicationSettings();

		showMeshes();
		showFileTextures();
		ShowMaterialWindow();

		if (currentScene)
			ShowEntityWindow();

		if (!selectionList.Empty())
		{
			RigidBody* rbComp = currentScene->GetComponentForEntity<RigidBody>(currentScene->GetEntity(*selectionList.Begin()));
			if (rbComp)
			{
				if (ImGui::Begin("DEBUG WINDOW"))
				{
					if (rbComp->m_colliders.count("Collider") > 0)
					{
						reactphysics3d::Transform tf = rbComp->m_colliders.at("Collider")->getLocalToBodyTransform();
						float localToBody[3]{ tf.getPosition().x, tf.getPosition().y, tf.getPosition().z };
						ImGui::InputFloat3("Local To Body", localToBody);

						tf = rbComp->m_colliders.at("Collider")->getLocalToWorldTransform();
						float localToWorld[3]{ tf.getPosition().x, tf.getPosition().y, tf.getPosition().z };
						ImGui::InputFloat3("Local To World", localToWorld);
					}
					ImGui::End();
				}
			}
		}
	}
}

void LevelEditorUI::DrawEntityComponents(aZeroECS::Entity& _entity)
{
	std::shared_ptr<Camera> cam = camera.lock();
	Transform* tf = currentScene->GetComponentForEntity<Transform>(_entity);

	aZeroECS::Entity& rootEntity = currentScene->GetEntity(selectionList.GetRoot());
	if (!editingPLight)
	{
		if (tf)
		{
			static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
			static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

			if (InputManager::KeyDown('W'))
				mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			if (InputManager::KeyDown('R'))
				mCurrentGizmoOperation = ImGuizmo::ROTATE;
			if (InputManager::KeyDown('E'))
				mCurrentGizmoOperation = ImGuizmo::SCALE;

			ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
			DXM::Matrix matrix = tf->GetWorldMatrix();
			
			if (ImGuizmo::Manipulate(&cam->GetView()._11, &cam->GetProj()._11, mCurrentGizmoOperation, mCurrentGizmoMode, &matrix._11, NULL, 0))
			{
				int parentID = engine->GetParentSystem().lock()->GetParentEntityID(rootEntity);

				if (parentID != -1)
				{
					aZeroECS::Entity& parentEnt = currentScene->GetEntity(parentID);

					Transform* parentTF = currentScene->GetComponentForEntity<Transform>(parentEnt);
					if (parentTF)
					{
						DXM::Matrix wParent = parentTF->GetWorldMatrix();
						DXM::Matrix local = matrix * wParent.Invert();
						DXM::Quaternion rotationDegQuat;
						local.Decompose(tf->GetScale(), rotationDegQuat, tf->GetTranslation());
						tf->GetRotation() = rotationDegQuat.ToEuler();
					}
				}
				else
				{
					DXM::Quaternion rotationDegQuat;
					matrix.Decompose(tf->GetScale(), rotationDegQuat, tf->GetTranslation());
					tf->GetRotation() = rotationDegQuat.ToEuler();
				}
			}
		}
	}


	static int selComponentPopup = -1;
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("CompPopup");

	aZeroECS::ComponentManager& cManager = engine->GetComponentManager();
	if (ImGui::BeginPopup("CompPopup"))
	{
		for (auto [first, second] : cManager.GetBitFlagMap())
		{
			if (!_entity.m_componentMask[second])
			{
				const std::string componentNameType(first.name());
				size_t lastSpaceIndex = componentNameType.find_last_of(" ");
				const std::string componentName(componentNameType.substr(lastSpaceIndex + 1, componentNameType.length() - lastSpaceIndex));

				if (ImGui::Selectable(componentName.c_str()))
				{
					switch (second)
					{
					case 0:
					{
						Transform comp;
						currentScene->AddComponentToEntity<Transform>(_entity, std::move(comp));
						break;
					}
					case 1:
					{
						Mesh comp;
						comp.SetID(engine->GetModelCache().GetID("defaultCube"));
						currentScene->AddComponentToEntity<Mesh>(_entity, std::move(comp));

						aZeroECS::ComponentArray<MaterialComponent>& arr = engine->GetComponentManager().GetComponentArray<MaterialComponent>();

						if (!engine->GetComponentManager().HasComponent<MaterialComponent>(_entity))
						{
							MaterialComponent compMat;
							compMat.materialID = engine->GetMaterialManager().GetReferenceID<PBRMaterial>("DefaultPBRMaterial");
							compMat.type = MATERIALTYPE::PBR;
							currentScene->AddComponentToEntity<MaterialComponent>(_entity, std::move(compMat));
						}
						
						break;
					}
					case 2:
					{
						MaterialComponent comp;
						comp.materialID = engine->GetMaterialManager().GetReferenceID<PBRMaterial>("DefaultPBRMaterial");
						comp.type = MATERIALTYPE::PBR;
						currentScene->AddComponentToEntity<MaterialComponent>(_entity, std::move(comp));
						break;
					}
					case 3:
					{
						PointLightComponent comp;
						currentScene->AddComponentToEntity<PointLightComponent>(_entity, std::move(comp));
						break;
					}
					case 5:
					{
						currentScene->AddComponentToEntity<RigidBody>(_entity);
						break;
					}
					}
				}
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::TreeNode("Components"))
	{
		for (auto [first, second] : cManager.GetBitFlagMap())
		{
			if (_entity.m_componentMask[second])
			{
				const std::string componentNameType(first.name());
				size_t lastSpaceIndex = componentNameType.find_last_of(" ");
				const std::string componentName(componentNameType.substr(lastSpaceIndex + 1, componentNameType.length() - lastSpaceIndex));

				if (ImGui::TreeNode((void*)(intptr_t)second, componentName.c_str()))
				{
					switch (second)
					{
					case 0:
					{
						if (tf)
						{
							ImGui::InputFloat3("Translation", &tf->GetTranslation().x);
							DXM::Quaternion qRotInRadians(tf->GetRotation());
							DXM::Vector3 vRotInRadians = tf->GetRotation();
							DXM::Vector3 vRotInDegrees;
							bool inputAngleChanged = false;
							vRotInDegrees.x = vRotInRadians.x * (180 / 3.14f);
							vRotInDegrees.y = vRotInRadians.y * (180 / 3.14f);
							vRotInDegrees.z = vRotInRadians.z * (180 / 3.14f);

							if (ImGui::SliderAngle("##angleX", (float*)&vRotInRadians.x))
							{
								tf->GetRotation() = vRotInRadians;
							}
							ImGui::SameLine();
							if (ImGui::InputFloat("X-Axis##rot", (float*)&vRotInDegrees.x))
								inputAngleChanged = true;


							if (ImGui::SliderAngle("##angleY", (float*)&vRotInRadians.y))
							{
								tf->GetRotation() = vRotInRadians;
							}
							ImGui::SameLine();
							if (ImGui::InputFloat("Y-Axis##rot", (float*)&vRotInDegrees.y))
								inputAngleChanged = true;

							if (ImGui::SliderAngle("##angleZ", (float*)&vRotInRadians.z))
							{
								tf->GetRotation() = vRotInRadians;
							}
							ImGui::SameLine();
							if (ImGui::InputFloat("Z-Axis##rot", (float*)&vRotInDegrees.z))
								inputAngleChanged = true;

							if (inputAngleChanged)
							{
								DXM::Vector3 rotInRad = DXM::Vector3(vRotInDegrees.x / (180 / 3.14f), vRotInDegrees.y / (180 / 3.14f), vRotInDegrees.z / (180 / 3.14f));
								tf->GetRotation() = rotInRad;
							}

							ImGui::InputFloat3("Scale", &tf->GetScale().x);

							if (ImGui::Button("Reset##1"))
							{
								tf->GetTranslation() = { 0,0,0 };
								tf->GetRotation() = { 0,0,0 };
								tf->GetScale() = { 1,1,1 };
							}

						}
						break;
					}
					case 1:
					{
						Mesh* meshComp = currentScene->GetComponentForEntity<Mesh>(rootEntity);
						if (meshComp)
						{
							auto& modelCache = engine->GetModelCache();
							ImGui::Text("Current: ");
							ImGui::SameLine();
							ImGui::TextWrapped(modelCache.GetFileName(meshComp->GetID()).c_str());

							if (ImGui::BeginDragDropTarget())
							{
								if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshToComponentDragDrop"))
								{
									int payloadData = *(const int*)payload->Data;
									meshComp->SetID(payloadData);
								}
								ImGui::EndDragDropTarget();
							}

							// Should be scrollable ----
							if (ImGui::Button("Change Mesh"))
								ImGui::OpenPopup("MeshPopup");

							if (ImGui::BeginPopup("MeshPopup"))
							{
								std::vector<ModelAsset>& vbVec = modelCache.GetAllResources();

								if (ImGui::Button("Load Mesh"))
								{
									// Potential problem if a mesh with the same name as an already loaded one is trying to be loaded
									// This will simply cause the selected mesh to not be loaded and the current edited mesh component will
									// be set to the already loaded mesh.
									std::optional<std::string> loadedName = this->LoadModelDataFromDirectory();
									if (loadedName.has_value())
										meshComp->SetID(modelCache.GetID(loadedName.value()));
								}
								else
								{
									for (ModelAsset& vb : vbVec)
									{
										if (ImGui::Selectable(vb.getMeshName().c_str()))
										{
											meshComp->SetID(modelCache.GetID(vb.getMeshName()));
											break;
										}
									}
								}

								ImGui::EndPopup();
							}
							// ----

							if (ImGui::Checkbox("Cast Shadows", &meshComp->castShadows));

							bool recShadows = false;
							if (meshComp->receiveShadows == 1.f)
								recShadows = true;

							if (ImGui::Checkbox("Receive Shadows", &recShadows))
							{
								if (recShadows)
									meshComp->receiveShadows = 1.f;
								else
									meshComp->receiveShadows = 0.f;
							}

							if (ImGui::Button("Delete##1"))
							{
								currentScene->RemoveComponentFromEntity<Mesh>(_entity);
							}
						}

						break;
					}
					case 2:
					{
						MaterialComponent* matComp = currentScene->GetComponentForEntity<MaterialComponent>(_entity);
						if (matComp)
						{
							MaterialManager& mManager = engine->GetMaterialManager();

							ImGui::Text("Current: ");
							ImGui::SameLine();
							if (matComp->materialID != -1)
							{
								ImGui::Text(mManager.GetMaterial<PBRMaterial>(matComp->materialID)->GetName().c_str());
							}
							else
							{
								ImGui::Text("Default PBR");
							}

							// Should be scrollable ----
							if (ImGui::Button("Change Material"))
								ImGui::OpenPopup("MatPopup");

							if (ImGui::BeginPopup("MatPopup"))
							{
								std::vector<PBRMaterial>& pbrMaterials = mManager.GetPBRMaterials();

								if (ImGui::Button("Load Material"))
								{
									// Potential problem if a material with the same name as an already loaded one is trying to be loaded
									// This will simply cause the selected material to not be loaded and the current edited material component will
									// be set to the already loaded material.
									std::optional<std::string> loadedName = LoadPBRMaterialFromDirectory();
									if (loadedName.has_value())
										matComp->materialID = mManager.GetReferenceID<PBRMaterial>(loadedName.value());
								}
								else
								{
									if (ImGui::BeginListBox("##matselpopuplistbox"))
									{
										for (PBRMaterial& mat : pbrMaterials)
										{
											if (ImGui::Selectable(mat.GetName().c_str()))
											{
												matComp->materialID = mManager.GetReferenceID<PBRMaterial>(mat.GetName());
												break;
											}
										}
										ImGui::EndListBox();
									}
								}

								ImGui::EndPopup();
							}
							// ----

							if (ImGui::Button("Delete##2"))
							{
								currentScene->RemoveComponentFromEntity<MaterialComponent>(_entity);
							}

						}
						break;
					}
					case 3:
					{
						PointLightComponent* plComp = currentScene->GetComponentForEntity<PointLightComponent>(_entity);
						if (plComp)
						{
							std::shared_ptr<LightSystem> lightSystem = engine->GetLightSystem().lock();

							PointLight* pLight = lightSystem->GetLight(*plComp);
							if (pLight)
							{
								if (ImGui::Button("Edit Light Position"))
									editingPLight = !editingPLight;

								if (editingPLight)
								{
									static ImGuizmo::OPERATION imGuizmoOpPLight(ImGuizmo::TRANSLATE);
									static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

									ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

									DXM::Matrix lightMat = DXM::Matrix::CreateTranslation(pLight->position.x, pLight->position.y, pLight->position.z);

									if (ImGuizmo::Manipulate(&cam->GetView()._11, &cam->GetProj()._11,
										imGuizmoOpPLight, mCurrentGizmoMode, &lightMat._11, NULL, 0))
									{
										DXM::Quaternion emptyRot;
										DXM::Vector3 emptyScale;
										DXM::Vector3 tempTransLight;
										lightMat.Decompose(emptyScale, emptyRot, tempTransLight);
										pLight->position = DXM::Vector3(tempTransLight.x, tempTransLight.y, tempTransLight.z);

										lightSystem->UpdateLight(*plComp, *pLight);
									}
								}

								// Property Editing
								float pos[3]{ pLight->position.x, pLight->position.y, pLight->position.z };
								if (ImGui::InputFloat3("Position", pos))
								{
									pLight->position.x = pos[0];
									pLight->position.y = pos[1];
									pLight->position.z = pos[2];
									lightSystem->UpdateLight(*plComp, *pLight);
								}

								float col[3]{ pLight->color.x, pLight->color.y, pLight->color.z };
								if (ImGui::ColorEdit3("Color", col))
								{
									pLight->color.x = col[0];
									pLight->color.y = col[1];
									pLight->color.z = col[2];
									lightSystem->UpdateLight(*plComp, *pLight);
								}

								float intensity = pLight->intensity;
								if (ImGui::InputFloat("Intensity", &intensity, 0.f, FLT_MAX))
								{
									pLight->intensity = intensity;
									lightSystem->UpdateLight(*plComp, *pLight);
								}

								float range = pLight->range;
								if (ImGui::InputFloat("Range", &range, 0.f, FLT_MAX))
								{
									pLight->range = range;
									lightSystem->UpdateLight(*plComp, *pLight);
								}

								if (ImGui::Button("Delete Point Light"))
								{
									currentScene->RemoveComponentFromEntity<PointLightComponent>(rootEntity);
									editingPLight = false;
								}
							}
						}
						break;
					}
					case 5:
					{
						RigidBody* rigidBodyComp = engine->GetComponentManager().GetComponent<RigidBody>(rootEntity);
						if (rigidBodyComp)
						{
							static const char* physicsRigidbodyTypeStr = { "STATIC\0KINEMATIC\0DYNAMIC" };
							reactphysics3d::BodyType rbType = rigidBodyComp->m_body->getType();
							if (ImGui::Combo("Type", (int*)&rbType, physicsRigidbodyTypeStr))
							{
								rigidBodyComp->m_body->setType(rbType);
							}

							bool enableGravity = rigidBodyComp->m_body->isGravityEnabled();
							if (ImGui::Checkbox("Enable Gravity##1", &enableGravity))
							{
								rigidBodyComp->m_body->enableGravity(enableGravity);
							}

							float rbMass = rigidBodyComp->m_body->getMass();
							if (ImGui::InputFloat("Mass##1", &rbMass, 1.f))
							{
								rigidBodyComp->m_body->setMass(rbMass);
							}

							static std::string lastSelected = "";
							static int collCount = 0;
							std::shared_ptr<PhysicSystem> pSystem = engine->GetPhysicSystem().lock();
							if (ImGui::Button("Add Collider##Rb"))
							{
								const std::string collName = "BoxCollider" + std::to_string(collCount);
								pSystem->addBoxCollider(*rigidBodyComp, collName);
								collCount++;
							}

							if (ImGui::Button("Add Collider2##Rb"))
							{
								const std::string collName = "Sphere" + std::to_string(collCount);
								pSystem->addSphereCollider(*rigidBodyComp, collName);
								//pSystem->addConvexCollider(*rigidBodyComp, collName, "defaultSphere");
								collCount++;
							}

							if (ImGui::Button("Remove Collider"))
								ImGui::OpenPopup("RemoveColliderPopup");

							if (ImGui::BeginPopup("RemoveColliderPopup"))
							{
								if (ImGui::BeginListBox("##RemoveColliderListbox"))
								{
									for (auto& [name, collider] : rigidBodyComp->m_colliders)
									{
										if (ImGui::Selectable(name.c_str()))
										{
											if (lastSelected == name)
												lastSelected = "";
											pSystem->removeCollider(*rigidBodyComp, name);
											ImGui::CloseCurrentPopup();
											break;
										}
									}
									ImGui::EndListBox();
								}
								
								ImGui::EndPopup();
							}

							if (ImGui::BeginListBox("Colliders"))
							{
								for (auto& [name, collider] : rigidBodyComp->m_colliders)
								{
									bool test = name == lastSelected ? 1 : 0;
									if (ImGui::Selectable(name.c_str(), test))
									{
										lastSelected = name;
										break;
									}
								}
								ImGui::EndListBox();
							}

							if (lastSelected != "")
							{
								if (rigidBodyComp->m_colliders.count(lastSelected))
								{
									reactphysics3d::Collider* collider = rigidBodyComp->m_colliders.at(lastSelected);
									reactphysics3d::CollisionShape* shape = collider->getCollisionShape();

									reactphysics3d::CollisionShapeName name = shape->getName();

									if (name == reactphysics3d::CollisionShapeName::BOX)
									{
										reactphysics3d::BoxShape* box = dynamic_cast<reactphysics3d::BoxShape*>(shape);
										float tempBoxDims[3]{ box->getHalfExtents().x,box->getHalfExtents().y, box->getHalfExtents().z };
										ImGui::SliderFloat3("Dimensions", tempBoxDims, 0.f, 5.f);
										box->setHalfExtents({ tempBoxDims[0], tempBoxDims[1], tempBoxDims[2] });
									}
								}
							}

							if (ImGui::Button("Delete##1337"))
							{
								currentScene->RemoveComponentFromEntity<RigidBody>(rootEntity);
							}
						}
					}
					}
					ImGui::TreePop();
				}
			}
		}
		ImGui::TreePop();
	}
}

void LevelEditorUI::ShowSceneWindow()
{
	if (ImGui::Begin("Scene Manager"))
	{
		if (!currentScene)
		{
			const std::string sceneNameText = "Scene: NULL";
			ImGui::Text(sceneNameText.c_str());

			ImGuizmo::Enable(false);
			if (ImGui::Button("New Scene"))
				currentScene = engine->NewScene("Empty");
		}
		else
		{
			const std::string sceneNameText = "Scene: " + currentScene->GetName();
			ImGui::Text(sceneNameText.c_str());
		}

		if (!m_currentlyPlaying)
		{
			if (ImGui::Button("Open Scene"))
			{
				std::string sceneNameWithExt = "";
				if (Helper::OpenFileDialogForExtension({ ".azs" }, sceneNameWithExt))
				{
					std::string::size_type const p(sceneNameWithExt.find_last_of('.'));
					std::string sceneNameWithoutExt = sceneNameWithExt.substr(0, p);

					if (currentScene)
						currentScene.reset();

					GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
					currentScene = engine->LoadScene(context, "..\\scenes\\", sceneNameWithoutExt);
					engine->GetCommandManager().executeContext(context);

					editingPLight = false;
					selectionList.Clear();
				}
			}

			if (currentScene)
			{
				if (ImGui::Button("Close Scene"))
				{
					currentScene.reset();
					editingPLight = false;
					selectionList.Clear();
				}

				static char sceneNameBuffer[40] = "";
				ImGui::InputText("Scene Name", sceneNameBuffer, 40);
				static std::string name = "";
				static bool trySave = false;


				if (ImGui::Button("Save Scene"))
					trySave = true;

				if (trySave)
				{
					const std::string sceneName(sceneNameBuffer);
					ImGui::OpenPopup("##OverwriteScenePopup");

					if (sceneName.size() == 0)
					{
						const std::string fileFullPath = "..\\scenes\\" + currentScene->GetName() + ".azs";
						std::filesystem::path filePath(fileFullPath);

						if (std::filesystem::exists(filePath))
						{
							if (ImGui::BeginPopupModal("##OverwriteScenePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
							{
								const std::string msg = "File " + currentScene->GetName() + " already exists and will be overwritten!";
								ImGui::Text(msg.c_str());
								if (ImGui::Button("OK"))
								{
									if (currentScene->GetName().size() > 0)
										currentScene->Save("..\\scenes\\", currentScene->GetName().c_str(), &engine->GetTexture2DCache());

									ImGui::CloseCurrentPopup();
									trySave = false;
									ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
								}

								if (ImGui::Button("Cancel"))
								{
									ImGui::CloseCurrentPopup();
									trySave = false;
								}
								ImGui::EndPopup();
							}
						}
						else
						{
							if (currentScene->GetName().size() > 0)
							{
								currentScene->Save("..\\scenes\\", currentScene->GetName().c_str(), &engine->GetTexture2DCache());
								ImGui::CloseCurrentPopup();
								trySave = false;
								ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
							}
						}
					}
					else
					{
						const std::string fileFullPath = "..\\scenes\\" + sceneName + ".azs";
						std::filesystem::path filePath(fileFullPath);

						if (std::filesystem::exists(filePath))
						{
							if (ImGui::BeginPopupModal("OverwriteScenePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
							{
								const std::string msg = "File " + sceneName + " already exists and will be overwritten!";
								ImGui::Text(msg.c_str());

								if (ImGui::Button("OK"))
								{
									currentScene->Save("..\\scenes\\", sceneName, &engine->GetTexture2DCache());

									ImGui::CloseCurrentPopup();
									trySave = false;
									ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
								}

								if (ImGui::Button("Cancel"))
								{
									ImGui::CloseCurrentPopup();
									trySave = false;
								}

								ImGui::EndPopup();
							}
						}
						else
						{
							currentScene->Save("..\\scenes\\", sceneName, &engine->GetTexture2DCache());
							ImGui::CloseCurrentPopup();
							trySave = false;
							ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
						}
					}
				}
			}
		}
	}
	ImGui::End();
}

void LevelEditorUI::ShowEntityWindow()
{
	if (ImGui::Begin("Entity Editor"))
	{
		std::vector<aZeroECS::Entity>& entityVec = currentScene->GetEntityVector();

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("Scene Hierarchy##1"))
		{
			for (auto& ent : entityVec)
			{
				std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
				int id = pSys->GetParentEntityID(ent);
				if (id == -1)
				{
					ShowEntityHierarchy(ent);
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("Create Entity"))
		{
			aZeroECS::Entity& ent = currentScene->CreateEntity("Entity_");
		}

		int rootEntity = selectionList.GetRoot();
		if (rootEntity != -1)
		{
			aZeroECS::Entity& currentEnt = currentScene->GetEntity(rootEntity);

			if (ImGui::Button("Add Child"))
				ImGui::OpenPopup("AddChildPopup");

			if (ImGui::BeginPopup("AddChildPopup"))
			{
				std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
				std::vector<int> childrenIDs = pSys->GetChildrenEntityID(currentEnt);

				for (aZeroECS::Entity& ent : entityVec)
				{
					bool isChild = false;
					for (auto id : childrenIDs)
					{
						if (id == ent.m_id)
						{
							isChild = true;
							break;
						}
					}

					if (pSys->IsChildInBranch(ent, currentEnt.m_id))
						continue;

					if (!isChild)
					{
						if (ent.m_id != rootEntity)
						{
							int parId = pSys->GetParentEntityID(currentEnt);
							if (parId != ent.m_id)
							{
								std::optional<std::string> entIDStr = currentScene->GetEntityName(ent);
								if (entIDStr.has_value())
								{
									if (ImGui::Selectable(entIDStr.value().c_str()))
									{
										if (ent.m_id != rootEntity)
										{
											pSys->Parent(currentEnt, ent);
										}
									}
								}
							}
						}
					}
				}
				ImGui::EndPopup();
			}

			if (ImGui::Button("Remove As Child"))
			{
				std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
				aZeroECS::Entity& currentEnt = currentScene->GetEntity(rootEntity);
				int parentID = pSys->GetParentEntityID(currentEnt);
				if (parentID != -1)
				{
					aZeroECS::Entity tempParent(parentID);
					pSys->UnParent(tempParent, currentEnt);
				}
			}

			if (ImGui::Button("Delete Entity"))
			{
				currentScene->DeleteEntity(rootEntity);
				selectionList.Remove(rootEntity);
			}
			else
			{
				if (ImGui::Button("Rename Entity"))
					ImGui::OpenPopup("RenamePopup");

				if (ImGui::BeginPopup("RenamePopup"))
				{
					static char newName[40] = { 0 };
					if (ImGui::InputText("##1", newName, ARRAYSIZE(newName)))
					{
						const std::string newNameStr(newName);
						if (newNameStr != "")
						{
							currentScene->RenameEntity(currentScene->GetEntity(rootEntity), newName);
							ZeroMemory(newName, ARRAYSIZE(newName));
						}
					}
					ImGui::EndPopup();
				}

				DrawEntityComponents(currentScene->GetEntity(rootEntity));
			}
		}
	}
	ImGui::End();
}

void LevelEditorUI::ShowEntityHierarchy(const aZeroECS::Entity& _current)
{
	std::optional<std::string> name = currentScene->GetEntityName(_current);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if(selectionList.Selected(_current.m_id))
		flags |= ImGuiTreeNodeFlags_Selected;

	std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
	std::vector<int> childrenIDs = pSys->GetChildrenEntityID(_current);

	if (childrenIDs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx((void*)(intptr_t)_current.m_id, flags, name.value().c_str()))
	{
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			if (InputManager::KeyHeld(VK_SHIFT))
			{
				if (selectionList.Selected(_current.m_id))
					selectionList.Remove(_current.m_id);
				else
					selectionList.Add(_current.m_id);
			}
			else
			{
				selectionList.Clear();
				selectionList.Add(_current.m_id);
			}
		}

		for (int id : childrenIDs)
		{
			aZeroECS::Entity ent(id);
			ShowEntityHierarchy(ent);
		}

		ImGui::TreePop();
	}


}

void LevelEditorUI::ShowMaterialWindow()
{
	if (ImGui::Begin("Material Editor"))
	{
		MaterialManager& mManager = engine->GetMaterialManager();
		std::vector<PBRMaterial>& pbrMaterials = mManager.GetPBRMaterials();

		// Show Materials
		if (ImGui::BeginListBox("##0"))
		{
			for (const auto& mat : engine->GetMaterialManager().GetPBRMaterials())
			{
				const std::string matName = mat.GetName();
				bool selected = false;
				if (matName == selPBRMatName)
					selected = true;

				if (ImGui::Selectable(matName.c_str(), selected))
				{
					selPBRMatName = matName;
					selPBRMatID = mManager.GetReferenceID<PBRMaterial>(matName);
					break;
				}

				if (ImGui::BeginDragDropSource())
				{
					int matID = mManager.GetReferenceID<PBRMaterial>(matName);
					ImGui::SetDragDropPayload("MatDragDrop", (void*)&matID, sizeof(int));
					ImGui::Text(matName.c_str());
					ImGui::EndDragDropSource();
				}
			}
			ImGui::EndListBox();
		}

		if (selPBRMatID != -1)
		{
			const std::string selMat = "Selected Material: " + selPBRMatName;
			ImGui::Text(selMat.c_str());
		}
		// ----

		// Create Material
		static char matNameBuffer[64] = "";
		static bool alrExists = false;
		if (ImGui::InputText("Material Name (MAX 64 CHARACTERS)", matNameBuffer, 64))
			alrExists = false;

		if (ImGui::Button("New##0") && matNameBuffer[0] != '\0')
		{
			const std::string newMatTempName(matNameBuffer);
			if (!mManager.Exists<PBRMaterial>(newMatTempName))
			{
				GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
				mManager.CreateMaterial<PBRMaterial>(newMatTempName);
				engine->GetCommandManager().executeContext(context);
				ZeroMemory(matNameBuffer, ARRAYSIZE(matNameBuffer));
				alrExists = false;
			}
			else
			{
				alrExists = true;
				ImGui::SameLine();
				ImGui::Text("Material with the same name already exists...");
			}
		}
		// ----

		// Save Material
		if (selPBRMatID != -1)
		{
			PBRMaterial* selectedMaterial = mManager.GetMaterial<PBRMaterial>(selPBRMatID);
			if (selectedMaterial->GetName() != "DefaultPBRMaterial")
			{
				if (ImGui::Button("Save##0"))
				{
					selectedMaterial->Save("../materials/", engine->GetTexture2DCache());
				}
			}
		}
		// ----

		// Load Material
		if (ImGui::Button("Load##0"))
		{
			std::string matNameWithExt = "";
			if (Helper::OpenFileDialogForExtension({ ".azmpbr" }, matNameWithExt))
			{
				std::string::size_type const p(matNameWithExt.find_last_of('.'));
				std::string matNameWithoutExt = matNameWithExt.substr(0, p);

				GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
				mManager.LoadMaterial<PBRMaterial>(engine->GetDevice(), context,
					engine->GetFrameIndex(), matNameWithoutExt);
				engine->GetCommandManager().executeContext(context);

				PBRMaterial* mat = mManager.GetMaterial<PBRMaterial>(matNameWithoutExt);
				if (mat)
				{
					selPBRMatID = mManager.GetReferenceID<PBRMaterial>(matNameWithoutExt);
					selPBRMatName = mat->GetName();
				}
			}
		}
		// ----

		// Delete Material
		if (selPBRMatID != -1)
		{
			if (selPBRMatName != "DefaultPBRMaterial")
			{
				if (ImGui::Button("Delete##0"))
				{
					if (currentScene)
					{
						int defaultID = mManager.GetReferenceID<PBRMaterial>("DefaultPBRMaterial");
						for (auto& ent : currentScene->GetEntityVector())
						{
							MaterialComponent* matComp = currentScene->GetComponentForEntity<MaterialComponent>(ent);
							if (matComp != nullptr)
							{
								if (matComp->materialID == selPBRMatID)
									matComp->materialID = defaultID;
							}
						}
					}
					mManager.RemoveMaterial<PBRMaterial>(selPBRMatID);
					selPBRMatID = -1;
					selPBRMatName = "";
				}
			}
		}
		// ----

		PBRMaterial* pbrMat = engine->GetMaterialManager().GetMaterial<PBRMaterial>(selPBRMatID);
		if (pbrMat != nullptr)
		{
			if (pbrMat->GetName() != "DefaultPBRMaterial")
			{
				ImGui::Text("Material Information");
				PBRMaterialInfo& info = pbrMat->GetInfo();

				// Albedo
				ImGui::Text("Albedo Texture");
				Texture* t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.albedoMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.albedoMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(123456); id > -1)
				{
					info.albedoMapIndex = id;
				}

				// Roughness
				ImGui::Text("Roughness Texture");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.roughnessMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.roughnessMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(12345); id > -1)
				{
					info.roughnessMapIndex = id;
				}

				if (info.roughnessMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##0"))
						info.roughnessMapIndex = -1;
				}

				if (info.roughnessMapIndex == -1)
					ImGui::SliderFloat("Roughness Factor##0", (float*)&info.roughnessFactor, 0.f, 1.f);

				// Metallic
				ImGui::Text("Metallic Texture");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.metallicMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.metallicMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(1234567); id > -1)
				{
					info.metallicMapIndex = id;
				}

				if (info.metallicMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##00"))
						info.metallicMapIndex = -1;
				}

				if (info.metallicMapIndex == -1)
					ImGui::SliderFloat("Metallic Factor##0", (float*)&info.metallicFactor, 0.f, 1.f);

				// Normal Map
				ImGui::Text("Normal Map");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.normalMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.normalMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(12345678); id > -1)
				{
					info.normalMapIndex = id;
				}

				if (info.normalMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##000"))
						info.normalMapIndex = -1;
				}

				// Transparency
				ImGui::Checkbox("Use Transparency##1337", (bool*)&info.enableTransparency);

				ImGui::SliderFloat("Opacity", &info.transparencyFactor, 0.f, 1.f);

				ImGui::Text("Transparency Map");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.transparencyMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.transparencyMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(123456789); id > -1)
				{
					info.transparencyMapIndex = id;
				}

				if (info.transparencyMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##0000"))
						info.transparencyMapIndex = -1;
				}

				// Glow
				ImGui::Checkbox("Enable Glow##1337", (bool*)&info.enableGlow);

				ImGui::SliderFloat("Glow Intensity##1337", &info.glowIntensity, 0.f, 10.f);

				ImGui::Text("Glow Map");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.glowMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->getSRVHandle().getGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureToMaterialDragDrop"))
					{
						int payloadData = *(const int*)payload->Data;
						info.glowMapIndex = payloadData;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				if (int id = this->applyTexturePopup(223456789); id > -1)
				{
					info.glowMapIndex = id;
				}

				if (info.glowMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##00000"))
						info.glowMapIndex = -1;
				}

				ImGui::ColorPicker3("Glow Full Color##1111", (float*)&info.glowFullColor);
			}
		}
	}
	ImGui::End();
}

void LevelEditorUI::showMeshes()
{
	if (ImGui::Begin("Meshes"))
	{
		Texture2DCache& tCache = engine->GetTexture2DCache();
		auto& modelCache = engine->GetModelCache();
		MaterialManager& mManager = engine->GetMaterialManager();
		std::vector<aZeroECS::Entity>& entityVec = currentScene->GetEntityVector();

		if (ImGui::Button("Load"))
		{
			this->LoadModelDataFromDirectory();
		}

		if (selMeshID != -1)
		{
			if (selMeshName != "defaultCube" && selMeshName != "defaultSphere")
			{
				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{
					if (modelCache.Exists(selMeshID))
					{
						if (currentScene)
						{
							for (auto& ent : entityVec)
							{
								Mesh* meshComp = currentScene->GetComponentForEntity<Mesh>(ent);
								if (meshComp && (selMeshID == meshComp->GetID()))
								{
									meshComp->SetID(modelCache.GetID("defaultCube"));
								}
							}
						}

						modelCache.RemoveResource(selMeshID);
						selMeshID = -1;
						selMeshName = "";
					}
				}
			}
		}

		if (ImGui::BeginListBox("##133333377"))
		{
			for (ModelAsset& model : modelCache.GetAllResources())
			{
				const std::string name = model.getMeshName();

				if (ImGui::Selectable(name.c_str()))
				{
					selMeshName = name;
					selMeshID = modelCache.GetID(name);
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					const std::string tx = "Name: " + name + 
						"\nNum Vertices: " + std::to_string(model.getNumVertices()) +
						"\nNum Indices: " + std::to_string(model.getNumIndices()) +
						"\nBounding Radius: " + std::to_string(model.getBoundingRadius());
					ImGui::Text(tx.c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::BeginDragDropSource())
				{
					int meshIndex = modelCache.GetID(name);
					ImGui::SetDragDropPayload("MeshToComponentDragDrop", (void*)&meshIndex, sizeof(int));

					ImGui::EndDragDropSource();
				}
			}

			ImGui::EndListBox();
		}
	}
	ImGui::End();
}

void LevelEditorUI::showFileTextures()
{
	if (ImGui::Begin("File Textures"))
	{
		Texture2DCache& tCache = engine->GetTexture2DCache();
		MaterialManager& mManager = engine->GetMaterialManager();

		if (ImGui::Button("Load"))
		{
			std::string fileName = "";
			if (Helper::OpenFileDialogForExtension({ ".png" , ".jpg", ".dds"}, fileName))
			{
				GraphicsContextHandle context = engine->GetCommandManager().getGraphicsContext();
				tCache.LoadResource(engine->GetDevice(), context,
					engine->GetFrameIndex(), fileName, "..\\textures\\");
				engine->GetCommandManager().executeContext(context);
			}
		}

		if (selTexture2DID != -1)
		{
			if (selTexture2DName != "defaultDiffuse.png")
			{
				ImGui::SameLine();
				if (ImGui::Button("Delete"))
				{
					int heapIndexToDelete = tCache.GetResource(selTexture2DID)->getSRVHandle().getHeapIndex();
					for (auto& mat : mManager.GetPBRMaterials())
					{
						if (mat.GetInfo().albedoMapIndex == heapIndexToDelete)
						{
							mat.GetInfo().albedoMapIndex = tCache.GetResource("defaultDiffuse.png")->getSRVHandle().getHeapIndex();
						}
						else if (mat.GetInfo().normalMapIndex == heapIndexToDelete)
						{
							mat.GetInfo().normalMapIndex = -1;
						}
						else if (mat.GetInfo().roughnessMapIndex == heapIndexToDelete)
						{
							mat.GetInfo().roughnessMapIndex = -1;
						}
						else if (mat.GetInfo().metallicMapIndex == heapIndexToDelete)
						{
							mat.GetInfo().metallicMapIndex = -1;
						}
					}

					tCache.RemoveResource(selTexture2DID);
					selTexture2DID = -1;
					selTexture2DName = "";
				}
			}
		}
		
		if (ImGui::BeginListBox("##13333337", ImVec2(ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y)))
		{
			for (const Texture& texture : tCache.GetAllResources())
			{
				const std::string name = tCache.GetFileNameByHeapIndex(texture.getSRVHandle().getHeapIndex());
				
				ImGui::SameLine();
				if (selTexture2DID == tCache.GetID(name))
				{
					if (ImGui::ImageButton((ImTextureID)texture.getSRVHandle().getGPUHandle().ptr, ImVec2(80.f, 80.f), 
						ImVec2(0,0), ImVec2(1, 1), 3, ImVec4(1,0,0,1), ImVec4(1,1,1,1)))
					{
						selTexture2DName = name;
						selTexture2DID = tCache.GetID(name);
					}
				}
				else
				{
					if (ImGui::ImageButton((ImTextureID)texture.getSRVHandle().getGPUHandle().ptr, ImVec2(80.f, 80.f),
						ImVec2(0, 0), ImVec2(1, 1), 1, ImVec4(0, 1, 0, 1), ImVec4(0.3, 0.3, 0.3, 1)))
					{
						selTexture2DName = name;
						selTexture2DID = tCache.GetID(name);
					}
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					const std::string tx = "Name: " + name + "\nDimensions: " + std::to_string(static_cast<int>(texture.getWidth()))
						+ " x " + std::to_string(static_cast<int>(texture.getHeight()))
							+ "\nMip Levels: " + std::to_string(static_cast<int>(texture.getMipLevels()));
					ImGui::Text(tx.c_str());
					ImGui::EndTooltip();
				}

				if (ImGui::BeginDragDropSource())
				{
					int textureHeapIndex = texture.getSRVHandle().getHeapIndex();
					ImGui::SetDragDropPayload("TextureToMaterialDragDrop", (void*)&textureHeapIndex, sizeof(int));
					ImGui::Image((ImTextureID)texture.getSRVHandle().getGPUHandle().ptr, { 30,30 });
					ImGui::EndDragDropSource();
				}
			}

			ImGui::EndListBox();
		}
	}
	ImGui::End();
}
