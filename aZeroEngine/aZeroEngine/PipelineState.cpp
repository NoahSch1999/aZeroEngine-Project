#include "PipelineState.h"
#include "HelperFunctions.h"

// Only works for backbuffer as render target...
void PipelineState::Init(ID3D12Device* _device, RootSignature* _rootSignature, const InputLayout& _inputLayout, const RasterState& _rasterState, int _numRenderTargets, 
	DXGI_FORMAT _rtvFormat, DXGI_FORMAT _dsvFormat, const std::wstring& _vsPath, const std::wstring& _psPath, 
	const std::wstring& _dsPath, const std::wstring& _hsPath, const std::wstring& _gsPath, bool _test, bool _enableBlending,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE _primitiveType)
{

	// use constructor / init list...
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	Microsoft::WRL::ComPtr<ID3DBlob> vShader = Helper::LoadBlobFromFile(_vsPath);
	Microsoft::WRL::ComPtr<ID3DBlob> pShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> dShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> hShader = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> gShader = nullptr;

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


	desc.InputLayout = { &_inputLayout.descs[0], 4 };
	desc.pRootSignature = _rootSignature->GetSignature();
	desc.VS = { reinterpret_cast<BYTE*>(vShader->GetBufferPointer()), vShader->GetBufferSize() };
	desc.RasterizerState = _rasterState.GetDesc();

	//D3D12_RENDER_TARGET_BLEND_DESC rtvBlend;
	//ZeroMemory(&rtvBlend, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
	//rtvBlend.BlendEnable = _enableBlending;
	//rtvBlend.LogicOpEnable = false;

	//rtvBlend.SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
	//rtvBlend.DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
	//rtvBlend.BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	//rtvBlend.SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
	//rtvBlend.DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
	//rtvBlend.BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	//rtvBlend.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_SET;
	//rtvBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//D3D12_BLEND_DESC blendDesc;
	//ZeroMemory(&blendDesc, sizeof(D3D12_BLEND_DESC));
	//blendDesc.AlphaToCoverageEnable = _enableBlending;
	//blendDesc.IndependentBlendEnable = false;
	//blendDesc.RenderTarget[0] = rtvBlend;

	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = _primitiveType;
	desc.NumRenderTargets = _numRenderTargets;
	desc.RTVFormats[0] = _rtvFormat;

	if (_test)
	{
		desc.NumRenderTargets = 2;
		desc.RTVFormats[0] = _rtvFormat;
		desc.RTVFormats[1] = DXGI_FORMAT_R32_SINT;
	}

	desc.DSVFormat = _dsvFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	HRESULT hr = _device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipelineState.GetAddressOf()));
	if (FAILED(hr))
		throw;

#ifdef _DEBUG
	const std::wstring str(_vsPath + _psPath + _dsPath + _hsPath + _gsPath);
	pipelineState->SetName(str.c_str());
#endif // _DEBUG

}
