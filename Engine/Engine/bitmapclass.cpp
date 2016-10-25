#include "bitmapclass.h"

BitmapClass::BitmapClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;
    m_Texture = 0;
}

BitmapClass::BitmapClass(const BitmapClass& other)
{

}

BitmapClass::~BitmapClass()
{

}

bool BitmapClass::Initialize(ID3D11Device* device, int screenWidth, int screenHeight, WCHAR* textureFilename, int bitmapWidth, int bitmapHeight)
{
    bool result;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_bitmapWidth = bitmapWidth;
    m_bitmapHeight = bitmapHeight;

    // Initialize image location
    // If location hasn't change since last frame, it won't modify the dynamic vertex buffer
    m_previousPosX = -1;
    m_previousPosY = -1;

    // Init vertex and index buffers
    result = InitializeBuffers(device);
    if (!result)
        return false;

    // Load texture for this model
    result = LoadTexture(device, textureFilename);
    if (!result)
        return false;

    return true;
}

void BitmapClass::Shutdown()
{
    ReleaseTexture();
    ShutdownBuffers();
    return;
}

bool BitmapClass::Render(ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
    bool result;

    // Rebuild dynamic vertex buffer for rendering
    result = UpdateBuffers(deviceContext, positionX, positionY);
    if (!result)
        return false;

    // Put vertex and index buffers on the graphics pipeline to prepare for drawing
    RenderBuffers(deviceContext);

    return true;
}

int BitmapClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
    // Returns a pointer to the texture resource
    // Shader will call this function so it has access to the image when drawing the buffers
    return m_Texture->GetTexture();
}

bool BitmapClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // Set vertex and index array count
    m_vertexCount = 6;
    m_indexCount = m_vertexCount;

    // Create vertex and index array
    vertices = new VertexType[m_vertexCount];
    indices = new unsigned long[m_indexCount];
    if (!vertices || !indices)
        return false;
    
    // Init vertex array
    memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

    // Load index array with data
    for (i = 0; i < m_indexCount; i++)
        indices[i] = i;

    // Setup static vertex buffer description
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    // Create vertex buffer
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
    if (FAILED(result))
        return false;

    // Setup static index buffer description
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    // Create index buffer
    result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
    if (FAILED(result))
        return false;

    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;

    return true;
}

void BitmapClass::ShutdownBuffers()
{
    if(m_indexBuffer)
    {
        m_indexBuffer->Release();
        m_indexBuffer = 0;
    }

    if (m_vertexBuffer)
    {
        m_vertexBuffer->Release();
        m_vertexBuffer = 0;
    }

    return;
}


bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
    float left, right, top, bottom;
    VertexType* vertices;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    VertexType* verticesPtr;
    HRESULT result;

    // Check if position of bitmap has changed
    if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
        return true;

    m_previousPosX = positionX;
    m_previousPosY = positionY;

    // Calculating the screen coordinates for each side of the bitmap
    left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
    right = left + (float)m_bitmapWidth;
    top = (float)(m_screenHeight / 2) - (float)positionY;
    bottom = top - (float)m_bitmapHeight;

    // Create vertex array
    vertices = new VertexType[m_vertexCount];
    if (!vertices)
        return false;

    // Load vertex array with data
    vertices[0].position = D3DXVECTOR3(left, top, 0.0f);    // Top left
    vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[1].position = D3DXVECTOR3(right, bottom, 0.0f);    // Bottom right
    vertices[1].texture = D3DXVECTOR2(1.0f, 1.0f);

    vertices[2].position = D3DXVECTOR3(left, bottom, 0.0f); // Bottom left
    vertices[2].texture = D3DXVECTOR2(0.0f, 1.0f);

    vertices[3].position = D3DXVECTOR3(left, top, 0.0f);    // Top left
    vertices[3].texture = D3DXVECTOR2(0.0f, 0.0f);

    vertices[4].position = D3DXVECTOR3(right, top, 0.0f);   // Top right
    vertices[4].texture = D3DXVECTOR2(1.0f, 0.0f);

    vertices[5].position = D3DXVECTOR3(right, bottom, 0.0f);    // Bottom right
    vertices[5].texture = D3DXVECTOR2(1.0f, 1.0f);

    // Lock vertex buffer to be written to
    result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    // Pointer to the data in the vertex buffer
    verticesPtr = (VertexType*)mappedResource.pData;

    // Copy data into vertex buffer
    memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

    // Unlock vertex buffer
    deviceContext->Unmap(m_vertexBuffer, 0);

    delete[] vertices;
    vertices = 0;

    return true;
}

void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    stride = sizeof(VertexType);
    offset = 0;

    // Set vertex buffer to active in the input assembler so it can be rendered
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set index buffer to active in the inpuse assembler so it can be rendered
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set type of primitive that should be rendered from this vertex buffer
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

bool BitmapClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
    bool result;

    m_Texture = new TextureClass;
    if (!m_Texture)
        return false;

    result = m_Texture->Initialize(device, filename);
    if (!result)
        return false;

    return true;
}

void BitmapClass::ReleaseTexture()
{
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}