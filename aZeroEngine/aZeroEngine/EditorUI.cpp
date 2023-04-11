#include "EditorUI.h"
#include "PBRMaterial.h"

void EditorUI::ShowPerformanceData()
{
	//------------------------Performance Data Window------------------------
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

	{
		/*ImGui::Text("Camera Transform");
		std::string p = "Position: ";
		Vector3 po = graphics.renderSystem->GetMainCameraGeo()->GetPosition();
		p += std::to_string(po.x) + " " + std::to_string(po.y) + " " + std::to_string(po.z);
		ImGui::Text(p.c_str());*/

	}

	ImGui::Text("Average MS and FPS: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

void EditorUI::Update()
{
//	if (editorMode)
//	{
//		ShowSceneManager();
//		ShowEntityEditor();
//		ShowMaterialEditor();
//		ShowGlobalLightingManager();
//		//ShowDebugTextures();
//
//		ImGui::Begin("Resources");
//		ShowLoadedVertexBuffers();
//		ShowLoadedTextures();
//		ImGui::End();
//
//		//ImGui::ShowDemoWindow();
//		//ImGui::ShowStyleEditor();
//		ShowPerformanceData();
//		
	//}

	//// TEMP
	//if (selectedEntityID != -1)
	//{
	//	MaterialComponent* temp = graphics.scene->GetComponentForEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID));
	//	if (temp)
	//	{
	//		temp->materialID = graphics.materialManager.GetReferenceID<PBRMaterial>("GoblinMaterial");
	//	}
	//}

	//if (showPreview)
	//{
	//	DrawMaterialPreview();
	//	ShowMaterialPreview();
	//}
}

void EditorUI::ShowSettings()
{
	ImGui::Begin("Project Settings");
	static int sel = -1;
	static int lastSel = -1;
	if (ImGui::BeginListBox("Resolution"))
	{
		const char* resolutions[] = { "800x600", "1920x1080" };
		
		for (int i = 0; i < ARRAYSIZE(resolutions); i++)
		{
			if (ImGui::Selectable(resolutions[i]))
			{
				sel = i;
				break;
			}
		}
		ImGui::EndListBox();
	}

	if (sel != lastSel)
	{
		switch (sel)
		{
		case 0: // 800x600
		{

			break;
		}
		case 1: // 1920x1080
		{

			break;
		}
		}
	}

	lastSel = sel;

	ImGui::End();
}

void EditorUI::ShowMaterialEditor()
{
	//------------------------Material Editor------------------------
	ImGui::Begin("Material Editor");

	/*if (ImGui::Button("Show Preview"))
	{
		showPreview = !showPreview;
	}*/

	if (ImGui::BeginTabBar("Materials"))
	{
		if (ImGui::BeginTabItem("PBR Materials"))
		{
			PBRMatEditing();

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Phong Materials"))
		{
			PhongMatEditing();

			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}

	
	ImGui::End();
	//-----------------------------------------------------------------------
}

void EditorUI::ShowEntityEditor()
{
	// Available entities && entity selection
	if (graphics.scene != nullptr)
	{

		ImGui::Begin("Entity Editor");

		//------------------------Entity Editor------------------------

		//if (graphics.scene->entities.GetObjects().size() > 0)
		//{
		//	if (ImGui::BeginListBox("Entities"))
		//	{
		//		for (auto& ent : graphics.scene->entities.GetObjects())
		//		{
		//			const std::string name = graphics.scene->GetEntityName(ent);
		//			bool selected = false;
		//			if (ent.id == selectedEntityID)
		//				selected = true;

		//			if (ImGui::Selectable(std::string(name).c_str(), selected))
		//			{
		//				//selectedEntityStr = name;
		//				selectedEntityID = ent.id;
		//				break;
		//			}
		//		}
		//		ImGui::EndListBox();

		//		ImGui::Text("Selected Entity: ");
		//		ImGui::SameLine();
		//		if(selectedEntityID != -1)
		//			ImGui::Text(graphics.scene->GetEntityName(graphics.scene->GetEntity(selectedEntityID)).c_str());
		//	}
		//}
		//-----------------------------------------------------------------

		//------------------------Create New Entity------------------------------
		static int counter = 0;
		if (ImGui::Button("Create Entity"))
		{
			//Entity& ent = graphics.scene->CreateEntity(graphics.device, graphics.resourceEngine);
			////selectedEntityStr = graphics.scene->GetEntityName(ent);
			//selectedEntityID = ent.id;
			//counter++;
		}
		//-----------------------------------------------------------------------

		if (selectedEntityID != -1)
		{
			//----------------------------Delete Entity------------------------------
			if (ImGui::Button("Delete Entity"))
			{
				graphics.scene->DeleteEntity(selectedEntityID);
				//selectedEntityStr = "";
				selectedEntityID = -1;
			}
			//-----------------------------------------------------------------------

			static char newName[40] = { 0 };
			ImGui::InputText("New Name", newName, ARRAYSIZE(newName));

			//-----------------------------Rename Entity-----------------------------
			if (ImGui::Button("Rename Entity"))
			{
				const std::string newNameStr(newName);
				if (newNameStr != "")
				{
					if (!graphics.scene->EntityExists(newNameStr))
					{
						graphics.scene->RenameEntity(graphics.scene->GetEntityName(graphics.scene->GetEntity(selectedEntityID)), newName);
						//selectedEntityStr = newName;
						ZeroMemory(newName, ARRAYSIZE(newName));
					}
				}
			}
			//-----------------------------------------------------------------------
		}

		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

		if (selectedEntityID != -1)
		{
			Transform* tf = graphics.ecs.GetComponentManager().GetComponent<Transform>(graphics.scene->GetEntity(selectedEntityID));
			if (!editingPLight)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_W))
					mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
				if (ImGui::IsKeyPressed(ImGuiKey_R))
					mCurrentGizmoOperation = ImGuizmo::ROTATE;
				if (ImGui::IsKeyPressed(ImGuiKey_E))
					mCurrentGizmoOperation = ImGuizmo::SCALE;

				ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
				Matrix editMat = tf->Compose();

				/*if (ImGuizmo::Manipulate(&graphics.renderSystem->GetMainCameraGeo()->GetView()._11, &graphics.renderSystem->GetMainCameraGeo()->GetProj()._11, mCurrentGizmoOperation, mCurrentGizmoMode, &editMat._11, NULL, 0))
				{
					Quaternion tempRotDegrees(tf->GetRotation());
					editMat.Decompose(tf->GetScale(), tempRotDegrees, tf->GetTranslation());

					Vector3 tempRotRad = tempRotDegrees.ToEuler();
					tf->SetRotation(tempRotRad);

					tf->Update(graphics.resourceEngine, graphics.frameIndex);
				}*/
			}

			//------------------------Components For Selected Entity------------------------
			const char* comps[] = { "Transform", "Mesh", "Material", "Point Light"};
			if (ImGui::TreeNode("Components"))
			{
				//graphics.WaitForGPU();
				for (int i = 0; i < ARRAYSIZE(comps); i++)
				{
					if (ImGui::TreeNode((void*)(intptr_t)i, comps[i]))
					{
						switch (i)
						{
						case 0:	// Transform Component
						{

							if (ImGui::InputFloat3("Translation", &tf->GetTranslation().x))
							{
								tf->Update(graphics.resourceEngine);
							}

							Quaternion rotInDegrees(tf->GetRotation());
							Vector3 rotInDeg = tf->GetRotation();
							rotInDeg.x = rotInDeg.x * (180 / 3.14f);
							rotInDeg.y = rotInDeg.y * (180 / 3.14f);
							rotInDeg.z = rotInDeg.z * (180 / 3.14f);
							if (ImGui::InputFloat3("Rotation", &rotInDeg.x))
							{
								Vector3 rotInRad = Vector3(rotInDeg.x / (180 / 3.14f), rotInDeg.y / (180 / 3.14f), rotInDeg.z / (180 / 3.14f));
								tf->SetRotation(rotInRad);
								tf->Update(graphics.resourceEngine);
							}

							if (ImGui::InputFloat3("Scale", &tf->GetScale().x))
							{
								tf->Update(graphics.resourceEngine);
							}

							break;
						}
						case 1: // Mesh Component
						{
							Mesh* meshComp = graphics.scene->GetComponentForEntity<Mesh>(graphics.scene->GetEntity(selectedEntityID));

							// Should remove the old deprecated Windows functions and replace with the IFileDialog API...
							if (ImGui::Button("Load Vertex Buffer"))
							{
								std::string fbxNameWithExt = "";

								if (Helper::OpenFileDialogForExtension(".fbx", fbxNameWithExt))
								{
									std::string::size_type const p(fbxNameWithExt.find_last_of('.'));
									std::string fileNameWithoutExt = fbxNameWithExt.substr(0, p);

									graphics.vbCache.LoadResource(fileNameWithoutExt, "..\\meshes\\");
									if (meshComp == nullptr)
									{
										Mesh tempMesh(graphics.vbCache.GetID(fileNameWithoutExt));
										graphics.scene->AddComponentToEntity<Mesh>(graphics.scene->GetEntity(selectedEntityID), tempMesh);

										MaterialComponent tempMat(graphics.materialManager.GetReferenceID<PhongMaterial>("DefaultPhongMaterial"));
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID), tempMat);
									}
									else
									{
										meshComp->SetID(graphics.vbCache.GetID(fileNameWithoutExt));
									}
								}
							}

							const std::string selMeshTemp = "Attach Selected Mesh: " + selectedMeshStr;
							if (ImGui::Button(selMeshTemp.c_str()))
							{
								if (selectedMeshID != -1)
								{
									if (meshComp == nullptr)
									{
										Mesh tempMesh(graphics.vbCache.GetID(selectedMeshStr));
										graphics.scene->AddComponentToEntity<Mesh>(graphics.scene->GetEntity(selectedEntityID), tempMesh);

										MaterialComponent tempMat(graphics.materialManager.GetReferenceID<PhongMaterial>("DefaultPhongMaterial"));
										tempMat.type = MATERIALTYPE::PHONG;
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID), tempMat);
									}
									else
									{
										meshComp->SetID(graphics.vbCache.GetID(selectedMeshStr));
									}
								}
							}

							if (meshComp != nullptr)
							{
								std::string vbName = "Current Vertex Buffer: " + graphics.vbCache.GetResource(meshComp->GetID())->GetName();
								ImGui::Text(vbName.c_str());

								if (ImGui::Checkbox("Cast Shadows", &meshComp->castShadows));

								bool recShadows = false;
								if (meshComp->receiveShadows == 1.f)
								{
									recShadows = true;
								}

								if (ImGui::Checkbox("Receive Shadows", &recShadows))
								{
									if (recShadows)
									{
										meshComp->receiveShadows = 1.f;
									}
									else
									{
										meshComp->receiveShadows = 0.f;
									}
								}
							}
							else
							{
								ImGui::Text("Current Vertex Buffer: NULL");
							}

							break;
						}
						case 2: // Material Component
						{
							MaterialComponent* matComp = graphics.scene->GetComponentForEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID));

							std::string btnName = "Attach Selected Material ( PHONG ): " + selectedPhongMaterialStr;

							if (ImGui::Button(btnName.c_str()))
							{
								if (selectedPhongMaterialStr != "")
								{
									if (matComp == nullptr)
									{
										MaterialComponent tempMat(graphics.materialManager.GetReferenceID<PhongMaterial>(selectedPhongMaterialStr));
										tempMat.type = MATERIALTYPE::PHONG;
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID), tempMat);
									}
									else
									{
										matComp->type = MATERIALTYPE::PHONG;
										matComp->materialID = graphics.materialManager.GetReferenceID<PhongMaterial>(selectedPhongMaterialStr);
									}
								}

							}

							btnName = "Attach Selected Material ( PBR ): " + selectedPBRMaterialStr;

							if (ImGui::Button(btnName.c_str()))
							{
								if (selectedPBRMaterialStr != "")
								{
									if (matComp == nullptr)
									{
										MaterialComponent tempMat(graphics.materialManager.GetReferenceID<PBRMaterial>(selectedPBRMaterialStr));
										tempMat.type = MATERIALTYPE::PBR;
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->GetEntity(selectedEntityID), tempMat);
									}
									else
									{
										matComp->type = MATERIALTYPE::PBR;
										matComp->materialID = graphics.materialManager.GetReferenceID<PBRMaterial>(selectedPBRMaterialStr);
									}
								}

							}

							if (matComp != nullptr)
							{
								// Info about what type of material...
								if (matComp->type == MATERIALTYPE::PHONG)
								{
									PhongMaterial* pMat = graphics.materialManager.GetMaterial<PhongMaterial>(matComp->materialID);
									if (pMat != nullptr)
									{
										std::string matName = "Current Material: " + pMat->GetName();
										ImGui::Text(matName.c_str());
									}
								}
								else if (matComp->type == MATERIALTYPE::PBR)
								{
									PBRMaterial* pbrMat = graphics.materialManager.GetMaterial<PBRMaterial>(matComp->materialID);
									if (pbrMat != nullptr)
									{
										std::string matName = "Current Material: " + pbrMat->GetName();
										ImGui::Text(matName.c_str());
									}
								}
								//
								
							}
							else
							{
								ImGui::Text("Current Material: NULL");
							}


							break;
						}
						case 3:
						{
							PointLightComponent* plComp = graphics.scene->GetComponentForEntity<PointLightComponent>(graphics.scene->GetEntity(selectedEntityID));
							if (!plComp)
							{
								editingPLight = false;
								if (ImGui::Button("Add Point Light"))
								{
									PointLightComponent pComp;
									graphics.scene->AddComponentToEntity<PointLightComponent>(graphics.scene->GetEntity(selectedEntityID), pComp);
								}
							}
							else
							{
								PointLight* pLight = graphics.lightSystem->GetLightManager()->GetLight<PointLight>(plComp->id);
								if (pLight)
								{

									if (ImGui::Button("Edit Light Position"))
									{
										editingPLight = !editingPLight;
									}

									if (editingPLight)
									{
										static ImGuizmo::OPERATION imGuizmoOpPLight(ImGuizmo::TRANSLATE);

										ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);



										Matrix lightMat = Matrix::CreateTranslation(pLight->position.x, pLight->position.y, pLight->position.z);

										/*if (ImGuizmo::Manipulate(&graphics.renderSystem->GetMainCameraGeo()->GetView()._11, &graphics.renderSystem->GetMainCameraGeo()->GetProj()._11, imGuizmoOpPLight, mCurrentGizmoMode, &lightMat._11, NULL, 0))
										{
											Quaternion emptyRot;
											Vector3 emptyScale;
											Vector3 tempTransLight;
											lightMat.Decompose(emptyScale, emptyRot, tempTransLight);
											pLight->position = Vector3(tempTransLight.x, tempTransLight.y, tempTransLight.z);

											graphics.lightSystem->UpdateLight(*plComp, *pLight);
										}*/
									}

									// Bug caused of not all lights being copied. So if ex. one light has been copied from subresource 0, and then a light is modified and copied from subr 1, the first light
									// wont be copied with it.
									// Temp fix is to always use frame index 0, won't work with tripple buffering thought...
									float pos[3]{ pLight->position.x, pLight->position.y, pLight->position.z };
									if (ImGui::InputFloat3("Position", pos))
									{
										pLight->position.x = pos[0];
										pLight->position.y = pos[1];
										pLight->position.z = pos[2];
										graphics.lightSystem->UpdateLight(*plComp, *pLight);
									}

									float col[3]{ pLight->color.x, pLight->color.y, pLight->color.z };
									if (ImGui::ColorEdit3("Color", col))
									{
										pLight->color.x = col[0];
										pLight->color.y = col[1];
										pLight->color.z = col[2];
										graphics.lightSystem->UpdateLight(*plComp, *pLight);
									}

									float intensity = pLight->intensity;
									if (ImGui::InputFloat("Intensity", &intensity, 0.f, FLT_MAX))
									{
										pLight->intensity = intensity;
										graphics.lightSystem->UpdateLight(*plComp, *pLight);
									}

									float range = pLight->range;
									if (ImGui::InputFloat("Range", &range, 0.f, FLT_MAX))
									{
										pLight->range = range;
										graphics.lightSystem->UpdateLight(*plComp, *pLight);
									}

									if (ImGui::Button("Delete Point Light"))
									{
										graphics.scene->RemoveComponentFromEntity<PointLightComponent>(graphics.scene->GetEntity(selectedEntityID));
									}
								}
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

		ImGui::End();
		//---------------------------------------------------------------
	}
}

