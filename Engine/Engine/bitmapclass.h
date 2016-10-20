#pragma once

#include <d3d11.h>
#include <d3dx10math.h>
#include "textureclass.h"

class BitmapClass
{
private:
    struct VertexType
    {
        D3DXVECTOR3 position;
        D3DXVECTOR2 texture;
    };

public:
    BitmapClass();
    BitmapClass(const BitmapClass&);
    ~BitmapClass();

    bool Initialize(ID3D11Device*, int, int, WCHAR*, int, int);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, int, int);

    int GetIndexCount();
    ID3D11ShaderResourceView* GetTexture();

private:
    bool InitializeBuffers(ID3D11Device*);
    void ShutdownBuffers();
    bool UpdateBuffers(ID3D11DeviceContext*, int, int);
    void RenderBuffers(ID3D11DeviceContext*);

    bool LoadTexture(ID3D11Device*, WCHAR*);
    void ReleaseTexture();

private:
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
    int m_vertexCount;
    int m_indexCount;
    TextureClass* m_Texture;

    // Maintain some extra info that a 3D model wouldn't
    int m_screenWidth;
    int m_screenHeight;
    int m_bitmapWidth;
    int m_bitmapHeight;
    int m_previousPosX;
    int m_previousPosY;
};