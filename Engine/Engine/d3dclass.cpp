#include "d3dclass.h"

D3DClass::D3DClass() {
    // Initialize all member pointers to null
    m_swapChain = 0;
    m_device = 0;
    m_deviceContext = 0;
    m_renderTargetView = 0;
    m_depthStencilBuffer = 0;
    m_depthStencilState = 0;
    m_depthStencilView = 0;
    m_rasterState = 0;
}

D3DClass::D3DClass(const D3DClass& other) {

}

D3DClass::~D3DClass() {

}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear) {
    // hwnd is to handle the window, rest are self explanatory
    HRESULT result;
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    unsigned int numModes, i, numerator, denominator, stringLength;
    DXGI_MODE_DESC* displayModeList;
    DXGI_ADAPTER_DESC adapterDesc;
    int error;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Texture2D* backBufferPtr;
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;
    float fieldOfView, screenAspect;
    m_vsync_enabled = vsync;

    // Create DX graphics interface factory
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        return false;
    }

    // Use factory to create an adapater for primary video card
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        return false;
    }

    // Enumerate the primary monitor
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        return false;
    }

    // Get num of modes that fit DXGI_FORMAT_R8G8B8A8_UNORM
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(result)) {
        return false;
    }

    // Create list of all possible display modes for the monitor/video card combo
    displayModeList = new DXGI_MODE_DESC[numModes];
    if (!displayModeList) {
        return false;
    }

    // Fill display mode list structures
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    if (FAILED(result)) {
        return false;
    }

    // Loop through all display modes to find matching screenWidth and height
    // Then store numerator and denominator of refresh rate
    for (i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    // Get video card description
    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        return false;
    }
        
    // Store dedicated v mem in megabytes
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
    // Store vCard name
    error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0) {
        return false;
    }

    // Releasing structures and interfaces
    delete[] displayModeList;
    displayModeList = 0;
    adapterOutput->Release();
    adapterOutput = 0;
    adapter->Release();
    adapter = 0;
    factory->Release();
    factory = 0;

    // Filling out swap chain description
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Swap chain refresh rate description
    if (m_vsync_enabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }
    
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    
    if (fullscreen) {
        swapChainDesc.Windowed = false;
    }
    else {
        swapChainDesc.Windowed = true;
    }
        
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;
    featureLevel = D3D_FEATURE_LEVEL_11_0;    // Tells what version of DX to use

                                              // Use D3D_DRIVER_TYPE_REFERENCE if D11 isn't supported
                                              // Else use D3D_DRIVER_TYPE_HARDWARE
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
    if (FAILED(result)) {
        return false;
    }

    // Pointer to back buffer
    result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    if (FAILED(result)) {
        return false;
    }        

    // Create render target view with back buffer pointer
    result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
    if (FAILED(result)) {
        return false;
    }        

    backBufferPtr->Release();
    backBufferPtr = 0;

    // Setup depth buffer description
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
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

    // After polygons are sorted and rasterized, they end up being just 2D coloured pixels
    result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
    if (FAILED(result)) {
        return false;
    }       

    // Depth stencil description
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Depth stencil state
    result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(result)) {
        return false;
    }        

    // Set depth stencil state
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

    // Setup depth stencil view description
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create depth stencil view
    result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
    if (FAILED(result)) {
        return false;
    }        

    // Bind render target and depth stencil to output render pipeline
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // Create rasterizer description
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
    // Create rasterizer state
    result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
    if (FAILED(result)) {
        return false;
    }
        
    m_deviceContext->RSSetState(m_rasterState);    // Set state
    // Viewport description
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    m_deviceContext->RSSetViewports(1, &viewport);    // Create viewport

    fieldOfView = (float)D3DX_PI / 4.0f;
    screenAspect = (float)screenWidth / (float)screenHeight;
    D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);
    D3DXMatrixIdentity(&m_worldMatrix);    // Convert vertice of objects to vertice in 3D space
    D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);
    return true;
}

void D3DClass::Shutdown() {
    // Release and cleanup pointers used in initalize
    // But needs to be in windowed mode first (or else it'll throw exceptions)
    if (m_swapChain) {
        m_swapChain->SetFullscreenState(false, NULL);
    }        

    if (m_rasterState) {
        m_rasterState->Release();
        m_rasterState = 0;
    }
    if (m_depthStencilView) {
        m_depthStencilView->Release();
        m_depthStencilView = 0;
    }
    if (m_depthStencilState) {
        m_depthStencilState->Release();
        m_depthStencilState = 0;
    }
    if (m_depthStencilBuffer) {
        m_depthStencilBuffer->Release();
        m_depthStencilBuffer = 0;
    }
    if (m_renderTargetView) {
        m_renderTargetView->Release();
        m_renderTargetView = 0;
    }
    if (m_deviceContext) {
        m_deviceContext->Release();
        m_deviceContext = 0;
    }
    if (m_device) {
        m_device->Release();
        m_device = 0;
    }
    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = 0;
    }
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha) {
    // Initialize buffers
    float color[4];
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3DClass::EndScene() {
    // Swap chain
    if (m_vsync_enabled) {
        m_swapChain->Present(1, 0);
    }
    else {
        m_swapChain->Present(0, 0);
    }
}

// Pointers to Direct3D device and device context
ID3D11Device* D3DClass::GetDevice() {
    return m_device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext() {
    return m_deviceContext;
}

// Give copies of matrices
void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix) {
    projectionMatrix = m_projectionMatrix;
}

void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix) {
    worldMatrix = m_worldMatrix; 
}

void D3DClass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix) {
    orthoMatrix = m_orthoMatrix; 
}

// Return vCard name and dedicated memory
void D3DClass::GetVideoCardInfo(char* cardName, int& memory) {
    strcpy_s(cardName, 128, m_videoCardDescription);
    memory = m_videoCardMemory;
}