void EditorUI::ShowSceneManager()
{
	//-----------------------------Scene Manager-----------------------------
	ImGui::Begin("Scene Manager");

	if (graphics.scene == nullptr)
	{
		ImGuizmo::Enable(false);

		if (ImGui::Button("New Scene"))
		{
			/*graphics.scene = new Scene(&graphics.ecs, &graphics.vbCache, &graphics.materialManager, &graphics.descriptorManager,
				&graphics.textureCache, graphics.lightSystem);*/
		}
	}
	else
	{
		const std::string sceneNameText = "Current Scene Name: " + graphics.scene->GetName();
		ImGui::Text(sceneNameText.c_str());
	}

	if (ImGui::Button("Open Scene"))
	{
		std::string sceneNameWithExt = "";
		if (Helper::OpenFileDialogForExtension(".azs", sceneNameWithExt))
		{
			std::string::size_type const p(sceneNameWithExt.find_last_of('.'));
			std::string sceneNameWithoutExt = sceneNameWithExt.substr(0, p);
			
			if (graphics.scene)
			{
				//graphics.scene->ShutDown(graphics.resourceEngine, graphics.frameIndex);
				delete graphics.scene;
			}

			// temp
		/*	graphics.scene = new Scene(&graphics.ecs, &graphics.vbCache, &graphics.materialManager, 
				&graphics.descriptorManager, &graphics.textureCache, graphics.lightSystem);*/
			//graphics.scene->Load(graphics.device, graphics.resourceEngine, graphics.frameIndex, "..\\scenes\\", sceneNameWithoutExt);

			editingDLight = false;
			editingPLight = false;
			//selectedEntityStr = "";
			selectedEntityID = -1;
		}
	}

	if (graphics.scene != nullptr)
	{
		if (ImGui::Button("Close Scene"))
		{
			//graphics.scene->ShutDown(graphics.resourceEngine, graphics.frameIndex);
			delete graphics.scene;
			graphics.scene = nullptr;

			editingDLight = false;
			editingPLight = false;
			//selectedEntityStr = "";
			selectedEntityID = -1;
		}

		static char sceneNameBuffer[40] = "";
		ImGui::InputText("Scene Name", sceneNameBuffer, 40);
		if (ImGui::Button("Save Scene"))
		{
			const std::string sceneName(sceneNameBuffer);
			if (sceneName.size() == 0)
			{
				graphics.scene->Save("..\\scenes\\", graphics.scene->GetName().c_str());
			}
			else
			{
				graphics.scene->Save("..\\scenes\\", sceneName.c_str());
			}

			ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
		}
	}

	ImGui::End();
	//-----------------------------------------------------------------------
}

