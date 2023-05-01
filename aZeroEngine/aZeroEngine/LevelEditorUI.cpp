#include "LevelEditorUI.h"
#include <filesystem>

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

	ImGui::End();
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

std::optional<std::string> LevelEditorUI::LoadMeshFromDirectory()
{
	std::string fbxNameWithExt = "";

	if (Helper::OpenFileDialogForExtension(".fbx", fbxNameWithExt))
	{
		std::string::size_type const p(fbxNameWithExt.find_last_of('.'));
		std::string fileNameWithoutExt = fbxNameWithExt.substr(0, p);

		GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();

		engine->GetModelCache().LoadResource(engine->GetDevice(), context, 
			engine->GetFrameIndex(), fileNameWithoutExt, "..\\meshes\\");

		engine->GetCommandManager().ExecuteContext(context);
		return fileNameWithoutExt;
	}
	return {};
}

std::optional<std::string> LevelEditorUI::LoadPBRMaterialFromDirectory()
{
	std::string matNameWithExt = "";

	if (Helper::OpenFileDialogForExtension(".azmpbr", matNameWithExt))
	{
		std::string::size_type const p(matNameWithExt.find_last_of('.'));
		std::string fileNameWithoutExt = matNameWithExt.substr(0, p);

		GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();

		engine->GetMaterialManager().LoadMaterial<PBRMaterial>(engine->GetDevice(), context,
			engine->GetFrameIndex(), fileNameWithoutExt);

		engine->GetCommandManager().ExecuteContext(context);

		return fileNameWithoutExt;
	}
	return {};
}

