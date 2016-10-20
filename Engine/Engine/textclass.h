#pragma once

#include "fontclass.h"
#include "fontshaderclass.h"

class TextClass
{
private:
    struct SentenceType
    {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        int vertexCount;
        int indexCount;
        int maxLenght;
        float red;
        float green;
        float blue;
    };

    struct VertexType
    {
        D3DXVECTOR3 position;
        D3DXVECTOR2 texture;
    };

public:
    TextClass();
    TextClass(const TextClass&);
    ~TextClass();

    bool Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, D3DXMATRIX);
    void Shutdown();
    bool Render(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX);

private:
    bool InitializeSentence(SentenceType**, int, ID3D11Device*);
    bool UpdateSentence(SentenceType*, char*, int, int, float, float, float, ID3D11DeviceContext*);
    void ReleaseSentence(SentenceType**);
    bool RenderSentence(ID3D11DeviceContext*, SentenceType*, D3DXMATRIX, D3DXMATRIX);

private:
    FontClass* m_Font;
    FontShaderClass* m_FontShader;
    int m_screenWidth;
    int m_screenHeight;
    D3DXMATRIX m_baseViewMatrix;

    // Number of sentence type equal to number of sentences that will be used
    SentenceType* m_sentence1;
    SentenceType* m_sentence2;
};