void EditorUI::ShowLoadedVertexBuffers()
{
	if (ImGui::BeginListBox("Loaded Meshes"))
	{
		for (auto& vb : graphics.vbCache.GetAllResources())
		{
			const std::string name = vb.GetName();
			if (ImGui::Selectable(name.c_str()))
			{
				selectedMeshStr = name;
				selectedMeshID = graphics.vbCache.GetID(name);
			}
		}

		ImGui::EndListBox();
	}

	if (selectedMeshID != -1)
	{
		const std::string t = "Selected Mesh: " + selectedMeshStr;
		ImGui::Text(t.c_str());

		VertexBuffer* selVB = graphics.vbCache.GetResource(selectedMeshID);
		const std::string tx = "Number of Vertices: " + std::to_string(selVB->GetNumVertices());
		ImGui::Text(tx.c_str());
	}

	if (ImGui::Button("Load Mesh"))
	{
		std::string fileName = "";
		if (Helper::OpenFileDialogForExtension(".fbx", fileName))
		{
			int extIndex = fileName.find_last_of(".");
			fileName.assign(fileName.begin(), fileName.begin() + extIndex);
			graphics.vbCache.LoadResource(fileName, "..\\meshes\\");
		}
	}

	if (selectedMeshID != -1)
	{
		if (selectedMeshStr != "demoCube")
		{
			if (ImGui::Button("Delete Mesh"))
			{
				if (graphics.vbCache.Exists(selectedMeshID))
				{
					/*if (graphics.scene)
					{
						for (auto& ent : graphics.scene->entities.GetObjects())
						{
							Mesh* meshComp = graphics.ecs.GetComponentManager().GetComponent<Mesh>(ent);
							if (meshComp && (selectedMeshID == meshComp->GetID()))
							{
								meshComp->SetID(graphics.vbCache.GetID("demoCube"));
							}
						}
					}*/

					graphics.vbCache.RemoveResource(selectedMeshID);
					selectedMeshID = -1;
					selectedMeshStr = "";
				}
			}
		}
	}
}

