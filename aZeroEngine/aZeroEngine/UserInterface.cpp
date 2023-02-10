#include "UserInterface.h"


/** @brief Opens the file dialog and returns true if the user selects a file that ends with the input .xxx extension. Otherwise it returns false.
* If it returns true, it will also copy the filename.xxx to the input _storeFileStr argument.
@return TRUE: File with extension selected, FALSE: File with extension not selected
*/
bool OpenFileDialogForExtension(const std::string& _extension, std::string& _storeFileStr)
{
	CHAR ogPath[MAX_PATH];
	DWORD dw = GetCurrentDirectoryA(MAX_PATH, ogPath);

	OPENFILENAMEA openDialog = { 0 };
	CHAR filePath[260] = { 0 };
	ZeroMemory(&openDialog, sizeof(openDialog));
	openDialog.lStructSize = sizeof(openDialog);
	openDialog.hwndOwner = NULL;
	openDialog.lpstrInitialDir = LPCSTR(ogPath);
	openDialog.lpstrFile = filePath;
	openDialog.nMaxFile = MAX_PATH;
	openDialog.lpstrTitle = "Select a file";
	openDialog.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&openDialog);

	const std::string selectedFilePath(filePath);

	if (selectedFilePath.ends_with(_extension))
	{
		std::string fileameWithExt = selectedFilePath.substr(selectedFilePath.find_last_of("/\\") + 1);
		_storeFileStr = fileameWithExt;
		return true;
	}
	return false;
}

