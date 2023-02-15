#pragma once
#include "BaseResource.h"

class BaseResource;

/** @brief Encapsulates a vertex buffer.
*/
class VertexBuffer : public BaseResource
{
private:
	D3D12_VERTEX_BUFFER_VIEW view;
	int numVertices;
	std::string fileName;

	// Inherited via BaseResource
	// Disabled
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, const std::wstring& _mainResourceName) override;
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, bool _trippleBuffered, const std::wstring& _mainResourceName) override;
public:
	VertexBuffer();

	/**Initiates as a static vertex buffer.
	@param _device Device to use when creating the D3D12 resources.
	@param _cmdList CommandList to execute the resource initiation commands on.
	@param _data Data to initiate the buffer with.
	@param _size Size of the data to initiate with.
	@param _stride Size of each vertex within the resource.
	*/
	VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride);

	~VertexBuffer();

	/** Returns a reference to the vertex buffer view.
	@return D3D12_VERTEX_BUFFER_VIEW&
	*/
	D3D12_VERTEX_BUFFER_VIEW& GetView() { return view; }

	/** Get the number of vertices within the buffer.
	@return int
	*/
	int GetNumVertices() { return numVertices; }
	
	/** Set the number of vertices within the buffer.
	@param _numVertices Number of vertices.
	@return void
	*/
	void SetNumVertices(int _numVertices) { numVertices = _numVertices; }

	/**Returns a constant pointer to the name of the mesh data file.
	@return const char*
	*/
	const std::string GetFileName()const { return fileName; }

	/**Sets the internal name of the mesh data file.
	@return void
	*/
	void SetFileName(const std::string& _fileName) { fileName = _fileName; }

	void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName);
};