void LevelEditorUI::Update()
{
	ShowPerformanceData();
	if (editorMode)
	{
		ShowSceneWindow();

		if (InputManager::MouseBtnDown(LEFT))
		{
			if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			{
				if (!ImGuizmo::IsOver())
				{
					std::optional<Vector2> mousePos = engine->GetMouseWindowPosition();
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

		if (currentScene)
			ShowEntityWindow();

		ShowMaterialWindow();
		ShowResourceWindow();
	}
}

void LevelEditorUI::DrawEntityComponents(Entity& _entity)
{
	ImGui::ShowDemoWindow();

	std::shared_ptr<Camera> cam = camera.lock();
	Transform* tf = currentScene->GetComponentForEntity<Transform>(_entity);

	Entity& rootEntity = currentScene->GetEntity(selectionList.GetRoot());
	if (!editingPLight)
	{
		if (tf)
		{
			/*static float rotTemp = 0.f;
			Vector3 s;
			Vector3 y;
			Matrix rot = Matrix::CreateRotationY(rotTemp);
			Quaternion rotQ = Quaternion::CreateFromRotationMatrix(rot);
			rot.Decompose(s, rotQ, y);
			tf->GetRotation() = rotQ.ToEuler();
			rotTemp += 0.005f;*/

			static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
			static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

			if (InputManager::KeyDown('W'))
				mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			if (InputManager::KeyDown('R'))
				mCurrentGizmoOperation = ImGuizmo::ROTATE;
			if (InputManager::KeyDown('E'))
				mCurrentGizmoOperation = ImGuizmo::SCALE;

			ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
			Matrix matrix = tf->GetWorldMatrix();

			if (ImGuizmo::Manipulate(&cam->GetView()._11, &cam->GetProj()._11, mCurrentGizmoOperation, mCurrentGizmoMode, &matrix._11, NULL, 0))
			{
				int parentID = engine->GetParentSystem().lock()->GetParentEntityID(rootEntity);

				if (parentID != -1)
				{
					Entity& parentEnt = currentScene->GetEntity(parentID);

					Transform* parentTF = currentScene->GetComponentForEntity<Transform>(parentEnt);
					if (parentTF)
					{
						Matrix wParent = parentTF->GetWorldMatrix();
						Matrix local = matrix * wParent.Invert();
						Quaternion rotationDegQuat;
						local.Decompose(tf->GetScale(), rotationDegQuat, tf->GetTranslation());
						tf->GetRotation() = rotationDegQuat.ToEuler();
					}
				}
				else
				{
					Quaternion rotationDegQuat;
					matrix.Decompose(tf->GetScale(), rotationDegQuat, tf->GetTranslation());
					tf->GetRotation() = rotationDegQuat.ToEuler();
				}
			}
		}
	}


	static int selComponentPopup = -1;
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("CompPopup");

	if (ImGui::BeginPopup("CompPopup"))
	{
		for (int i = 0; i < COMPONENTENUM::MAX - 1; i++)
		{
			if (!_entity.componentMask[i])
			{
				if (ImGui::Selectable(COMPONENTENUM::COMPONENTNAMES[i].c_str()))
				{
					std::cout << COMPONENTENUM::COMPONENTNAMES[i] << "\n";
					COMPONENTENUM::COMPONENTBITID enumBit = (COMPONENTENUM::COMPONENTBITID)i;
					switch (enumBit)
					{
					case COMPONENTENUM::TRANSFORM:
					{
						Transform comp;
						currentScene->AddComponentToEntity<Transform>(_entity, std::move(comp));
						break;
					}
					case COMPONENTENUM::MESH:
					{
						Mesh comp;
						comp.SetID(engine->GetModelCache().GetID("defaultCube"));
						currentScene->AddComponentToEntity<Mesh>(_entity, std::move(comp));

						MaterialComponent compMat;
						compMat.materialID = engine->GetMaterialManager().GetReferenceID<PBRMaterial>("DefaultPBRMaterial");
						compMat.type = MATERIALTYPE::PBR;
						currentScene->AddComponentToEntity<MaterialComponent>(_entity, std::move(compMat));
						break;
					}
					case COMPONENTENUM::MATERIAL:
					{
						MaterialComponent comp;
						comp.materialID = engine->GetMaterialManager().GetReferenceID<PBRMaterial>("DefaultPBRMaterial");
						comp.type = MATERIALTYPE::PBR;
						currentScene->AddComponentToEntity<MaterialComponent>(_entity, std::move(comp));
						break;
					}
					case COMPONENTENUM::PLIGHT:
					{
						PointLightComponent comp;
						currentScene->AddComponentToEntity<PointLightComponent>(_entity, std::move(comp));
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
		for (int i = 0; i < COMPONENTENUM::MAX - 1; i++)
		{
			if (_entity.componentMask[i])
			{
				if (ImGui::TreeNode((void*)(intptr_t)i, COMPONENTENUM::COMPONENTNAMES[i].c_str()))
				{
					COMPONENTENUM::COMPONENTBITID enumBit = (COMPONENTENUM::COMPONENTBITID)i;

					switch (enumBit)
					{
					case COMPONENTENUM::TRANSFORM:
					{
						if (tf)
						{
							ImGui::InputFloat3("Translation", &tf->GetTranslation().x);

							Quaternion rotInDegrees(tf->GetRotation());
							Vector3 rotInDeg = tf->GetRotation();
							rotInDeg.x = rotInDeg.x * (180 / 3.14f);
							rotInDeg.y = rotInDeg.y * (180 / 3.14f);
							rotInDeg.z = rotInDeg.z * (180 / 3.14f);
							if (ImGui::InputFloat3("Rotation", &rotInDeg.x))
							{
								Vector3 rotInRad = Vector3(rotInDeg.x / (180 / 3.14f), rotInDeg.y / (180 / 3.14f), rotInDeg.z / (180 / 3.14f));
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
					case COMPONENTENUM::MESH:
					{
						Mesh* meshComp = currentScene->GetComponentForEntity<Mesh>(rootEntity);
						if (meshComp)
						{
							auto& modelCache = engine->GetModelCache();
							ImGui::Text("Current: ");
							ImGui::SameLine();
							ImGui::Text(modelCache.GetFileName(meshComp->GetID()).c_str());

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
									std::optional<std::string> loadedName = LoadMeshFromDirectory();
									if (loadedName.has_value())
										meshComp->SetID(modelCache.GetID(loadedName.value()));
								}
								else
								{
									for (ModelAsset& vb : vbVec)
									{
										if (ImGui::Selectable(vb.GetMeshName().c_str()))
										{
											meshComp->SetID(modelCache.GetID(vb.GetMeshName()));
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
					case COMPONENTENUM::MATERIAL:
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
									for (PBRMaterial& mat : pbrMaterials)
									{
										if (ImGui::Selectable(mat.GetName().c_str()))
										{
											matComp->materialID = mManager.GetReferenceID<PBRMaterial>(mat.GetName());
											break;
										}
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
					case COMPONENTENUM::PLIGHT:
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

									Matrix lightMat = Matrix::CreateTranslation(pLight->position.x, pLight->position.y, pLight->position.z);

									if (ImGuizmo::Manipulate(&cam->GetView()._11, &cam->GetProj()._11,
										imGuizmoOpPLight, mCurrentGizmoMode, &lightMat._11, NULL, 0))
									{
										Quaternion emptyRot;
										Vector3 emptyScale;
										Vector3 tempTransLight;
										lightMat.Decompose(emptyScale, emptyRot, tempTransLight);
										pLight->position = Vector3(tempTransLight.x, tempTransLight.y, tempTransLight.z);

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
			ImGuizmo::Enable(false);
			if (ImGui::Button("New Scene"))
				currentScene = engine->NewScene("Empty");
		}
		else
		{
			const std::string sceneNameText = "Scene: " + currentScene->GetName();
			ImGui::Text(sceneNameText.c_str());
		}

		if (ImGui::Button("Open Scene"))
		{
			std::string sceneNameWithExt = "";
			if (Helper::OpenFileDialogForExtension(".azs", sceneNameWithExt))
			{
				std::string::size_type const p(sceneNameWithExt.find_last_of('.'));
				std::string sceneNameWithoutExt = sceneNameWithExt.substr(0, p);

				if (currentScene)
					currentScene.reset();

				GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();
				currentScene = engine->LoadScene(context, "..\\scenes\\", sceneNameWithoutExt);
				engine->GetCommandManager().ExecuteContext(context);

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
				ImGui::OpenPopup("OverwriteScenePopup");

				if (sceneName.size() == 0)
				{
					const std::string fileFullPath = "..\\scenes\\" + currentScene->GetName() + ".azs";
					std::filesystem::path filePath(fileFullPath);

					if (std::filesystem::exists(filePath))
					{
						if (ImGui::BeginPopupModal("OverwriteScenePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
	ImGui::End();
}

void LevelEditorUI::ShowEntityWindow()
{
	if (ImGui::Begin("Entity Editor"))
	{
		std::vector<Entity>& entityVec = currentScene->GetEntityVector();

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
			Entity& ent = currentScene->CreateEntity("Entity_");
		}

		int rootEntity = selectionList.GetRoot();
		if (rootEntity != -1)
		{
			Entity& currentEnt = currentScene->GetEntity(rootEntity);

			if (ImGui::Button("Add Child"))
				ImGui::OpenPopup("AddChildPopup");

			if (ImGui::BeginPopup("AddChildPopup"))
			{
				std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
				std::vector<int> childrenIDs = pSys->GetChildrenEntityID(currentEnt);

				for (Entity& ent : entityVec)
				{
					bool isChild = false;
					for (auto id : childrenIDs)
					{
						if (id == ent.id)
						{
							isChild = true;
							break;
						}
					}

					if (pSys->IsChildInBranch(ent, currentEnt.id))
						continue;

					if (!isChild)
					{
						if (ent.id != rootEntity)
						{
							int parId = pSys->GetParentEntityID(currentEnt);
							if (parId != ent.id)
							{
								std::optional<std::string> entIDStr = currentScene->GetEntityName(ent);
								if (entIDStr.has_value())
								{
									if (ImGui::Selectable(entIDStr.value().c_str()))
									{
										if (ent.id != rootEntity)
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
				Entity& currentEnt = currentScene->GetEntity(rootEntity);
				int parentID = pSys->GetParentEntityID(currentEnt);
				if (parentID != -1)
				{
					Entity tempParent;
					tempParent.id = parentID;
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

void LevelEditorUI::ShowEntityHierarchy(const Entity& _current)
{
	std::optional<std::string> name = currentScene->GetEntityName(_current);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if(selectionList.Selected(_current.id))
		flags |= ImGuiTreeNodeFlags_Selected;

	std::shared_ptr<ParentSystem> pSys = engine->GetParentSystem().lock();
	std::vector<int> childrenIDs = pSys->GetChildrenEntityID(_current);

	if (childrenIDs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (ImGui::TreeNodeEx((void*)(intptr_t)_current.id, flags, name.value().c_str()))
	{
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			if (InputManager::KeyHeld(VK_SHIFT))
			{
				if (selectionList.Selected(_current.id))
					selectionList.Remove(_current.id);
				else
					selectionList.Add(_current.id);
			}
			else
			{
				selectionList.Clear();
				selectionList.Add(_current.id);
			}
		}

		for (int id : childrenIDs)
		{
			Entity ent;
			ent.id = id;
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
				GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();
				mManager.CreateMaterial<PBRMaterial>(engine->GetDevice(), context, engine->GetFrameIndex(), newMatTempName);
				engine->GetCommandManager().ExecuteContext(context);
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
			if (Helper::OpenFileDialogForExtension(".azmpbr", matNameWithExt))
			{
				std::string::size_type const p(matNameWithExt.find_last_of('.'));
				std::string matNameWithoutExt = matNameWithExt.substr(0, p);

				GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();
				mManager.LoadMaterial<PBRMaterial>(engine->GetDevice(), context,
					engine->GetFrameIndex(), matNameWithoutExt);
				engine->GetCommandManager().ExecuteContext(context);

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
					ImGui::Image((ImTextureID)t->GetSRVHandle().GetGPUHandle().ptr, ImVec2(70, 70));

				ImGui::SameLine();
				if (ImGui::Button("Apply Selected Texture##0"))
				{
					t = engine->GetTexture2DCache().GetResource(selTexture2DID);
					if (t)
					{
						info.albedoMapIndex = t->GetSRVHandle().GetHeapIndex();
					}
				}

				// Roughness
				ImGui::Text("Roughness Texture");
				t = engine->GetTexture2DCache().GetResource(engine->GetTexture2DCache().GetTextureName(info.roughnessMapIndex));
				if (t)
					ImGui::Image((ImTextureID)t->GetSRVHandle().GetGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				ImGui::SameLine();
				if (ImGui::Button("Apply Selected Texture##00"))
				{
					t = engine->GetTexture2DCache().GetResource(selTexture2DID);
					if (t)
						info.roughnessMapIndex = t->GetSRVHandle().GetHeapIndex();
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
					ImGui::Image((ImTextureID)t->GetSRVHandle().GetGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				ImGui::SameLine();
				if (ImGui::Button("Apply Selected Texture##000"))
				{
					Texture* tt = engine->GetTexture2DCache().GetResource(selTexture2DID);
					if (tt)
						info.metallicMapIndex = tt->GetSRVHandle().GetHeapIndex();
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
					ImGui::Image((ImTextureID)t->GetSRVHandle().GetGPUHandle().ptr, ImVec2(70, 70));
				else
					ImGui::Text("NULL");

				ImGui::SameLine();
				if (ImGui::Button("Apply Selected Texture##0000"))
				{
					Texture* tt = engine->GetTexture2DCache().GetResource(selTexture2DID);
					if (tt)
						info.normalMapIndex = tt->GetSRVHandle().GetHeapIndex();
				}

				if (info.normalMapIndex != -1)
				{
					if (ImGui::Button("Remove Texture##000"))
						info.normalMapIndex = -1;
				}
			}
		}

	}
	ImGui::End();
}

void LevelEditorUI::ShowResourceWindow()
{
	if (ImGui::Begin("Resources"))
	{
		Texture2DCache& tCache = engine->GetTexture2DCache();
		auto& modelCache = engine->GetModelCache();
		MaterialManager& mManager = engine->GetMaterialManager();
		std::vector<Entity>& entityVec = currentScene->GetEntityVector();

		if (ImGui::BeginListBox("Meshes"))
		{
			for (auto& vb : modelCache.GetAllResources())
			{
				const std::string name = vb.GetMeshName();
				if (ImGui::Selectable(name.c_str()))
				{
					selMeshName = name;
					selMeshID = modelCache.GetID(name);
				}
			}

			ImGui::EndListBox();
		}

		if (selMeshID != -1)
		{
			const std::string t = "Selected Mesh: " + selMeshName;
			ImGui::Text(t.c_str());

			ModelAsset* selVB = modelCache.GetResource(selMeshID);
			const std::string tx = "Number of Vertices: " + std::to_string(selVB->GetNumVertices());
			ImGui::Text(tx.c_str());
		}

		if (ImGui::Button("Load Mesh"))
		{
			std::optional<std::string> loadedName = LoadMeshFromDirectory();
			if (loadedName.has_value())
			{
				GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();
				modelCache.LoadResource(engine->GetDevice(), context,
					engine->GetFrameIndex(), loadedName.value(), "..\\meshes\\");
				engine->GetCommandManager().ExecuteContext(context);
			}
		}

		if (selMeshID != -1)
		{
			if (selMeshName != "defaultCube" && selMeshName != "defaultSphere")
			{
				if (ImGui::Button("Delete Mesh"))
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

		if (ImGui::BeginListBox("Textures"))
		{
			for (auto& texture : tCache.GetAllResources())
			{
				const std::string name = tCache.GetFileNameByHeapIndex(texture.GetSRVHandle().GetHeapIndex());
				const std::string selName = name + "##1";
				if (ImGui::Selectable(selName.c_str()))
				{
					selTexture2DName = name;
					selTexture2DID = tCache.GetID(name);
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::Button("Load Texture"))
		{
			std::string fileName = "";
			if (Helper::OpenFileDialogForExtension(".png", fileName))
			{

				GraphicsContextHandle context = engine->GetCommandManager().GetGraphicsContext();
				tCache.LoadResource(engine->GetDevice(), context,
					engine->GetFrameIndex(), fileName, "..\\textures\\");
				engine->GetCommandManager().ExecuteContext(context);
			}
		}

		if (selTexture2DID != -1)
		{
			Texture* selText = tCache.GetResource(selTexture2DID);

			const std::string t = "Selected Texture: " + selTexture2DName;
			ImGui::Text(t.c_str());

			const std::string tx = "Dimensions: " + std::to_string(selText->GetDimensions().x) + " : " + std::to_string(selText->GetDimensions().y);
			ImGui::Text(tx.c_str());

			if (selText)
				ImGui::Image((ImTextureID)selText->GetSRVHandle().GetGPUHandle().ptr, ImVec2(100, 100));

			if (selTexture2DName != "defaultDiffuse.png")
			{
				// Move deleting to the scene
				if (ImGui::Button("Delete Texture"))
				{
					int heapIndexToDelete = tCache.GetResource(selTexture2DID)->GetSRVHandle().GetHeapIndex();
					for (auto& mat : mManager.GetPBRMaterials())
					{
						if (mat.GetInfo().albedoMapIndex == heapIndexToDelete)
						{
							mat.GetInfo().albedoMapIndex = tCache.GetResource("defaultDiffuse.png")->GetSRVHandle().GetHeapIndex();
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

	}
	ImGui::End();
}