void EditorUI::Update()
{
	// Available entities && entity selection
	if (graphics.scene != nullptr)
	{
		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

		if (ImGui::IsKeyPressed(ImGuiKey_T))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_S)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;


		//------------------------Entity Editor------------------------
		ImGui::Begin("Entity Editor");
		if (graphics.scene->entities.GetObjects().size() > 0)
		{
			ImGui::ListBoxHeader("Entities");
			for (auto [name, index] : graphics.scene->entities.GetStringToIndexMap())
			{
				if (ImGui::Selectable(std::string(name).c_str()))
				{
					selectedEntityStr = name;
					selectedEntityID = graphics.scene->entities.GetID(selectedEntityStr);
					break;
				}
			}
			ImGui::ListBoxFooter();

			ImGui::Text("Selected Entity: ");
			ImGui::SameLine();
			ImGui::Text(selectedEntityStr.c_str());
		}
		//-----------------------------------------------------------------

		//------------------------Create New Entity------------------------
		static int counter = 0;
		if (ImGui::Button("Create Entity"))
		{
			graphics.WaitForGPU();
			Entity& ent = graphics.scene->CreateEntity(graphics.device, &graphics.directCmdList);
			selectedEntityStr = graphics.scene->GetEntityName(ent);
			selectedEntityID = graphics.scene->entities.GetID(selectedEntityStr);
			counter++;
		}
		//-----------------------------------------------------------------------

		if (selectedEntityID != -1)
		{
			// Hide / Show Entity
			// NOT DONE. THERE IS A BUG WHICH HAPPENS IF THE ENTITY IS HIDDEN AND IT GETS BOUND TO THE RENDERER. THIS BUG WILL DISABLE THE ABILITY TO HIDE/SHOW THE ENTITY.
			bool& isHidden = graphics.scene->entities.Get(selectedEntityID).disabled;
			if (ImGui::Checkbox("Hide Entity", &isHidden))
			{
				if (isHidden)
				{
					graphics.renderSystem->UnBind(graphics.scene->entities.Get(selectedEntityID));
				}
				else
				{
					graphics.renderSystem->Bind(graphics.scene->entities.Get(selectedEntityID));
				}
			}

			//------------------------Components For Selected Entity------------------------
			const char* comps[] = { "Transform", "Mesh", "Material" };
			if (ImGui::TreeNode("Components"))
			{
				graphics.WaitForGPU();
				for (int i = 0; i < 3; i++)
				{
					if (ImGui::TreeNode((void*)(intptr_t)i, comps[i]))
					{
						switch (i)
						{
						case 0:	// Transform Component
						{
							Transform* tf = graphics.scene->GetComponentForEntity<Transform>(graphics.scene->entities.Get(selectedEntityID));
							Vector3 translation;
							Quaternion rotQuat;
							Vector3 rotRad;
							Vector3 scale;
							tf->worldMatrix.Decompose(scale, rotQuat, translation);

							rotRad = rotQuat.ToEuler();

							Vector3 rotDeg;
							rotDeg.x = rotRad.x * (180 / 3.14f);
							rotDeg.y = rotRad.y * (180 / 3.14f);
							rotDeg.z = rotRad.z * (180 / 3.14f);

							float pos[3]{ translation.x, translation.y, translation.z };
							if (ImGui::InputFloat3("Translation", pos))
							{
								tf->worldMatrix = (Matrix::CreateRotationX(rotRad.x) * Matrix::CreateRotationY(rotRad.y) * Matrix::CreateRotationZ(rotRad.z)) * Matrix::CreateScale(scale) * Matrix::CreateTranslation(pos[0], pos[1], pos[2]);
								tf->Update(&graphics.directCmdList, graphics.frameIndex);
							}

							float rot[3]{ rotDeg.x, rotDeg.y, rotDeg.z };
							if (ImGui::InputFloat3("Rotation", rot))
							{
								rotRad.x = rot[0] / (180 / 3.14f);
								rotRad.y = rot[1] / (180 / 3.14f);
								rotRad.z = rot[2] / (180 / 3.14f);
								tf->worldMatrix = (Matrix::CreateRotationX(rotRad.x) * Matrix::CreateRotationY(rotRad.y) * Matrix::CreateRotationZ(rotRad.z)) * Matrix::CreateScale(scale) * Matrix::CreateTranslation(translation);
								tf->Update(&graphics.directCmdList, graphics.frameIndex);
							}

							float sca[3]{ scale.x, scale.y, scale.z };
							if (ImGui::InputFloat3("Scale", sca))
							{
								tf->worldMatrix = (Matrix::CreateRotationX(rotRad.x) * Matrix::CreateRotationY(rotRad.y) * Matrix::CreateRotationZ(rotRad.z)) * Matrix::CreateScale(sca[0], sca[1], sca[2]) * Matrix::CreateTranslation(translation);
								tf->Update(&graphics.directCmdList, graphics.frameIndex);
							}

							break;
						}
						case 1: // Mesh Component
						{
							Mesh* meshComp = graphics.scene->GetComponentForEntity<Mesh>(graphics.scene->entities.Get(selectedEntityID));

							// Should remove the old deprecated Windows functions and replace with the IFileDialog API...
							if (ImGui::Button("Load Vertex Buffer"))
							{
								std::string fbxNameWithExt = "";

								if (OpenFileDialogForExtension(".fbx", fbxNameWithExt))
								{
									std::string::size_type const p(fbxNameWithExt.find_last_of('.'));
									std::string fileNameWithoutExt = fbxNameWithExt.substr(0, p);

									graphics.vbCache.LoadResource(graphics.device, &graphics.directCmdList, fileNameWithoutExt);
									if (meshComp == nullptr)
									{
										Mesh tempMesh;
										tempMesh.vbIndex = graphics.vbCache.GetBufferIndex(fileNameWithoutExt);
										graphics.scene->AddComponentToEntity<Mesh>(graphics.scene->entities.Get(selectedEntityID), tempMesh);
										graphics.renderSystem->Bind(graphics.scene->entities.Get(selectedEntityID));
									}
									else
									{
										meshComp->vbIndex = graphics.vbCache.GetBufferIndex(fileNameWithoutExt);
									}
								}
							}

							if (meshComp != nullptr)
							{
								std::string vbName = "Current Vertex Buffer: " + graphics.vbCache.GetBuffer(meshComp->vbIndex)->GetFileName();
								ImGui::Text(vbName.c_str());
							}
							else
							{
								ImGui::Text("Current Vertex Buffer: NULL");
							}

							break;
						}
						case 2: // Material Component
						{
							MaterialComponent* matComp = graphics.scene->GetComponentForEntity<MaterialComponent>(graphics.scene->entities.Get(selectedEntityID));

							// Should remove the old deprecated Windows functions and replace with the IFileDialog API...
							if (ImGui::Button("Load Phong Material"))
							{
								std::string matNameWithExt = "";
								if (OpenFileDialogForExtension(".azm", matNameWithExt))
								{
									std::string::size_type const p(matNameWithExt.find_last_of('.'));
									std::string matNameWithoutExt = matNameWithExt.substr(0, p);

									graphics.materialManager.CreateMaterial<PhongMaterial>(graphics.device, graphics.resourceManager, graphics.directCmdList, graphics.textureCache, "..\\materials\\", matNameWithoutExt);

									if (matComp == nullptr)
									{
										MaterialComponent tempMat;
										tempMat.materialID = graphics.materialManager.GetReferenceID<PhongMaterial>(matNameWithoutExt);
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->entities.Get(selectedEntityID), tempMat);
										graphics.renderSystem->Bind(graphics.scene->entities.Get(selectedEntityID));
									}
									else
									{
										matComp->materialID = graphics.materialManager.GetReferenceID<PhongMaterial>(matNameWithoutExt);
									}
								}
							}

							const std::string btnName = "Attach Selected Material: " + selectedMaterialStr;

							if (ImGui::Button(btnName.c_str()))
							{
								if (selectedMaterialStr != "")
								{
									if (matComp == nullptr)
									{
										MaterialComponent tempMat;
										tempMat.materialID = graphics.materialManager.GetReferenceID<PhongMaterial>(selectedMaterialStr);
										graphics.scene->AddComponentToEntity<MaterialComponent>(graphics.scene->entities.Get(selectedEntityID), tempMat);
										graphics.renderSystem->Bind(graphics.scene->entities.Get(selectedEntityID));
									}
									else
									{
										matComp->materialID = graphics.materialManager.GetReferenceID<PhongMaterial>(selectedMaterialStr);
									}
								}

							}

							if (matComp != nullptr)
							{
								// Info about what type of material...

								//
								PhongMaterial* pMat = graphics.materialManager.GetMaterial<PhongMaterial>(matComp->materialID);
								if (pMat != nullptr)
								{
									std::string matName = "Current Material: " + pMat->GetName();
									ImGui::Text(matName.c_str());
								
									PhongMaterialInformation& info = pMat->GetInfoPtr();
									const std::string textureName = "Diffuse Texture Name: " + graphics.textureCache.GetTextureName(info.diffuseTextureID);
									ImGui::Text(textureName.c_str());

									if (ImGui::Button("Set Diffuse Texture"))
									{
										std::string textureNameWithExt = "";
										if (OpenFileDialogForExtension(".png", textureNameWithExt))
										{
											if (!graphics.textureCache.Exists(textureNameWithExt))
												graphics.textureCache.LoadResource(graphics.device, graphics.resourceManager.GetTexture2DDescriptor(), &graphics.directCmdList, textureNameWithExt);

											info.diffuseTextureID = graphics.textureCache.GetResource(textureNameWithExt).GetHandle().GetHeapIndex();
											pMat->Update(&graphics.directCmdList, graphics.frameIndex);
										}
									}

									float ambientSpecs[3]{ info.ambientAbsorbation.x, info.ambientAbsorbation.y, info.ambientAbsorbation.z };
									if (ImGui::InputFloat3("Ambient Absorbation", ambientSpecs))
									{
										info.ambientAbsorbation.x = ambientSpecs[0];
										info.ambientAbsorbation.y = ambientSpecs[1];
										info.ambientAbsorbation.z = ambientSpecs[2];
										pMat->Update(&graphics.directCmdList, graphics.frameIndex);
									}

									float specularSpecs[3]{ info.specularAbsorbation.x, info.specularAbsorbation.y, info.specularAbsorbation.z };
									if (ImGui::InputFloat3("Specular Absorbation", specularSpecs))
									{
										info.specularAbsorbation.x = specularSpecs[0];
										info.specularAbsorbation.y = specularSpecs[1];
										info.specularAbsorbation.z = specularSpecs[2];
										pMat->Update(&graphics.directCmdList, graphics.frameIndex);
									}

									if (ImGui::InputFloat("Specular Exponent", &info.specularShine))
									{
										pMat->Update(&graphics.directCmdList, graphics.frameIndex);
									}
								}
							}
							else
							{
								ImGui::Text("Current Material: NULL");
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


		//------------------------Material Editor------------------------
		ImGui::Begin("Material Editor");

		ImGui::ListBoxHeader("Materials");
		for (PhongMaterial& mat : graphics.materialManager.GetPhongMaterials())
		{
			if (ImGui::Selectable(mat.GetName().c_str()))
			{
				selectedMaterialStr = mat.GetName();
				selectedMaterialID = graphics.materialManager.GetReferenceID<PhongMaterial>(mat.GetName());
				break;
			}
		}
		ImGui::ListBoxFooter();

		const std::string selMat = "Selected Material: " + selectedMaterialStr;
		ImGui::Text(selMat.c_str());

		static char matNameBuffer[64] = "";
		ImGui::InputText("Material Name (MAX 64 CHARACTERS)", matNameBuffer, 64);

		if (ImGui::Button("Create Phong Material") && matNameBuffer[0] != '\0')
		{
			graphics.WaitForGPU();
			const std::string newMatTempName(matNameBuffer);
			if (!graphics.materialManager.Exists(newMatTempName))
			{
				graphics.materialManager.CreateMaterial<PhongMaterial>(graphics.device, graphics.directCmdList, graphics.textureCache, newMatTempName);
				PhongMaterial* tempNew = graphics.materialManager.GetMaterial<PhongMaterial>(newMatTempName);
				tempNew->GetInfoPtr().diffuseTextureID = graphics.textureCache.GetResource("defaultDiffuse.png").GetHandle().GetHeapIndex();
				tempNew->Update(&graphics.directCmdList, graphics.frameIndex);
			}
		}

		ImGui::End();
		//-----------------------------------------------------------------------
	}

	//-----------------------------Scene Manager-----------------------------
	ImGui::Begin("Scene Manager");

	if (graphics.scene == nullptr)
	{
		ImGui::Text("Current Scene Name: NULL");
	}
	else
	{
		const std::string sceneNameText = "Current Scene Name: " + graphics.scene->GetName();
		ImGui::Text(sceneNameText.c_str());
	}

	if (ImGui::Button("Load Scene"))
	{
		graphics.WaitForGPU();
		std::string sceneNameWithExt = "";
		if (OpenFileDialogForExtension(".azs", sceneNameWithExt))
		{
			std::string::size_type const p(sceneNameWithExt.find_last_of('.'));
			std::string sceneNameWithoutExt = sceneNameWithExt.substr(0, p);

			if (graphics.scene != nullptr)
			{
				for (const auto& ent : graphics.scene->entities.GetObjects())
				{
					graphics.renderSystem->UnBind(ent);
				}

				delete graphics.scene;
			}
			graphics.scene = new Scene(graphics.ecs, graphics.vbCache, graphics.materialManager, graphics.resourceManager, graphics.textureCache);
			graphics.scene->Load(graphics.device, graphics.directCmdList, graphics.frameIndex, "..\\scenes\\", sceneNameWithoutExt);
			for (const auto& ent : graphics.scene->entities.GetObjects())
			{
				graphics.renderSystem->Bind(ent);
			}

			selectedEntityStr = "";
			selectedEntityID = -1;
		}
	}

	if (ImGui::Button("Close Scene"))
	{
		graphics.WaitForGPU();

		if (graphics.scene != nullptr)
		{
			for (const auto& ent : graphics.scene->entities.GetObjects())
			{
				graphics.renderSystem->UnBind(ent);
			}
			delete graphics.scene;
			graphics.scene = nullptr;
		}

		selectedEntityStr = "";
		selectedEntityID = -1;
	}

	static char sceneNameBuffer[40] = "";
	ImGui::InputText("Scene Name", sceneNameBuffer, 40);
	if (ImGui::Button("Save Scene"))
	{
		const std::string sceneName(sceneNameBuffer);
		if (sceneName.size() == 0)
		{
			graphics.scene->Save("..\\scenes\\", graphics.scene->GetName().c_str(), false);
		}
		else
		{
			graphics.scene->Save("..\\scenes\\", sceneName.c_str(), false);
		}

		ZeroMemory(sceneNameBuffer, sizeof(sceneNameBuffer));
	}

	ImGui::End();
	//-----------------------------------------------------------------------

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
		ImGui::Text("Camera Transform");
		std::string p = "Position: ";
		Vector3 po = graphics.renderSystem->camera.position;
		p += std::to_string(po.x) + " " + std::to_string(po.y) + " " + std::to_string(po.z);
		ImGui::Text(p.c_str());

	}

	ImGui::Text("Average MS and FPS: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
	//-----------------------------------------------------------------------

	ImGui::ShowDemoWindow();

	lastSelectedEntityStr = selectedEntityStr;
	lastSelectedEntityID = selectedEntityID;
}
