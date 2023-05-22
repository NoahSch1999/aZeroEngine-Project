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
		int m_NumVertices;
		void* m_rawVertexData = nullptr;
		int m_NumIndices;
		void* m_rawIndexData = nullptr;
		std::vector<PerSubmeshInfo> m_subMeshInfo;

		~LoadedModelContainer()
		{
			delete m_rawVertexData;
			m_rawVertexData = nullptr;
			delete m_rawIndexData;
			m_rawIndexData = nullptr;
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