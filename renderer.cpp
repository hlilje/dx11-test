#include "renderer.hpp"

#include <d3dcompiler.h>

#include <cmath>


namespace {
	using Float3 = DirectX::XMFLOAT3;
	using Vector = DirectX::XMVECTOR;
	using Matrix = DirectX::XMMATRIX;

	struct Vertex {
		Float3 _position;
		Float3 _color;
	};
}


bool Renderer::Create(const Config& config) {
	if (!CreateContext(config)) {
		return false;
	}

	if (!CreateRenderTarget()) {
		return false;
	}

	if (!CreateResources()) {
		return false;
	}

	if (!CreateShaders()) {
		return false;
	}

	return true;
}

void Renderer::Run() {
	Update();
	Render();
	Present();

	++_frame;
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

bool Renderer::CreateRenderTarget() {
	if (FAILED(_swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&_backBuffer))) {
		return false;
	}

	D3D11_TEXTURE2D_DESC backBufferDesc;
	_backBuffer->GetDesc(&backBufferDesc);

	ZeroMemory(&_viewport, sizeof(D3D11_VIEWPORT));
	_viewport.Height = (float)(_backBufferHeight = backBufferDesc.Height);
	_viewport.Width = (float)(_backBufferWidth = backBufferDesc.Width);
	_viewport.MinDepth = 0;
	_viewport.MaxDepth = 1;

	if (FAILED(_device->CreateRenderTargetView(
		_backBuffer.Get(),
		nullptr,
		_renderTarget.GetAddressOf()))) {
		return false;
	}

	return true;
}

bool Renderer::CreateResources() {
	constexpr Vertex vertices[] = {
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
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

	if (FAILED(_device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &_vertexBuffer))) {
		return false;
	}

	constexpr unsigned int indices[] = {
		0,2,1, 1,2,3,
		4,5,6, 5,7,6,
		0,1,5, 0,5,4,
		2,6,7, 2,7,3,
		0,4,6, 0,6,2,
		1,3,7, 1,7,5,
	};
	_indexCount = ARRAYSIZE(indices);

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

	if (FAILED(_device->CreateBuffer(&indexBufferDesc, &indexSubresource, &_indexBuffer))) {
		return false;
	}

	CreateMatrices();

	CD3D11_BUFFER_DESC constantBufferDesc(
		sizeof(_constantBufferData),
		D3D11_BIND_CONSTANT_BUFFER
	);

	if (FAILED(_device->CreateBuffer(&constantBufferDesc, nullptr, &_constantBuffer))) {
		return false;
	}

	return true;
}

bool Renderer::CreateShaders() {
	if (!CompileShader(L"vertex_shader.hlsl", "main", "vs_5_0", &_vertexShaderBlob)) {
		return false;
	}

	if (FAILED(_device->CreateVertexShader(
		_vertexShaderBlob->GetBufferPointer(),
		_vertexShaderBlob->GetBufferSize(),
		nullptr,
		&_vertexShader) ) ) {
		return false;
	}

	if (!CompileShader(L"pixel_shader.hlsl", "main", "ps_5_0", &_pixelShaderBlob)) {
		return false;
	}

	if (FAILED(_device->CreatePixelShader(
		_pixelShaderBlob->GetBufferPointer(),
		_pixelShaderBlob->GetBufferSize(),
		nullptr,
		&_pixelShader) ) ) {
		return false;
	}

	constexpr D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	if (FAILED( _device->CreateInputLayout(
		inputLayoutDesc,
		ARRAYSIZE(inputLayoutDesc),
		_vertexShaderBlob->GetBufferPointer(),
		_vertexShaderBlob->GetBufferSize(),
		&_inputLayout ))) {
		return false;
	}

	return true;
}

void Renderer::CreateMatrices() {
	const Vector eye = DirectX::XMVectorSet(0.0f, 0.7f, 1.5f, 0.f);
	const Vector at = DirectX::XMVectorSet(0.0f, -0.1f, 0.0f, 0.f);
	const Vector up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f);

	const Matrix lookAt = DirectX::XMMatrixLookAtRH(eye, at, up);
	const Matrix lookAtT = DirectX::XMMatrixTranspose(lookAt);
	DirectX::XMStoreFloat4x4(&_constantBufferData._view, lookAtT);

	const float aspectRatioX = (float)_backBufferWidth / (float)_backBufferHeight;
	const float aspectRatioY = aspectRatioX < (16.0f / 9.0f) ? aspectRatioX / (16.0f / 9.0f) : 1.0f;
	const float fovAngleY = 2.0f * std::atan(std::tan(DirectX::XMConvertToRadians(70) * 0.5f) / aspectRatioY);

	const Matrix perspective = DirectX::XMMatrixPerspectiveFovRH(fovAngleY, aspectRatioX, 0.01f, 100.0f); 
	const Matrix perspectiveT = DirectX::XMMatrixTranspose(perspective);
	DirectX::XMStoreFloat4x4(&_constantBufferData._projection, perspectiveT);
}

bool Renderer::CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob) {
	constexpr D3D_SHADER_MACRO defines[1] = {};
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

	if (FAILED(result)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob) {
			shaderBlob->Release();
		}

		return false;
	}

	*blob = shaderBlob;

	return true;
}

void Renderer::Update() {
	const float radians = DirectX::XMConvertToRadians((float)_frame++);
	const Matrix rotation = DirectX::XMMatrixRotationY(radians);
	const Matrix rotationT = DirectX::XMMatrixTranspose(rotation);
	DirectX::XMStoreFloat4x4(&_constantBufferData._world, rotationT);

	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, nullptr, &_constantBufferData, 0, 0);
}

void Renderer::Render() {
	constexpr float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), black);

	ID3D11RenderTargetView* renderTargets[] = {_renderTarget.Get()};
	_deviceContext->OMSetRenderTargets(ARRAYSIZE(renderTargets), renderTargets, nullptr);

	D3D11_VIEWPORT viewports[] = {_viewport};
	_deviceContext->RSSetViewports(ARRAYSIZE(viewports), viewports);

	ID3D11Buffer* vertexBuffers[] = {_vertexBuffer.Get()};
	constexpr UINT strides[] = {sizeof(Vertex)};
	constexpr UINT offsets[] = {0};
	_deviceContext->IASetVertexBuffers(0, ARRAYSIZE(vertexBuffers), vertexBuffers, strides, offsets);
	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* constantBuffers[] = {_constantBuffer.Get()};
	_deviceContext->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

	_deviceContext->IASetInputLayout(_inputLayout.Get());
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
	_deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);

	_deviceContext->DrawIndexed(_indexCount, 0, 0);
}

void Renderer::Present() {
	_swapChain->Present(1, 0);
}
