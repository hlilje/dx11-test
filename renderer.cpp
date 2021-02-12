#include "renderer.hpp"


bool Renderer::Create(const Config& config) {
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = config._width;
	swapChainDesc.BufferDesc.Height = config._height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = config._window;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;

	return true;
}

void Renderer::Run() {
	Update();
	Render();
	Present();
}

void Renderer::Update() {
	// TODO
}

void Renderer::Render() {
	// TODO
}

void Renderer::Present() {
	// TODO
}
