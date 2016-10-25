#include "graphicsclass.h"
#include "cameraclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
    m_Camera = 0;
    m_Model = 0;
	m_LightShader = 0;
	m_Light = 0;
    m_TextureShader = 0;
    m_Bitmap = 0;
    m_Text = 0;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other)
{

}

GraphicsClass::~GraphicsClass()
{

}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
    D3DXMATRIX baseViewMatrix;

    // Create the Direct3D object
	m_D3D = new D3DClass;
	if (!m_D3D)
		return false;

    // Init the Direct3D object
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

    // Create camera object
    m_Camera = new CameraClass;
    if (!m_Camera)
        return false;

    // Set initial position of camera
    m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

    m_Camera->Render();
    m_Camera->GetViewMatrix(baseViewMatrix);

    // Create text object
    m_Text = new TextClass;
    if (!m_Text)
        return false;

    // Init text object
    result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
        return false;
    }

    // Create texture shader object
    m_TextureShader = new TextureShaderClass;
    if (!m_TextureShader)
        return false;

    // Init texture sahder object
    result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
        return false;
    }

    // Create model object
    m_Model = new ModelClass;
    if (!m_Model)
        return false;

    // Init model object
    result = m_Model->Initialize(m_D3D->GetDevice(), "../Engine/data/cube.txt", L"../Engine/data/seafloor.dds");
    if (!result)
    {
        MessageBox(hwnd, L"Coult not initialize the model object", L"Error", MB_OK);
        return false;
    }

    // Create light shader object
	m_LightShader = new LightShaderClass;
	if (!m_LightShader)
		return false;

    // Init light shader object
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

    // Create light object
	m_Light = new LightClass;
	if (!m_Light)
		return false;

    // Init light object
    m_Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetDirection(0.0f, 0.0f, 1.0f);
    m_Light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_Light->SetSpecularPower(32.0f);       // Lower power the greater the effect

    m_Bitmap = new BitmapClass;
    if (!m_Bitmap)
        return false;

    result = m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"../Engine/data/seafloor.dds", 256, 256);
    if (!result)
    {
        MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
        return false;
    }

	return true;
}

void GraphicsClass::Shutdown()
{
    if (m_Text)
    {
        m_Text->Shutdown();
        delete m_Text;
        m_Text = 0;
    }

    if (m_Bitmap)
    {
        m_Bitmap->Shutdown();
        delete m_Bitmap;
        m_Bitmap = 0;
    }

    if (m_TextureShader)
    {
        m_TextureShader->Shutdown();
        delete m_TextureShader;
        m_TextureShader = 0;
    }

	if (m_Light)
	{
		delete m_Light;
		m_Light = 0;
	}
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}
    if (m_Model)
    {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = 0;
    }
    if (m_Camera)
    {
        delete m_Camera;
        m_Camera = 0;
    }
	if (m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}
	return;
}

bool GraphicsClass::Frame()
{
	bool result;
	static float rotation = 0.0f;

    // Update the rotation variable each frame
	rotation += (float)D3DX_PI * 0.005f;
	if (rotation > 360.0f)
		rotation -= 360;
    
    // Render the graphics scene
	result = Render(rotation);
	if (!result)
		return false;

	return true;
}

bool GraphicsClass::Render(float rotation)
{
    D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix, orthoMatrix;
    bool result;

    // Clear buffers
	m_D3D->BeginScene(0.0f, 0.5f, 0.5f, 1.0f);

    // Generate the view matrix based on camera's position
    m_Camera->Render();

    m_Camera->GetViewMatrix(viewMatrix);
    m_D3D->GetWorldMatrix(worldMatrix);
    m_D3D->GetProjectionMatrix(projectionMatrix);

    m_D3D->GetOrthoMatrix(orthoMatrix);

    // Turn off the Z buffer to begin all 2D rendering
    m_D3D->TurnZBufferOff();

    // Put bitmap vertex and index buffers on the graphics pipeline to prepare for drawing
    result = m_Bitmap->Render(m_D3D->GetDeviceContext(), 100, 100);
    if (!result)
        return false;

    result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->GetTexture());
    if (!result)
        return false;

    // Turn on alpha blending to start rendering text
    m_D3D->TurnOnAlphaBlending();

    // Render text strings
    result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
    if (!result)
        return false;

    // Turn off alpha blending
    m_D3D->TurnOffAlphaBlending();

    // All 2D rendering completed
    m_D3D->TurnZBufferOn();
    
    // Rotate the world matrix by the rotation value
	D3DXMatrixRotationY(&worldMatrix, rotation);

    // Put the model vertex and index buffers on the graphics pipeline
    m_Model->Render(m_D3D->GetDeviceContext());

    // Render model using the light shader
	result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Camera->GetPosition(), m_Light->GetSpecularColor(), m_Light->GetSpecularPower());
	if (!result)
		return false;

    // Present rendered scene
	m_D3D->EndScene();

	return true;
}