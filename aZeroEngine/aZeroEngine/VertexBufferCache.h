#pragma once
#include "VertexBuffer.h"
#include "MappedVector.h"
#include "HelperFunctions.h"

class VertexBufferCache
{
private:
	MappedVector<VertexBuffer*>vertexBuffers;
public:
	  VertexBufferCache() = default;
	  ~VertexBufferCache();

	  void LoadBuffer(ID3D12Device* _device, CommandList* _cmdList, const std::string& _name)
	  {
		  VertexBuffer* temp = new VertexBuffer();
		  Helper::LoadVertexDataFromFile(_device, _cmdList, "../meshes/" + _name, temp);
		  vertexBuffers.Add(_name, temp);
	  }

	  void RemoveBuffer(const std::string& _name)
	  {
		  delete vertexBuffers.Get(_name);
		  vertexBuffers.Remove(_name);
	  }

	  VertexBuffer* GetBuffer(int _ID)
	  {
		  return vertexBuffers.Get(_ID);
	  }

	  VertexBuffer* GetBuffer(const std::string& _name)
	  {
		  return vertexBuffers.Get(_name);
	  }

	  int GetReferenceID(const std::string& _name)
	  {
		  vertexBuffers.Get(_name)->referenceCount++;
		  return vertexBuffers.GetID(_name);
	  }

	  void FreeReferenceID(int _ID)
	  {
		  VertexBuffer* temp = vertexBuffers.Get(_ID);
		  if (temp->referenceCount > 0)
			  temp->referenceCount--;
	  }

	  void ClearUnused()
	  {
		  for (auto& [key, value] : vertexBuffers.GetStringToIndexMap())
		  {
			  if (vertexBuffers.Get(value)->referenceCount == 0)
			  {
				  vertexBuffers.Remove(key);
			  }
		  }
	  }
};