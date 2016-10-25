#include "graphicsclass.h"
#include "cameraclass.h"

GraphicsClass::GraphicsClass() {
    m_D3D = 0;
    m_Camera = 0;
}

GraphicsClass::GraphicsClass(const GraphicsClass& other) {

}

GraphicsClass::~GraphicsClass() {

}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {
    bool result;

    // Create the Direct3D object
    m_D3D = new D3DClass;
    if (!m_D3D) {
        return false;
    }

    // Init the Direct3D object
    result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
        MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
        return false;
    }

    // Create camera object
    m_Camera = new CameraClass;
    if (!m_Camera) {
        return false;
    }

    // Set initial position of camera
    m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

    return true;
}

void GraphicsClass::Shutdown() {
    if (m_Camera) {
        delete m_Camera;
        m_Camera = 0;
    }
    if (m_D3D) {
        m_D3D->Shutdown();
        delete m_D3D;
        m_D3D = 0;
    }
}

bool GraphicsClass::Frame() {
    bool result;
    static float rotation = 0.0f;

    // Update the rotation variable each frame
    rotation += (float)D3DX_PI * 0.005f;
    if (rotation > 360.0f) {
        rotation -= 360;
    }

    // Render the graphics scene
    result = Render(rotation);
    if (!result) {
        return false;
    }

    return true;
}

bool GraphicsClass::Render(float rotation) {
    D3DXMATRIX viewMatrix, projectionMatrix, worldMatrix;
    bool result;

    // Clear buffers
    m_D3D->BeginScene(0.0f, 0.5f, 0.5f, 1.0f);

    // Generate the view matrix based on camera's position
    m_Camera->Render();

    m_Camera->GetViewMatrix(viewMatrix);
    m_D3D->GetWorldMatrix(worldMatrix);
    m_D3D->GetProjectionMatrix(projectionMatrix);

    // Rotate the world matrix by the rotation value
    D3DXMatrixRotationY(&worldMatrix, rotation);

    // Present rendered scene
    m_D3D->EndScene();

    return true;
}