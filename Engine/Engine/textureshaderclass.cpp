#include "textureshaderclass.h"

TextureShaderClass::TextureShaderClass()
{
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;

    m_sampleState = 0;
}

TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{

}

TextureShaderClass::~TextureShaderClass()
{

}

bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    bool result;

    // Init vertex and pixel shaders
    result = InitializeShader(device, hwnd, L"../Engine/texture.vs", L"../Engine/texture.ps");
    if (!result)
        return false;

    return true;
}

void TextureShaderClass::Shutdown()
{
    ShutdownShader();
    return;
}

bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
    bool result;

    // Set shader params that it will use for rendering
    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
    if (!result)
        return false;

    // Render the buffers with the shader
    RenderShader(deviceContext, indexCount);

    return true;
}

bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ID3D10Blob* errorMessage;
    ID3D10Blob* vertexShaderBuffer;
    ID3D10Blob* pixelShaderBuffer;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;

    D3D11_SAMPLER_DESC samplerDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    // Compile vertex shader
    result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
    if (FAILED(result))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        }
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Compile pixel shader
    result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
    if (FAILED(result))
    {
        if (errorMessage)
        {
            OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        }
        else
        {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Create vertex shader from buffer
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result))
        return false;

    // Create pixel shader from buffer
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result))
        return false;

    // Create vertex input layout description
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create vertex input layout
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
        return false;

    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Setup dynamic matrix constant buffer description in vertex shader
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create constant buffer pointer for access
    result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result))
        return false;

    // Create texture sampler state description
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create texture sampler state
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result))
        return false;

    return true;
}

void TextureShaderClass::ShutdownShader()
{
    if (m_sampleState)
    {
        m_sampleState->Release();
        m_sampleState = 0;
    }
    if (m_matrixBuffer)
    {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }
    if (m_layout)
    {
        m_layout->Release();
        m_layout = 0;
    }
    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }
    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }
    return;
}

void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long bufferSize, i;
    ofstream fout;

    compileErrors = (char*)(errorMessage->GetBufferPointer());

    bufferSize = errorMessage->GetBufferSize();

    fout.open("shader-error.txt");

    for (i = 0; i < bufferSize; i++)
        fout << compileErrors[i];

    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);

    return;
}

bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;

    // Prep matrices for shader
    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    // Lock constant buffer to be written to
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    dataPtr = (MatrixBufferType*)mappedResource.pData;

    dataPtr->world = worldMatrix;
    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlcok constant buffer
    deviceContext->Unmap(m_matrixBuffer, 0);

    bufferNumber = 0;

    // Set constant buffer in vertex shader with updated values
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Set shader texture resource in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // Set vertex input layout
    deviceContext->IASetInputLayout(m_layout);

    // Set vertex and pixel shaders
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Set sampler state in pixel shader
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);

    // Render
    deviceContext->DrawIndexed(indexCount, 0, 0);

    return;
}