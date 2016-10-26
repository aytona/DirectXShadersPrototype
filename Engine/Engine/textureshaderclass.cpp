#include "textureshaderclass.h"

TextureShaderClass::TextureShaderClass() {
    m_vertexShader = 0;
    m_pixelShader = 0;
    m_layout = 0;
    m_matrixBuffer = 0;
    m_sampleState = 0;
}

TextureShaderClass::TextureShaderClass(const TextureShaderClass& other) {

}

TextureShaderClass::~TextureShaderClass() {

}

bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd) {
    bool result;

    result = InitializeShader(device, hwnd, L"../Engine/sprite_vs.hlsl", L"../Engine/sprite_ps.hlsl");
    if (!result) {
        return false;
    }
    return true;
}

void TextureShaderClass::Shutdown() {
    ShutdownShader();
}

bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture) {
    bool result;

    result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
    if (!result) {
        return false;
    }

    RenderShader(deviceContext, indexCount);

    return true;
}

bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename) {
    HRESULT result;
    ID3D10Blob *errorMessage, *vertexShaderBuffer, *pixelShaderBuffer;
    int i;
    unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;

    errorMessage = 0;
    vertexShaderBuffer = 0;
    pixelShaderBuffer = 0;

    D3D11_BUFFER_DESC vbd;
    vbd.ByteWidth = sizeof(GuiElementVertexData);
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0; vbd.StructureByteStride = 0;

    result = device->CreateBuffer(&vbd, 0, &m_guiBuffer);
    if (result != S_OK) {
        return result;
    }

    result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
    if (FAILED(result)) {
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
        } else {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    result = D3DX11CompileFromFile(psFilename, NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
    if (FAILED(result)) {
        if (errorMessage) {
        OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
        } else {
            MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // Create vertex shader from buffer
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result)) {
        return false;
    }
        
    // Create pixel shader from buffer
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result)) {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },{ "SOURCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    numElements = sizeof(layoutDesc) / sizeof(layoutDesc[0]);

    // Create vertex input layout
    result = device->CreateInputLayout(layoutDesc, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) {
        return false;
    }

    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;
    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    return true;
}

void TextureShaderClass::ShutdownShader() {
    if (m_layout) {
        m_layout->Release();
        m_layout = 0;
    }
    if (m_vertexShader) {
        m_vertexShader->Release();
        m_vertexShader = 0;
    }
    if (m_pixelShader) {
        m_pixelShader->Release();
        m_pixelShader = 0;
    }
    if (m_matrixBuffer) {
        m_matrixBuffer->Release();
        m_matrixBuffer = 0;
    }
    if (m_sampleState) {
        m_sampleState->Release();
        m_sampleState = 0;
    }
}

void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {
    char* compileErrors;
    unsigned long bufferSize, i;
    ofstream fout;

    compileErrors = (char*)(errorMessage->GetBufferPointer());
    bufferSize = errorMessage->GetBufferSize();
    fout.open("shader-error.txt");

    for (i = 0; i < bufferSize; i++) {
        fout << compileErrors[i];
    }
        
    fout.close();

    errorMessage->Release();
    errorMessage = 0;

    MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
    return;
}

bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture) {
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    GuiElementVertexData* dataPtr;
    unsigned int bufferNumber;

    // Prep matrices for shader
    D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
    D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
    D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

    // Lock constant buffer to be written to
    result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    //dataPtr = (GuiElementVertexData*)mappedResource.pData;

    //dataPtr->world = worldMatrix;
    //dataPtr->view = viewMatrix;
    //dataPtr->projection = projectionMatrix;

    // Unlcok constant buffer
    deviceContext->Unmap(m_matrixBuffer, 0);

    bufferNumber = 0;

    // Set constant buffer in vertex shader with updated values
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Set shader texture resource in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &texture);

    return true;
}

void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount) {
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

