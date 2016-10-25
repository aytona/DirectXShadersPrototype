#include "modelclass.h"

ModelClass::ModelClass()
{
    m_vertexBuffer = 0;
    m_indexBuffer = 0;

	m_Texture = 0;

    m_model = 0;
}

ModelClass::ModelClass(const ModelClass& other)
{

}

ModelClass::~ModelClass()
{

}

bool ModelClass::Initialize(ID3D11Device* device, char* modelFilename, WCHAR* textureFilename)
{
    bool result;

    result = LoadModel(modelFilename);
    if (!result)
        return false;

    result = InitializeBuffers(device);
    if (!result)
        return false;

	result = LoadTexture(device, textureFilename);
	if (!result)
		return false;

    return true;
}

void ModelClass::Shutdown()
{
	ReleaseTexture();
    ShutdownBuffers();
    ReleaseModel();
    return;
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
    RenderBuffers(deviceContext);
    return;
}

int ModelClass::GetIndexCount()
{
    return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
    HRESULT result;
    int i;

    // Create both vertex and index arrays
    vertices = new VertexType[m_vertexCount];
    if (!vertices)
        return false;
    indices = new unsigned long[m_indexCount];
    if (!indices)
        return false;

    // Load vertex and index array with data
    for (i = 0; i < m_vertexCount; i++)
    {
        vertices[i].position = D3DXVECTOR3(m_model[i].x, m_model[i].y, m_model[i].z);
        vertices[i].texture = D3DXVECTOR2(m_model[i].tu, m_model[i].tv);
        vertices[i].normal = D3DXVECTOR3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

        indices[i] = i;
    }

    // Setup static vertex buffer description
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
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

void ModelClass::ShutdownBuffers()
{
    if (m_indexBuffer)
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

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    unsigned int stride;
    unsigned int offset;

    stride = sizeof(VertexType);
    offset = 0;

    // Set vertex buffer to active in the input assembler so it can be rendered
    deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set index buffer to active in the input assembler so it can be rendered
    deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set type of primitive that should be rendered from this vertex buffer
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return;
}

bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
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

void ModelClass::ReleaseTexture()
{
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}

bool ModelClass::LoadModel(char* filename)
{
    ifstream fin;
    char input;
    int i;

    fin.open(filename);

    if (fin.fail())
        return false;

    fin.get(input);
    while (input != ':')
        fin.get(input);

    fin >> m_vertexCount;

    m_indexCount = m_vertexCount;

    m_model = new ModelType[m_vertexCount];
    if (!m_model)
        return false;

    fin.get(input);
    while (input != ':')
        fin.get(input);

    fin.get(input);
    fin.get(input);

    for (i = 0; i < m_vertexCount; i++)
    {
        fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
        fin >> m_model[i].tu >> m_model[i].tv;
        fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
    }

    fin.close();

    return true;
}

void ModelClass::ReleaseModel()
{
    if (m_model)
    {
        delete[] m_model;
        m_model = 0;
    }
    return;
}