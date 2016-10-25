#include "textclass.h"

TextClass::TextClass()
{
    m_Font = 0;
    m_FontShader = 0;

    m_sentence1 = 0;
    m_sentence2 = 0;
}

TextClass::TextClass(const TextClass& other)
{

}

TextClass::~TextClass()
{

}

bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, D3DXMATRIX baseViewMatrix)
{
    bool result;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_baseViewMatrix = baseViewMatrix;

    // Create font object
    m_Font = new FontClass;
    if (!m_Font)
        return false;

    // Init font object
    result = m_Font->Initialize(device, "../Engine/data/fontdata.txt", L"../Engine/data/font.dds");
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
        return false;
    }

    // Create font shader object
    m_FontShader = new FontShaderClass;
    if (!m_FontShader)
        return false;

    // Init font shader object
    result = m_FontShader->Initialize(device, hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
        return false;
    }

    // Init first sentence
    result = InitializeSentence(&m_sentence1, 16, device);
    if (!result)
        return false;

    // Update sentence vertex buffer with new string info
    result = UpdateSentence(m_sentence1, "Hello", 100, 100, 1.0f, 1.0f, 1.0f, deviceContext);
    if (!result)
        return false;

    // Init second sentence
    result = InitializeSentence(&m_sentence2, 16, device);
    if (!result)
        return false;

    // Update sentence vertex buffer with new string info
    result = UpdateSentence(m_sentence2, "Goodbye", 100, 200, 1.0f, 1.0f, 1.0f, deviceContext);
    if (!result)
        return false;

    return true;
}

void TextClass::Shutdown()
{
    // Release same amount of sentences used
    ReleaseSentence(&m_sentence1);
    ReleaseSentence(&m_sentence2);

    if (m_FontShader)
    {
        m_FontShader->Shutdown();
        delete m_FontShader;
        m_FontShader = 0;
    }

    if (m_Font)
    {
        m_Font->Shutdown();
        delete m_Font;
        m_Font = 0;
    }

    return;
}

bool TextClass::Render(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
    bool result;

    // Draw first sentence
    result = RenderSentence(deviceContext, m_sentence1, worldMatrix, orthoMatrix);
    if (!result)
        return false;

    // Draw second sentence
    result = RenderSentence(deviceContext, m_sentence2, worldMatrix, orthoMatrix);
    if (!result)
        return false;

    return true;
}

bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc;
    D3D11_BUFFER_DESC indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;
    D3D11_SUBRESOURCE_DATA indexData;
    HRESULT result;
    int i;

    // Create new sentence object
    *sentence = new SentenceType;
    if (!*sentence)
        return false;

    // Init sentence buffers
    (*sentence)->vertexBuffer = 0;
    (*sentence)->indexBuffer = 0;

    // Set max length of sentence
    (*sentence)->maxLenght = maxLength;

    // Set num of vertices in the vertex array
    (*sentence)->vertexCount = 6 * maxLength;

    // Set num of indices in the index array
    (*sentence)->indexCount = (*sentence)->vertexCount;

    // Create vertex array
    vertices = new VertexType[(*sentence)->vertexCount];
    if (!vertices)
        return false;

    // Create index array
    indices = new unsigned long[(*sentence)->indexCount];
    if (!indices)
        return false;

    // Init vertex array
    memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

    // Init index array
    for (i = 0; i < (*sentence)->indexCount; i++)
        indices[i] = i;

    // Setup dynamic vertex buffer description
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Create vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
    if (FAILED(result))
        return false;

    // Setup static index buffer description
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
    if (FAILED(result))
        return false;

    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

bool TextClass::UpdateSentence(SentenceType* sentence, char* text, int positionX, int positionY, float red, float green, float blue, ID3D11DeviceContext* deviceContext)
{
    int numLetters;
    VertexType* vertices;
    float drawX, drawY;
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;

    sentence->red = red;
    sentence->green = green;
    sentence->blue = blue;

    // Get num letters in sentence
    numLetters = (int)strlen(text);

    // Check for possible buffer overflow
    if (numLetters > sentence->maxLenght)
        return false;

    // Create vertex array
    vertices = new VertexType[sentence->vertexCount];
    if (!vertices)
        return false;

    // Init vertex array
    memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

    // Calc the X and Y pixel position on screen to start drawing to
    drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
    drawY = (float)((m_screenHeight / 2) - positionY);

    // Use font class to build vertex array from the sentence text and sentence draw location
    m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

    // Lock vertex buffer
    result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    verticesPtr = (VertexType*)mappedResource.pData;

    // Copy data into vertex buffer
    memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

    // Unlock vertex buffer
    deviceContext->Unmap(sentence->vertexBuffer, 0);

    delete[] vertices;
    vertices = 0;

    return true;
}

void TextClass::ReleaseSentence(SentenceType** sentence)
{
    if (*sentence)
    {
        if ((*sentence)->vertexBuffer)
        {
            (*sentence)->vertexBuffer->Release();
            (*sentence)->vertexBuffer = 0;
        }

        if ((*sentence)->indexBuffer)
        {
            (*sentence)->indexBuffer->Release();
            (*sentence)->indexBuffer = 0;
        }

        delete * sentence;
        *sentence = 0;
    }

    return;
}

bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
    unsigned int stride, offset;
    D3DXVECTOR4 pixelColor;
    bool result;

    stride = sizeof(VertexType);
    offset = 0;

    // Set vertex buffer to active in input assembler so it can be rendered
    deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    // Set index buffer to active in input assembler so it can be rendered
    deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set type of primitive that should be rendered from this vertex buffer
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create pixel color vector with the input sentence color
    pixelColor = D3DXVECTOR4(sentence->red, sentence->green, sentence->blue, 1.0f);

    // Render text using the font shader
    result = m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor);

    if (!result)
        return false;

    return true;
}