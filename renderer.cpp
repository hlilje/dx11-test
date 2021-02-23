#include "renderer.hpp"

#include <d3dcompiler.h>


namespace {
	struct Float3 {
		float _x = 0.0f;
		float _y = 0.0f;
		float _z = 0.0f;
	};

	struct Vertex {
		Float3 _Position;
		Float3 _Color;
	};
}


bool Renderer::Create(const Config& config) {

	if (!CreateContext(config))
		return false;

	if (!CreateResources())
		return false;

	if (!CreateShaders())
		return false;

	return true;
}

void Renderer::Run() {
	Update();
	Render();
	Present();
}

bool Renderer::CreateContext(const Config& config) {
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

	const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1};
	D3D_FEATURE_LEVEL supportedFeatureLevels;
	HRESULT deviceSwapChainResult = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		featureLevels,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&_swapChain,
		&_device,
		&supportedFeatureLevels,
		&_deviceContext
	);

	return !FAILED(deviceSwapChainResult);
}

bool Renderer::CreateResources() {
	constexpr Vertex vertices[] = {
		{0.0f,  0.5f,  0.5f},
		{0.0f,  0.0f,  0.5f},
		{0.5f,  -0.5f, 0.5f},
		{0.5f,  0.0f,  0.0f},
		{-0.5f, -0.5f, 0.5f},
		{0.0f,  0.5f,  0.0f},
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	vertexSubresource.pSysMem = vertices;
	vertexSubresource.SysMemPitch = 0;
	vertexSubresource.SysMemSlicePitch = 0;

	if (FAILED(_device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &_vertexBuffer)))
		return false;

	unsigned int indices[] = {0, 1, 2};

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexSubresource;
	indexSubresource.pSysMem = indices;
	indexSubresource.SysMemPitch = 0;
	indexSubresource.SysMemSlicePitch = 0;

	if (FAILED(_device->CreateBuffer(&indexBufferDesc, &indexSubresource, &_indexBuffer)))
		return false;

	return true;
}

bool Renderer::CreateShaders() {
	if (!CompileShader(L"vertex_shader.hlsl", "main", "vs_5_0", &_vertexShaderBlob))
		return false;

	if (FAILED(_device->CreateVertexShader(
		_vertexShaderBlob->GetBufferPointer(),
		_vertexShaderBlob->GetBufferSize(),
		nullptr,
		&_vertexShader) ) )
		return false;

	if (!CompileShader(L"pixel_shader.hlsl", "main", "ps_5_0", &_pixelShaderBlob))
		return false;

	if (FAILED(_device->CreatePixelShader(
		_pixelShaderBlob->GetBufferPointer(),
		_pixelShaderBlob->GetBufferSize(),
		nullptr,
		&_pixelShader) ) )
		return false;

	constexpr D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED( _device->CreateInputLayout(
		inputLayoutDesc,
		ARRAYSIZE(inputLayoutDesc),
		_vertexShaderBlob->GetBufferPointer(),
		_vertexShaderBlob->GetBufferSize(),
		&_inputLayout )))
		return false;

	return true;
}

bool Renderer::CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob) {
	const D3D_SHADER_MACRO defines[1] = {};
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT result = D3DCompileFromFile(
		srcFile,
		defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		profile,
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&shaderBlob,
		&errorBlob);

	if(FAILED(result)) {
		if(errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if(shaderBlob)
			shaderBlob->Release();

		return false;
	}

	*blob = shaderBlob;

	return true;
}

void Renderer::Update() {
	ID3D11Buffer* vertexBuffers = {_vertexBuffer.Get()};
	constexpr UINT strides[] = {0};
	constexpr UINT offsets[] = {0};
	_deviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, strides, offsets);
	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->IASetInputLayout(_inputLayout.Get());

	_deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	_deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
}

void Renderer::Render() {
	_deviceContext->Draw(6, 0);
}

void Renderer::Present() {
	_swapChain->Present(1, 0);
}
