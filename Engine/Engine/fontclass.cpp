#include "fontclass.h"

FontClass::FontClass()
{
    m_Font = 0;
    m_Texture = 0;
}

FontClass::FontClass(const FontClass& other)
{

}

FontClass::~FontClass()
{

}

bool FontClass::Initialize(ID3D11Device* device, char* fontFilename, WCHAR* textureFilename)
{
    bool result;

    // Load text file containing font data
    result = LoadFontData(fontFilename);
    if (!result)
        return false;

    // Load texture that has font characters on it
    result = LoadTexture(device, textureFilename);
    if (!result)
        return false;

    return true;
}

void FontClass::Shutdown()
{
    ReleaseTexture();
    ReleaseFontData();

    return;
}

bool FontClass::LoadFontData(char* filename)
{
    ifstream fin;
    int i;
    char temp;

    // Create font spacing buffer
    m_Font = new FontType[95];
    if (!m_Font)
        return false;

    // Read font size and spacing between chars
    fin.open(filename);
    if (fin.fail())
        return false;

    // Read in 95 used ascii characters for text
    for (i = 0; i < 95; i++)
    {
        fin.get(temp);
        while (temp != ' ')
        {
            fin.get(temp);
        }
        fin.get(temp);

        while (temp != ' ')
        {
            fin.get(temp);
        }

        fin >> m_Font[i].left;
        fin >> m_Font[i].right;
        fin >> m_Font[i].size;
    }

    fin.close();

    return true;
}

void FontClass::ReleaseFontData()
{
    if (m_Font)
    {
        delete[] m_Font;
        m_Font = 0;
    }

    return;
}

bool FontClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
{
    bool result;

    // Create texture object
    m_Texture = new TextureClass;
    if (!m_Texture)
        return false;

    // Init texture object
    result = m_Texture->Initialize(device, filename);
    if (!result)
        return false;

    return true;
}

void FontClass::ReleaseTexture()
{
    if (m_Texture)
    {
        m_Texture->Shutdown();
        delete m_Texture;
        m_Texture = 0;
    }

    return;
}

ID3D11ShaderResourceView* FontClass::GetTexture()
{
    return m_Texture->GetTexture();
}

// Called by the TextClass to build vertex buffers out of text sentences
void FontClass::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
{
    VertexType* vertexPtr;
    int numLetters, index, i, letter;

    // Coerce input vertices into a VertexType structure
    vertexPtr = (VertexType*)vertices;

    // Get number of letters in the sentence
    numLetters = (int)strlen(sentence);

    index = 0;

    // Draw each letter onto a quad
    for (i = 0; i < numLetters; i++)
    {
        letter = ((int)sentence[i] - 32);

        // If space then just move over X amount of pixels
        if (letter == 0)
            drawX = drawX + 3.0f;
        else
        {
            // First triangle in quad
            vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);    // Top left
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 0.0f);
            index++;

            vertexPtr[index].position = D3DXVECTOR3((drawX + m_Font[letter].size), (drawY - 16), 0.0f); // Bottom right
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 1.0f);
            index++;

            vertexPtr[index].position = D3DXVECTOR3(drawX, (drawY - 16), 0.0f); // Bottom left
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 1.0f);
            index++;

            // Second triangle in quad
            vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);    // Top left
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].left, 0.0f);
            index++;

            vertexPtr[index].position = D3DXVECTOR3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 0.0f);
            index++;

            vertexPtr[index].position = D3DXVECTOR3((drawX + m_Font[letter].size), (drawY - 16), 0.0f); // Bottom right
            vertexPtr[index].texture = D3DXVECTOR2(m_Font[letter].right, 1.0f);
            index++;

            // Update x location for drawing by the size of the letter + 1 px
            drawX = drawX + m_Font[letter].size + 1.0f;
        }
    }
    
    return;
}