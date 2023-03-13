#include "PipelineState.h"
#include "HelperFunctions.h"

PipelineState::PipelineState()
{
}

PipelineState::~PipelineState()
{
	pipelineState->Release();
}

// Only works for backbuffer as render target...
void PipelineState::Init(ID3D12Device* _device, RootSignature* _rootSignature, const InputLayout& _inputLayout, const RasterState& _rasterState, int _numRenderTargets, 
	DXGI_FORMAT _rtvFormat, DXGI_FORMAT _dsvFormat, const std::wstring& _vsPath, const std::wstring& _psPath, 
	const std::wstring& _dsPath, const std::wstring& _hsPath, const std::wstring& _gsPath,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE _primitiveType)
{

	// use constructor / init list...
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	ID3DBlob* vShader = Helper::LoadBlobFromFile(_vsPath);
	ID3DBlob* pShader = nullptr;
	ID3DBlob* dShader = nullptr;
	ID3DBlob* hShader = nullptr;
	ID3DBlob* gShader = nullptr;

	if (!_psPath.empty())
	{
		pShader = Helper::LoadBlobFromFile(_psPath);
		desc.PS = { reinterpret_cast<BYTE*>(pShader->GetBufferPointer()), pShader->GetBufferSize() };
	}
	if (!_dsPath.empty())
	{
		dShader = Helper::LoadBlobFromFile(_dsPath);
		desc.DS = { reinterpret_cast<BYTE*>(dShader->GetBufferPointer()), dShader->GetBufferSize() };
	}
	if (!_hsPath.empty())
	{
		hShader = Helper::LoadBlobFromFile(_hsPath);
		desc.HS = { reinterpret_cast<BYTE*>(hShader->GetBufferPointer()), hShader->GetBufferSize() };
	}
	if (!_gsPath.empty())
	{
		gShader = Helper::LoadBlobFromFile(_gsPath);
		desc.GS = { reinterpret_cast<BYTE*>(gShader->GetBufferPointer()), gShader->GetBufferSize() };
	}


	desc.InputLayout = { &_inputLayout.descs[0], 3 };
	desc.pRootSignature = _rootSignature->GetSignature();
	desc.VS = { reinterpret_cast<BYTE*>(vShader->GetBufferPointer()), vShader->GetBufferSize() };
	desc.RasterizerState = _rasterState.GetDesc();

	//D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	//transparencyBlendDesc.BlendEnable = true;
	//transparencyBlendDesc.LogicOpEnable = false;
	//transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	//transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	//transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	//transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	//transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	//transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	//transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//D3D12_BLEND_DESC bDesc = {};
	//bDesc.AlphaToCoverageEnable = true;
	//bDesc.IndependentBlendEnable = true;
	//bDesc.RenderTarget[0] = transparencyBlendDesc;

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = _primitiveType;
	desc.NumRenderTargets = _numRenderTargets;
	desc.RTVFormats[0] = _rtvFormat;
	desc.DSVFormat = _dsvFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	HRESULT hr = _device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr))
		throw;

	// Solve this by using a comptr
	vShader->Release();
	//pShader->Release();
	//dShader->Release();
	//hShader->Release();
	//gShader->Release();
}