void EditorUI::ShowLoadedTextures()
{
	if (ImGui::BeginListBox("Loaded Textures"))
	{
		for (auto& texture : graphics.textureCache.GetAllResources())
		{
			const std::string name = texture.GetName();
			if (ImGui::Selectable(name.c_str()))
			{
				selectedTextureStr = name;
				selectedTextureID = graphics.textureCache.GetID(name);
			}
		}

		ImGui::EndListBox();
	}

	if (ImGui::Button("Load Texture"))
	{
		std::string fileName = "";
		if (Helper::OpenFileDialogForExtension(".png", fileName))
		{
			graphics.textureCache.LoadResource(fileName, "..\\textures\\");
		}
	}

	if (selectedTextureID != -1)
	{
		Texture2D* selText = graphics.textureCache.GetResource(selectedTextureID);

		const std::string t = "Selected Texture: " + selectedTextureStr;
		ImGui::Text(t.c_str());

		const std::string tx = "Dimensions: " + std::to_string(selText->GetDimensions().x) + " : " + std::to_string(selText->GetDimensions().y);
		ImGui::Text(tx.c_str());

		if(selText)
			ImGui::Image((ImTextureID)selText->GetHandle().GetGPUHandle().ptr, ImVec2(100, 100));

		if (selectedTextureStr != "defaultDiffuse.png")
		{
			if (ImGui::Button("Delete Texture"))
			{
				for (auto& mat : graphics.materialManager.GetPhongMaterials())
				{
					if (mat.GetInfoPtr().diffuseTextureID == graphics.textureCache.GetResource(selectedTextureID)->GetHandle().GetHeapIndex())
					{
						mat.GetInfoPtr().diffuseTextureID = graphics.textureCache.GetResource("defaultDiffuse.png")->GetHandle().GetHeapIndex();
						mat.Update(graphics.resourceEngine);
					}
				}

				for (auto& mat : graphics.materialManager.GetPBRMaterials())
				{
					if (mat.GetInfoPtr().albedoMapIndex == graphics.textureCache.GetResource(selectedTextureID)->GetHandle().GetHeapIndex())
					{
						mat.GetInfoPtr().albedoMapIndex = graphics.textureCache.GetResource("defaultDiffuse.png")->GetHandle().GetHeapIndex();
						mat.Update(graphics.resourceEngine);
					}
					else if(mat.GetInfoPtr().roughnessMapIndex == graphics.textureCache.GetResource(selectedTextureID)->GetHandle().GetHeapIndex())
					{
						mat.GetInfoPtr().roughnessMapIndex = -1;
						mat.Update(graphics.resourceEngine);
					}
					else if (mat.GetInfoPtr().metallicMapIndex == graphics.textureCache.GetResource(selectedTextureID)->GetHandle().GetHeapIndex())
					{
						mat.GetInfoPtr().metallicMapIndex = -1;
						mat.Update(graphics.resourceEngine);
					}
				}

				graphics.textureCache.RemoveResource(selectedTextureID);
				selectedTextureID = -1;
				selectedTextureStr = "";
			}
		}
	}
}

