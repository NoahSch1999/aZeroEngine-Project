#pragma once
#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <fstream>

namespace aZeroFiles
{
	struct Vertex
	{
		float m_position[3];
		float m_uv[2];
		float m_normal[3];
		float m_tangent[3];
	};

	struct PerSubmeshInfo
	{
		short m_startIndex;
		short m_stopIndex;
		float m_maxDistance;
	};

	struct ModelGenerateSettings
	{
		std::string m_sourceDirectory;
		std::string m_sourceFileName;
		std::string m_targetDirectory;
		std::string m_targetFileName;
		std::string m_fileExtension;
	};

	struct LoadedModelContainer
	{
		int m_numVertices;
		void* m_rawVertexData = nullptr;
		int m_numIndices;
		void* m_rawIndexData = nullptr;
		std::vector<PerSubmeshInfo> m_subMeshInfo;

		LoadedModelContainer() = default;

		LoadedModelContainer(const LoadedModelContainer&) = delete;
		LoadedModelContainer& operator=(const LoadedModelContainer&) = delete;

		LoadedModelContainer(LoadedModelContainer&& other) noexcept
		{
			m_numVertices = other.m_numVertices;
			m_numIndices = other.m_numIndices;
			m_rawVertexData = other.m_rawVertexData;
			m_rawIndexData = other.m_rawIndexData;
			m_subMeshInfo = other.m_subMeshInfo;

			other.m_rawVertexData = nullptr;
			other.m_rawIndexData = nullptr;
			
		}

		LoadedModelContainer& operator=(LoadedModelContainer&& other) noexcept
		{
			if (this != &other)
			{
				m_numVertices = other.m_numVertices;
				m_numIndices = other.m_numIndices;
				m_rawVertexData = other.m_rawVertexData;
				m_rawIndexData = other.m_rawIndexData;
				m_subMeshInfo = other.m_subMeshInfo;

				other.m_rawVertexData = nullptr;
				other.m_rawIndexData = nullptr;
			}
			return *this;
		}

		~LoadedModelContainer()
		{
			if (m_rawVertexData)
			{
				delete m_rawVertexData;
				delete m_rawIndexData;
			}
		}
	};

	struct VectorLoad
	{
		int m_numElements = 0;
		int m_sizePerElement = 0;
		void* m_data = nullptr;

		~VectorLoad()
		{
			delete m_data;
			m_data = nullptr;
		}
	};

	template<typename T>
	void WriteVector(std::ofstream& fileStream, const std::vector<T>& vector)
	{
		int numElements = vector.size();
		fileStream.write((char*)&numElements, sizeof(int));

		int sizePerElement = sizeof(T);
		fileStream.write((char*)&sizePerElement, sizeof(int));

		fileStream.write((char*)vector.data(), sizePerElement * numElements);
	}

	std::unique_ptr<VectorLoad> ReadVector(std::ifstream& fileStream);

	void GenerateAZModel(const ModelGenerateSettings& settings);

	std::optional<std::unique_ptr<LoadedModelContainer>> LoadAZModel(const std::string& sourceDirectory, const std::string& sourceFileName);
}