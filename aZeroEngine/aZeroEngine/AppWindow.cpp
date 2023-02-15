#include "AppWindow.h"

void AppWindow::InitSwapChain(ID3D12Device* _device, CommandQueue& _cmdQueue, CommandList& _cmdList, DescriptorHandle _dsvHandle, std::vector<DescriptorHandle> _bbHandles, int _numBackBuffers)
{
	bbFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	swapChain.Init(_device, handle, _cmdQueue, _cmdList, GetClientSize(), _dsvHandle, _bbHandles, _numBackBuffers, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

bool AppWindow::Update()
{
	MSG msg = { 0 };
	bool msgReturn = 1;
	while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

Vector2 AppWindow::GetClientSize()
{
	RECT size;
	GetClientRect(handle, &size);
	return Vector2(size.right - size.left, size.bottom - size.top);
}

Vector2 AppWindow::GetWindowSize()
{
	RECT size;
	GetClientRect(handle, &size);
	return Vector2(size.right - size.left, size.bottom - size.top);
}
