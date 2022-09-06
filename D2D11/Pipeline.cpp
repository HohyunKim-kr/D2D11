#include <d3d11.h>
#include <cassert>
#include "FreeImage.h"

// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "FreeImage.lib")

#if not defined _DEBUG
#define MUST(Expression) (      (         (Expression)))
#else
#define MUST(Expression) (assert(SUCCEEDED(Expression)))
#endif


namespace Pipeline
{
    namespace
    {
        ID3D11Device*           Device;
        ID3D11DeviceContext*    DeviceContext;
        IDXGISwapChain*         SwapChain;
        ID3D11RenderTargetView* RenderTargetView;
        ID3D11InputLayout*      InputLayout;
        ID3D11VertexShader*     VertexShader;
        ID3D11PixelShader*      PixelShader;

        namespace Buffer
        { ID3D11Buffer * Vertex; }
    }

    LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParam, LPARAM const lParam)
    {
        switch (uMessage)
        {
            case WM_CREATE: 
            {
                {
                    DXGI_SWAP_CHAIN_DESC Descriptor = DXGI_SWAP_CHAIN_DESC();
                    Descriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    Descriptor.BufferCount       = 1;
                    Descriptor.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    Descriptor.OutputWindow      = hWindow;
                    Descriptor.SampleDesc.Count  = 1;
                    Descriptor.Windowed          = true;
                                                 
                    MUST(D3D11CreateDeviceAndSwapChain
                    (
                        nullptr,
                        D3D_DRIVER_TYPE_HARDWARE,
                        nullptr,
                        0,
                        nullptr,
                        0,
                        D3D11_SDK_VERSION,
                        &Descriptor,
                        &SwapChain,
                        &Device,
                        nullptr,
                        &DeviceContext
                    ));
                }

                {
                    float const Coordinates[4][2]
                    {
                      { -0.5f, +0.5f }, 
                      { +0.5f, +0.5f }, 
                      { -0.5f, -0.5f }, 
                      { +0.5f, -0.5f }, 
                    };
             
                    D3D11_BUFFER_DESC Descriptor
                    { 
                        sizeof(Coordinates),
                        D3D11_USAGE_IMMUTABLE, 
                        D3D11_BIND_VERTEX_BUFFER 
                    };

                    D3D11_SUBRESOURCE_DATA SubResource {Coordinates};
           
                    ID3D11Buffer * Buffer = nullptr;

                    MUST(Device->CreateBuffer(&Descriptor, &SubResource, &Buffer));

                    const UINT Stride = sizeof(*Coordinates);
                    const UINT Offset = 0;

                    DeviceContext->IASetVertexBuffers(0, 1, &Buffer, &Stride, &Offset);

                    Buffer->Release();
                }
                {
                    float const Texcoord[4][2]
                    {
                        {   0.0f,   0.0f },
                        { 250.0f,   0.0f },
                        {   0.0f, 250.0f },
                        { 250.0f, 250.0f }
                    };

                    D3D11_BUFFER_DESC Descriptor
                    {
                        sizeof(Texcoord),
                        D3D11_USAGE_IMMUTABLE,
                        D3D11_BIND_VERTEX_BUFFER
                    };

                    D3D11_SUBRESOURCE_DATA SubResource{ Texcoord };

                    MUST(Device->CreateBuffer(&Descriptor, &SubResource, &Buffer::Vertex));

                    const UINT Stride = sizeof(*Texcoord);
                    const UINT Offset = 0;

                    DeviceContext->IASetVertexBuffers(1, 1, &Buffer::Vertex, &Stride, &Offset);
                }
                {
                    DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                }
                {
#include "Shader/Bytecode/Vertex.h"
                    {
                        D3D11_INPUT_ELEMENT_DESC Descriptor[2]
                        {
                            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0 },
                            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1 }
                        };
                        
                        MUST(Device->CreateInputLayout(Descriptor, 2, ByteCode, sizeof(ByteCode), &InputLayout));
                        DeviceContext->IASetInputLayout(InputLayout);

                    }
                    {
                        MUST(Device->CreateVertexShader(ByteCode, sizeof(ByteCode), nullptr, &VertexShader));
                        DeviceContext->VSSetShader(VertexShader, nullptr, 0);
                    }
                }
                {
#include "Shader/Bytecode/Pixel.h"
                    MUST(Device->CreatePixelShader(ByteCode, sizeof(ByteCode), nullptr, &PixelShader));
                    DeviceContext->PSSetShader(PixelShader, nullptr, 0);
                }
                {
                    FreeImage_Initialise();
                    {
                        FIBITMAP * Bitmap = FreeImage_Load(FREE_IMAGE_FORMAT::FIF_PNG, "Free.png");
                        {
                            FreeImage_FlipVertical(Bitmap);

                            D3D11_TEXTURE2D_DESC Descriptor = D3D11_TEXTURE2D_DESC();
                            Descriptor.Width            = FreeImage_GetWidth(Bitmap);
                            Descriptor.Height           = FreeImage_GetHeight(Bitmap);
                            Descriptor.MipLevels        = 1;
                            Descriptor.ArraySize        = 1;
                            Descriptor.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
                            Descriptor.SampleDesc.Count = 1;
                            Descriptor.Usage            = D3D11_USAGE_IMMUTABLE;
                            Descriptor.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

                            D3D11_SUBRESOURCE_DATA SubResource = D3D11_SUBRESOURCE_DATA();
                            SubResource.pSysMem     = FreeImage_GetBits(Bitmap);
                            SubResource.SysMemPitch = FreeImage_GetPitch(Bitmap);

                            ID3D11Texture2D * Texture2D = nullptr;

                            MUST(Device->CreateTexture2D(&Descriptor, &SubResource, &Texture2D));
                            {
                                ID3D11ShaderResourceView * ShaderResourceView = nullptr;
                                MUST(Device->CreateShaderResourceView(Texture2D, nullptr, &ShaderResourceView));
                                {
                                    DeviceContext->PSSetShaderResources(0, 1, &ShaderResourceView);
                                }
                                ShaderResourceView->Release();
                            }
                            Texture2D->Release();
                        }
                        FreeImage_Unload(Bitmap);
                    }
                    FreeImage_DeInitialise();

                }
                return 0;
            }
            case WM_SIZE:    
            {
                {
                    D3D11_VIEWPORT Viewport = D3D11_VIEWPORT();
                    Viewport.Width  = LOWORD(lParam);
                    Viewport.Height = HIWORD(lParam);

                    DeviceContext->RSSetViewports(1, &Viewport);
                }
                {
                    MUST(SwapChain->ResizeBuffers
                    (
                        1,
                        LOWORD(lParam),
                        HIWORD(lParam),
                        DXGI_FORMAT_B8G8R8A8_UNORM,
                        0
                    ));
                }
                {
                    ID3D11Texture2D * texture = nullptr;
                    MUST(SwapChain->GetBuffer(0, IID_PPV_ARGS(&texture)));
                    {
                        Device->CreateRenderTargetView(texture, nullptr, &RenderTargetView);
                    }
                    texture->Release();

                    DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
                }
                return 0;
            }
            case WM_APP:      
            {
                MUST(SwapChain->Present(1, 0));

                static float     a =      0;
                static float delta = 0.01f;

                a += delta; 

                float Color[4] = { 1.0f, 1.0f, a, 1.0f };

                if (a >= 1.0f || a <= 0.0f) delta *= -1.0f;

                DeviceContext->ClearRenderTargetView(RenderTargetView, Color);
                DeviceContext->Draw(4, 0);
                return 0;
            }
            case WM_DESTROY:
            {
                Buffer::Vertex->Release();
                InputLayout->Release();
                VertexShader->Release();
                PixelShader->Release();
                RenderTargetView->Release();
                Device->Release();
                DeviceContext->Release();
                SwapChain->Release();

                PostQuitMessage(0);
                return 0;
            }
            default:
            { return DefWindowProc(hWindow, uMessage, wParam, lParam); }
        }
    }
}