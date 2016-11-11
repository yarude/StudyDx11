////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"

D3DClass::D3DClass()
{
	// �����г�Աָ�븳��
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}

// vsync��ʾ�Ƿ���Ҫͬ������ʾ����ˢ��Ƶ�ʣ�����Ҫ�Ļ��ͻᾡ���ܿ��ˢ��
// hwndΪ���ھ������dx��ȡ�����Ĵ��ڵ���Ϣ
bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;      // ���Դ����ӿڹ����࣬�ö�������ö�ٳ��Կ��豸
	IDXGIAdapter* adapter;      // �Կ��豸�ӿ��࣬��������ö�ٳ���ʾ�豸 (������ʾ��)�����Ի�ȡ�Կ���һЩ��Ϣ
	IDXGIOutput* adapterOutput; // ��ʾ���豸�ӿ��࣬���Ը��ݸ�����rgba��ʽ��ѯ֧�ֵ���ʾģʽ (�ֱ��ʺ�ˢ����)
	unsigned int numModes, i, numerator, denominator; // ��ʾ�����Կ�֧�ֵ���ʾģʽ������ˢ���ʵķ��Ӻͷ�ĸ
	unsigned long long stringLength;                  // �����������Կ���������ת�����ַ���ʱʹ��
	DXGI_MODE_DESC* displayModeList;                  // ��ʱ������ʾ��֧�ֵ�ģʽ������
	DXGI_ADAPTER_DESC adapterDesc;                    // �����Կ�������Ϣ
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc; // ������������ʵ������Ȼ�����ã����ڴ���D3D�豸�ͽ�����
	D3D_FEATURE_LEVEL featureLevel;     // ����API���𣬿������ó���ǰ��Dx9��Dx12
	ID3D11Texture2D* backBufferPtr;     // ������ʱ��ź󱳻��������Ȼ�ȡ��ϢȻ�������������У�Ȼ����ݴ��µ���Ϣ������Ⱦ������ͼ
	D3D11_TEXTURE2D_DESC depthBufferDesc; // ���ڴ�����Ȼ���������ʱ��Ϣ��
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc; // ��Ϣ������������Ȳ��Ժ�ģ�����
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; // ��Ϣ�����ڴ�����Ⱥ�ģ����ͼ
	D3D11_RASTERIZER_DESC rasterDesc;                   // ��Ϣ�����ڿ��ƹ�դ��״̬
	D3D11_VIEWPORT viewport;    // ��Ϣ�����������ӿڿռ�
	float fieldOfView, screenAspect;


	// Store the vsync setting.
	m_vsync_enabled = vsync;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	// ����ö���Կ���һ�����������˵�ǰ����ö�ٵڼ����Կ�����ȡ�����Կ��豸����adapter
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	// ö�ٳ���ʾ��
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	// ��ȡ����DXGI_FORMAT_R8G8B8A8_UNORMģʽ������
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	// ����һ���б��������浱ǰ��ʾ�����Կ���Ͽ���֧�ֵ���ʾģʽ
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	// Ѱ�ҵ����ϵ�ǰ��ʾ�ֱ��ʵ���ʾģʽ����¼����ˢ����
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator; // ���ӿ�������60
				denominator = displayModeList[i].RefreshRate.Denominator; // ��ĸ������1
			}
		}
	}

	// Get the adapter (video card) description.
	// ��ȡ�Կ�������
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	// ��ȡ�Դ�Ĵ�С
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	// ���Կ�����ת�����ַ�������
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// �����������Ѿ���ȡ���Կ�����Ϣ�Լ�ˢ���ʵķ��Ӻͷ�ĸ��һЩ��ʱ�ṹ���Լ�һЩ���ڻ�ȡ��Ϣ�Ľӿڿ����ͷ���

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// ��ʼ��������
	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	// ���ý�������Ƶ�ʣ�����֮ǰ���õ��Ƿ���Ҫ��ֱͬ��
	// Set the refresh rate of the back buffer.
	if (m_vsync_enabled)
	{
		// ��ʹ�ô�ֱͬ��ʱ��Ҫ��ˢ�������óɺ�֮ǰ��ȡ��һ��
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	// ����backbuffer���÷�
	// ����һ����DXGI_USAGE_BACK_BUFFER��ʾ������Ⱦ���籣��һ��ͼƬ���ļ�
	// �²���ʱֻ��backû��front����DXGI_USAGE_RENDER_TARGET_OUTPUT�Ǳ�׼�Ľ������÷�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off.
	// �رն��ز���
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// Set the scan line ordering and scaling to unspecified.
	// �����׺����ɨ�����й�
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	// ���ύ�Ժ���backbuffer������
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	// ���ù��ܼ��𻹿������ó�dx9��dx10������֧��һЩ���豸��
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	// D3D_DRIVER_TYPE_HARDWARE������D3D_DRIVER_TYPE_REFERENCE��������û��֧��dx11���Կ��Ļ�
	// ���ﴴ���˽�������D3D�豸��D3D�������豸
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}


	// Get the pointer to the back buffer.
    // ��ȡ�󻺳�����ַ
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
    // ��ȾĿ����ͼ˵�����backbuffer�ǹ���Ⱦ�õģ���ʱҪ�������ͼ�󶨵���ˮ�ߵ���Ⱦ�׶�
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;


	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;


	// Create the texture for the depth buffer using the filled out description.
    // �����������ģ�建�������buffer
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}


	// Initialize the description of the stencil state.
	// ���Դ���������Ȳ��Ժ�ģ����Ե�state
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Initialize the depth stencil view.
	// ��ʼ����Ⱥ�ģ����ͼ��ÿһ��buffer��Դ��Ҫ��һ����ͼ��������buffer�󶨵�����
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
    // ������Ⱥ�ģ��buffer����ͼ����ʵ���ǽ�һ�ŷ����ָ�������󶨵���Ⱥ�ģ�����;
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// ����ȾĿ����ͼ������Լ�ģ�建��󶨵�����Ⱦ��ˮ�ߵ����
	// ������Ⱦ��ˮ�߽�ͼ����Ƶ�backbuffer��Ȼ����ͨ��������������ֵ���Ļ
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// ������Ϊֹ��ȾĿ������ú��ˣ�����Щ����ĺ�������ʹ�����ǿ��Ƴ�����
	// �����դ��״̬�������ƶ������������Ⱦ�����ǿ������ó��߿���Ⱦģʽ������˫�����Ρ�
	// Ĭ�ϵĹ�դ��״̬�����µ�����
	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

	// ����viewport�Թ��ü��ռ�ӳ�䵽��ȾĿ��ռ�
	// Setup the viewport for rendering.
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &viewport);

	// ����ͶӰ����
	// Setup the projection matrix.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// Initialize the world matrix to the identity matrix.
	// �������������һ����λ����
	m_worldMatrix = XMMatrixIdentity();

	// ����һ������ͶӰ����
	// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}




void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	// �����ȫ��ģʽ�¿��ܻ��׳��쳣���������л���ȫ��
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}

// ������ո���buffer
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

// ��ȡd3d�豸��d3d�������豸���������豸������ǰdx9���һ��d3d�豸
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

// ���ߺ�����ȡ֮ǰ���õ�һЩ����
void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

// ��ȡ�Կ������ƺ��Դ��С
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}
