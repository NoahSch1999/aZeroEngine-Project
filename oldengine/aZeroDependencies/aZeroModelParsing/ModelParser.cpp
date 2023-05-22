#include "aZeroModelParsing/ModelParser.h"
#include <filesystem>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

namespace aZeroFiles
{
	std::unique_ptr<VectorLoad> ReadVector(std::ifstream& fileStream)
	{
		int numElements = 0;
		fileStream.read((char*)&numElements, sizeof(int));

		int sizePerElement = 0;
		fileStream.read((char*)&sizePerElement, sizeof(int));

		std::unique_ptr<VectorLoad> load = std::make_unique<VectorLoad>();
		load->m_numElements = numElements;
		load->m_sizePerElement = sizePerElement;
		load->m_data = new char[numElements * sizePerElement];
		fileStream.read((char*)load->m_data, numElements * sizePerElement);

		return load;
	}

	void GenerateAZModel(const ModelGenerateSettings& settings)
	{
		Assimp::Importer importer;
		const std::string finalPath = settings.m_sourceDirectory + "/" + settings.m_sourceFileName + settings.m_fileExtension;
		const aiScene* scene = importer.ReadFile(finalPath,
			aiProcess_SortByPType | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		int numSubmeshes = scene->mNumMeshes;

		std::vector<PerSubmeshInfo> submeshesInfo;
		submeshesInfo.reserve(numSubmeshes);

		std::vector<Vertex> vertexData;
		std::vector<int> indexData;

		for (int i = 0; i < numSubmeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];
			vertexData.reserve(vertexData.size() + mesh->mNumVertices);

			PerSubmeshInfo perSubInfo;
			perSubInfo.m_startIndex = vertexData.size();
			perSubInfo.m_stopIndex = vertexData.size() + mesh->mNumVertices - 1;
			submeshesInfo.emplace_back(perSubInfo);

			for (int j = 0; j < mesh->mNumVertices; j++)
			{
				Vertex vertex;
				vertex.m_position[0] = mesh->mVertices[j].x;
				vertex.m_position[1] = mesh->mVertices[j].y;
				vertex.m_position[2] = mesh->mVertices[j].z;

				vertex.m_uv[0] = mesh->mTextureCoords[0][j].x;
				vertex.m_uv[1] = mesh->mTextureCoords[0][j].y;

				vertex.m_normal[0] = mesh->mNormals[j].x;
				vertex.m_normal[1] = mesh->mNormals[j].y;
				vertex.m_normal[2] = mesh->mNormals[j].z;

				vertex.m_tangent[0] = mesh->mTangents[j].x;
				vertex.m_tangent[1] = mesh->mTangents[j].y;
				vertex.m_tangent[2] = mesh->mTangents[j].z;

				vertexData.emplace_back(vertex);
			}

			for (int j = 0; j < mesh->mNumFaces; j++)
			{
				indexData.reserve(indexData.capacity() + mesh->mFaces->mNumIndices);
				for (int h = 0; h < mesh->mFaces[j].mNumIndices; h++)
				{
					indexData.emplace_back(mesh->mFaces[j].mIndices[h]);
				}
			}
		}

		std::ofstream fileStream(settings.m_targetDirectory + "/" + settings.m_targetFileName + ".azModel", 
			std::ios_base::trunc | std::ios::out | std::ios::binary);

		if (fileStream.is_open())
		{
			WriteVector(fileStream, submeshesInfo);

			WriteVector(fileStream, vertexData);

			WriteVector(fileStream, indexData);

			fileStream.close();
		}

	}

	std::optional<std::unique_ptr<LoadedModelContainer>> LoadAZModel(const std::string& sourceDirectory, const std::string& sourceFileName)
	{
		std::ifstream fileStream(sourceDirectory + "/" + sourceFileName + ".azModel", std::ios::in | std::ios::binary);

		if (fileStream.is_open())
		{
			std::unique_ptr<LoadedModelContainer> container = std::make_unique<LoadedModelContainer>();

			std::unique_ptr<VectorLoad> load = ReadVector(fileStream);
			int numSubmeshes = load->m_numElements;
			for (int i = 0; i < numSubmeshes; i++)
			{
				int offset = sizeof(PerSubmeshInfo) * i;
				PerSubmeshInfo info;
				memcpy(&info, (char*)load->m_data + offset, sizeof(PerSubmeshInfo));
				container->m_subMeshInfo.emplace_back(info);
			}

			load = ReadVector(fileStream);
			container->m_NumVertices = load->m_numElements;

			container->m_rawVertexData = new char[load->m_numElements * load->m_sizePerElement];
			memcpy(container->m_rawVertexData, load->m_data, load->m_numElements * load->m_sizePerElement);

			load = ReadVector(fileStream);
			container->m_NumIndices = load->m_numElements;

			container->m_rawIndexData = new char[load->m_numElements * load->m_sizePerElement];
			memcpy(container->m_rawIndexData, load->m_data, load->m_numElements * load->m_sizePerElement);

			return container;
		}

		return {};
	}
}