void EditorUI::ShowGlobalLightingManager()
{
	if (ImGui::Begin("Global Lighting Manager"))
	{

		ImGui::Text("Directional Light");
		static float pitch = 0.f;
		static float yaw = 0.f;
		static float roll = 0.f;
		ImGui::SliderAngle("Pitch", &pitch);
		ImGui::SliderAngle("Yaw", &yaw);

		Matrix view = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
		view = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
		Matrix proj = Matrix::CreateOrthographic(10.f, 10.f, 1.f, 10.f);
		graphics.lightSystem->GetLightManager()->dLightData.VPMatrix = proj * view;

		ImGui::End();
	}
}

void EditorUI::ShowDebugTextures()
{
	if (ImGui::Begin("Debug Textures"))
	{
		if (ImGui::BeginTabBar("Textures"))
		{
			if (ImGui::BeginTabItem("Picking RTV"))
			{
				graphics.pickingSystem->GetPickingRTV()->Transition(graphics.resourceEngine.renderPassList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				ImGui::Image((ImTextureID)graphics.pickingSystem->GetPickingRTV()->GetSrvHandle().GetGPUHandle().ptr, ImVec2(800, 800));
				graphics.pickingSystem->GetPickingRTV()->Transition(graphics.resourceEngine.renderPassList, D3D12_RESOURCE_STATE_RENDER_TARGET);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}

void EditorUI::PBRMatEditing()
{
	HandleListSelection(graphics.materialManager.GetPBRMaterials(), "##0", selectedPBRMaterialStr, selectedPBRMaterialID, 1);
	CreateMaterial<PBRMaterial>("New##0");
	SaveMaterial<PBRMaterial>("Save##0", selectedPBRMaterialID, "DefaultPBRMaterial");
	LoadMaterial<PBRMaterial>("Load##0", selectedPBRMaterialStr, selectedPBRMaterialID, ".azmpbr");
	DeleteMaterial<PBRMaterial>("Delete##0", selectedPBRMaterialStr, selectedPBRMaterialID, "DefaultPBRMaterial");

	PBRMaterial* pbrMat = graphics.materialManager.GetMaterial<PBRMaterial>(selectedPBRMaterialID);
	if (pbrMat != nullptr)
	{
		if (pbrMat->GetName() != "DefaultPBRMaterial")
		{
			ImGui::Text("Material Information");
			PBRMaterialInformation& info = pbrMat->GetInfoPtr();

			// Albedo
			ImGui::Text("Albedo Texture");
			Texture2D* t = graphics.textureCache.GetResource(graphics.textureCache.GetTextureName(info.albedoMapIndex));
			if (t)
				ImGui::Image((ImTextureID)t->GetHandle().GetGPUHandle().ptr, ImVec2(70, 70));

			ImGui::SameLine();
			if (ImGui::Button("Apply Selected Texture##0"))
			{
				t = graphics.textureCache.GetResource(selectedTextureID);
				if (t)
				{
					info.albedoMapIndex = t->GetHandle().GetHeapIndex();
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			// Roughness
			ImGui::Text("Roughness Texture");
			t = graphics.textureCache.GetResource(graphics.textureCache.GetTextureName(info.roughnessMapIndex));
			if (t)
				ImGui::Image((ImTextureID)t->GetHandle().GetGPUHandle().ptr, ImVec2(70, 70));
			else
				ImGui::Text("NULL");

			ImGui::SameLine();
			if (ImGui::Button("Apply Selected Texture##00"))
			{
				t = graphics.textureCache.GetResource(selectedTextureID);
				if (t)
				{
					info.roughnessMapIndex = t->GetHandle().GetHeapIndex();
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			if (info.roughnessMapIndex != -1)
			{
				if (ImGui::Button("Remove Texture##0"))
				{
					info.roughnessMapIndex = -1;
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			if (info.roughnessMapIndex == -1)
			{
				if (ImGui::SliderFloat("Roughness Factor##0", (float*)&info.roughnessFactor, 0.f, 1.f))
				{
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			// Metallic
			ImGui::Text("Metallic Texture");
			t = graphics.textureCache.GetResource(graphics.textureCache.GetTextureName(info.metallicMapIndex));
			if (t)
				ImGui::Image((ImTextureID)t->GetHandle().GetGPUHandle().ptr, ImVec2(70, 70));
			else
				ImGui::Text("NULL");

			ImGui::SameLine();
			if (ImGui::Button("Apply Selected Texture##000"))
			{
				Texture2D* tt = graphics.textureCache.GetResource(selectedTextureID);
				if (tt)
				{
					info.metallicMapIndex = tt->GetHandle().GetHeapIndex();
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			if (info.metallicMapIndex != -1)
			{
				if (ImGui::Button("Remove Texture##00"))
				{
					info.metallicMapIndex = -1;
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			if (info.metallicMapIndex == -1)
			{
				if (ImGui::SliderFloat("Metallic Factor##0", (float*)&info.metallicFactor, 0.f, 1.f))
				{
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			// Normal Map
			ImGui::Text("Normal Map");
			t = graphics.textureCache.GetResource(graphics.textureCache.GetTextureName(info.normalMapIndex));
			if (t)
				ImGui::Image((ImTextureID)t->GetHandle().GetGPUHandle().ptr, ImVec2(70, 70));
			else
				ImGui::Text("NULL");

			ImGui::SameLine();
			if (ImGui::Button("Apply Selected Texture##0000"))
			{
				Texture2D* tt = graphics.textureCache.GetResource(selectedTextureID);
				if (tt)
				{
					info.normalMapIndex = tt->GetHandle().GetHeapIndex();
					pbrMat->Update(graphics.resourceEngine);
				}
			}

			if (info.normalMapIndex != -1)
			{
				if (ImGui::Button("Remove Texture##000"))
				{
					info.normalMapIndex = -1;
					pbrMat->Update(graphics.resourceEngine);
				}
			}
		}
	}
}

void EditorUI::PhongMatEditing()
{
	HandleListSelection(graphics.materialManager.GetPhongMaterials(), "##1", selectedPhongMaterialStr, selectedPhongMaterialID, 1);
	CreateMaterial<PhongMaterial>("New##1");
	SaveMaterial<PhongMaterial>("Save##1", selectedPhongMaterialID, "DefaultPhongMaterial");
	LoadMaterial<PhongMaterial>("Load##1", selectedPhongMaterialStr, selectedPhongMaterialID, ".azm");
	DeleteMaterial<PhongMaterial>("Delete##1", selectedPhongMaterialStr, selectedPhongMaterialID, "DefaultPhongMaterial");
	
	PhongMaterial* pMat = graphics.materialManager.GetMaterial<PhongMaterial>(selectedPhongMaterialID);
	if (pMat != nullptr)
	{
		ImGui::Text("Material Information");
		if (pMat->GetName() != "DefaultPhongMaterial")
		{
			PhongMaterialInformation& info = pMat->GetInfoPtr();
			std::string textureName = "Diffuse Texture Name: ";
			std::string temp = graphics.textureCache.GetTextureName(info.diffuseTextureID);
			textureName += temp;
			ImGui::Text(textureName.c_str());

			const std::string selTexTemp = "Attach Selected Texture: " + selectedTextureStr;
			if (ImGui::Button(selTexTemp.c_str()))
			{
				if (selectedTextureID != -1)
				{
					Texture2D* t = graphics.textureCache.GetResource(selectedTextureID);
					if (t)
					{
						info.diffuseTextureID = t->GetHandle().GetHeapIndex();
						pMat->Update(graphics.resourceEngine);
					}
				}
			}

			if (ImGui::Button("Load Diffuse Texture"))
			{
				std::string textureNameWithExt = "";
				if (Helper::OpenFileDialogForExtension(".png", textureNameWithExt))
				{
					if (!graphics.textureCache.Exists(textureNameWithExt))
						graphics.textureCache.LoadResource(textureNameWithExt, "..\\textures\\");

					Texture2D* t = graphics.textureCache.GetResource(textureNameWithExt);
					if (t)
						info.diffuseTextureID = t->GetHandle().GetHeapIndex();
					else
						info.diffuseTextureID = 0;

					pMat->Update(graphics.resourceEngine);
				}
			}
			ImGui::SameLine();
			Texture2D* t = graphics.textureCache.GetResource(graphics.textureCache.GetTextureName(info.diffuseTextureID));
			if (t)
				ImGui::Image((ImTextureID)t->GetHandle().GetGPUHandle().ptr, ImVec2(70, 70));

			if (ImGui::ColorEdit3("Diffuse Absorbation", (float*)&info.ambientAbsorbation.x))
			{
				pMat->Update(graphics.resourceEngine);

			}

			if (ImGui::ColorEdit3("Specular Absorbation", (float*)&info.specularAbsorbation.x))
			{
				pMat->Update(graphics.resourceEngine);
			}

			if (ImGui::InputFloat("Specular Exponent", (float*)&info.specularShine))
			{
				pMat->Update(graphics.resourceEngine);
			}
		}
	}
}

void EditorUI::DrawMaterialPreview()
{
	if (selectedPhongMaterialID != -1)
	{
		static float rot = 0.f;
		Matrix world = Matrix::CreateRotationY(rot) * Matrix::CreateTranslation(0, 0, 2);
		//tempMatPrev.world = world.Transpose();
		rot += 0.001f;
		tempMatPrev.world = world;

		//graphics.resourceEngine.Update(cbMaterialPrev, (void*)&tempMatPrev, graphics.frameIndex);

		graphics.resourceEngine.renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics.resourceEngine.renderPassList.GetGraphicList()->SetPipelineState(psoMatPrev.GetPipelineState());
		graphics.resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootSignature(rootMatPrev.GetSignature());
		const FLOAT f[] = {0.5f, 0.5f, 0.5f,1.f};
		graphics.resourceEngine.renderPassList.GetGraphicList()->ClearRenderTargetView(matPrevRTV.GetHandle().GetCPUHandle(), f, 0, nullptr);
		graphics.resourceEngine.renderPassList.GetGraphicList()->ClearDepthStencilView(matPrevDSV.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
		graphics.resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(1, &matPrevRTV.GetHandle().GetCPUHandleRef(), false, &matPrevDSV.GetHandle().GetCPUHandleRef());
		graphics.resourceEngine.renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &graphics.vbCache.GetResource("demoCube")->GetView());
		graphics.resourceEngine.renderPassList.GetGraphicList()->RSSetViewports(1, &viewport);

		graphics.resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(0, cbMaterialPrev.GetGPUAddress());
		graphics.resourceEngine.renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, graphics.materialManager.GetMaterial<PhongMaterial>(selectedPhongMaterialID)->GetGPUAddress());

		graphics.resourceEngine.renderPassList.GetGraphicList()->DrawInstanced(graphics.vbCache.GetResource("demoCube")->GetNumVertices(), 1, 0, 0);

		/*D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(matPrevRTV.GetMainResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		graphics.resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);*/
	}
}
