#include "renderer.hpp"

#include <d3dcompiler.h>

#include <cmath>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>


namespace {
	struct Vertex {
		using Float3 = DirectX::XMFLOAT3;

		Float3 _position;
		Float3 _color;
	};

	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}
}


bool Renderer::Create(const Config& config) {
	if (!CreateContext(config)) {
		std::cerr << "Failed creating render context" << std::endl;
		return false;
	}

	if (!CreateRenderTarget()) {
		std::cerr << "Failed creating render target" << std::endl;
		return false;
	}

	if (!CreateDepthStencil()) {
		std::cerr << "Failed creating depth stencil" << std::endl;
		return false;
	}

	if (!CreateResources()) {
		std::cerr << "Failed creating resources" << std::endl;
		return false;
	}

	if (!CreateShaders()) {
		std::cerr << "Failed creating shaders" << std::endl;
		return false;
	}

	return true;
}

void Renderer::Run(long mousePosX, long mousePosY, bool clicking) {
	Update(mousePosX, mousePosY, clicking);
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

bool Renderer::CreateDepthStencil() {
	const CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		_backBufferWidth,
		_backBufferHeight,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
	);

	if (FAILED(_device->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencil))) {
		return false;
	}

	const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);

	if (FAILED(_device->CreateDepthStencilView(
		_depthStencil.Get(),
		&depthStencilViewDesc,
		&_depthStencilView))) {
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

	const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(D3D11_SUBRESOURCE_DATA));
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

	const CD3D11_BUFFER_DESC indexBufferDesc(
		sizeof(indices),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA indexSubresource;
	ZeroMemory(&indexSubresource, sizeof(D3D11_SUBRESOURCE_DATA));
	indexSubresource.pSysMem = indices;
	indexSubresource.SysMemPitch = 0;
	indexSubresource.SysMemSlicePitch = 0;

	if (FAILED(_device->CreateBuffer(&indexBufferDesc, &indexSubresource, &_indexBuffer))) {
		return false;
	}

	CreateMatrices();

	const CD3D11_BUFFER_DESC constantBufferDesc(
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
	const Vector eye = DirectX::XMVectorSet(0.0f, 0.7f, 1.5f, 0.0f);
	const Vector at = DirectX::XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	const Vector up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	CreateViewMatrix(eye, at, up);

	const float aspectRatio = (float)_backBufferWidth / (float)_backBufferHeight;
	const float ratio = aspectRatio < (16.0f / 9.0f) ? aspectRatio / (16.0f / 9.0f) : 1.0f;
	constexpr float fov = 70.0;
	const float verticalFov = 2.0f * std::atan(std::tan(DirectX::XMConvertToRadians(fov) * 0.5f) / ratio);

	constexpr float nearPlane = 0.01f;
	constexpr float farPlane = 100.0f;
	const Matrix perspective = DirectX::XMMatrixPerspectiveFovRH(verticalFov, aspectRatio, nearPlane, farPlane); 
	_projection._projection = DirectX::XMMatrixTranspose(perspective);

	const Matrix rotation = DirectX::XMMatrixRotationY(0.0f);
	_projection._model = DirectX::XMMatrixTranspose(rotation);
}

void Renderer::CreateViewMatrix(const Vector& eye, const Vector& at, const Vector& up) {
	_camera._eye = eye;
	_camera._at = at;
	_camera._up = up;

	const Matrix lookAt = DirectX::XMMatrixLookAtRH(eye, at, up);
	_projection._view = DirectX::XMMatrixTranspose(lookAt);
}

bool Renderer::CompileShader(LPCWSTR srcFile, LPCSTR entryPoint, LPCSTR profile, ID3DBlob** blob) {
	constexpr D3D_SHADER_MACRO defines[1] = {};
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	const HRESULT result = D3DCompileFromFile(
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

void Renderer::Update(long mousePosX, long mousePosY, bool clicking) {
	if (clicking) {
		UpdateArcballCamera(mousePosX, mousePosY);
	}

	const Matrix mvp = _projection._projection * _projection._view * _projection._model;
	DirectX::XMStoreFloat4x4(&_constantBufferData._mvp, mvp);

	_deviceContext->UpdateSubresource(_constantBuffer.Get(), 0, nullptr, &_constantBufferData, 0, 0);

	_lastMousePosX = mousePosX;
	_lastMousePosY = mousePosY;
}

void Renderer::Render() {
	constexpr float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), black);
	_deviceContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* renderTargets[] = {_renderTarget.Get()};
	_deviceContext->OMSetRenderTargets(ARRAYSIZE(renderTargets), renderTargets, _depthStencilView.Get());

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

void Renderer::UpdateArcballCamera(long mousePosX, long mousePosY) {
	const float deltaAngleX = (2.0f * (float)M_PI) / _viewport.Width;
	float deltaAngleY = (float)M_PI / _viewport.Height;

	const Vector viewDir = DirectX::XMVectorNegate(_projection._view.r[2]);
	const float cosAngle = DirectX::XMVectorGetX(DirectX::XMVector3Dot(viewDir, _camera._up));
	// TODO: Never hit
	if (cosAngle * sgn(deltaAngleY) > 0.99f) {
		deltaAngleY = 0.0f;
	}

	Vector position = DirectX::XMVectorSetW(_camera._eye, 1.0f);
	Vector pivot = DirectX::XMVectorSetW(_camera._at, 1.0f);

	const float angleX = (_lastMousePosX - mousePosX) * deltaAngleX;
	const Matrix rotMatrixX = DirectX::XMMatrixRotationAxis(_camera._up, angleX);
	position = DirectX::XMVectorAdd(DirectX::XMVector3Transform(
		DirectX::XMVectorSubtract(position, pivot), rotMatrixX), pivot);

	const float angleY = (_lastMousePosY - mousePosY) * deltaAngleY;
	const Vector rightVec = _projection._view.r[0];
	const Matrix rotMatrixY = DirectX::XMMatrixRotationAxis(rightVec, angleY);
	const Vector cameraPos = DirectX::XMVectorAdd(
		DirectX::XMVector3Transform(DirectX::XMVectorSubtract(position, pivot), rotMatrixY), pivot);

	CreateViewMatrix(cameraPos, _camera._at, _camera._up);
}
