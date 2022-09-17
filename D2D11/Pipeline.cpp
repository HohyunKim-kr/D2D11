#include <d3d11.h>
#include <cassert>
#include "FreeImage.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "FreeImage.lib")

#if not defined _DEBUG
#define MUST(Expression) (      (         (Expression)))
#else
#define MUST(Expression) (assert(SUCCEEDED(Expression)))
#endif

namespace Input
{
    void Procedure(HWND const, UINT const, WPARAM const, LPARAM const);
    namespace Get
    {
        bool Down(size_t);
    }
}

namespace Pipeline
{
    namespace
    {
        ID3D11Device* Device;
        ID3D11DeviceContext* DeviceContext;
        IDXGISwapChain* SwapChain;
        ID3D11RenderTargetView* RenderTargetView;
        ID3D11InputLayout* InputLayout;
        ID3D11VertexShader* VertexShader;
        ID3D11PixelShader* PixelShader;

        namespace Buffer
        {
            ID3D11Buffer* Vertex;
            ID3D11Buffer* Constant[3];

            template<typename Data>
            void Update(ID3D11Buffer* buffer, Data const& data)
            {
                D3D11_MAPPED_SUBRESOURCE subResource = D3D11_MAPPED_SUBRESOURCE();

                MUST(DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource));
                memcpy_s(subResource.pData, subResource.RowPitch, data, sizeof(data));
                DeviceContext->Unmap(buffer, 0);
            }
        }
    }

    LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParam, LPARAM const lParam)
    {
        switch (uMessage)
        {
        case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL: case WM_MOUSEMOVE:
        case WM_SYSKEYDOWN: case WM_LBUTTONDOWN: case WM_LBUTTONUP:
        case WM_SYSKEYUP: case WM_RBUTTONDOWN: case WM_RBUTTONUP:
        case WM_KEYUP: case WM_MBUTTONDOWN: case WM_MBUTTONUP:
        case WM_KEYDOWN: case WM_XBUTTONDOWN: case WM_XBUTTONUP:
        {
            Input::Procedure(hWindow, uMessage, wParam, lParam);
            return 0;
        }
        case WM_CREATE:
        {
            {
                DXGI_SWAP_CHAIN_DESC Descriptor = DXGI_SWAP_CHAIN_DESC();
                Descriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                Descriptor.BufferCount = 1;
                Descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                Descriptor.OutputWindow = hWindow;
                Descriptor.SampleDesc.Count = 1;
                Descriptor.Windowed = true;

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

                D3D11_SUBRESOURCE_DATA SubResource{ Coordinates };

                ID3D11Buffer* Buffer = nullptr;

                MUST(Device->CreateBuffer(&Descriptor, &SubResource, &Buffer));

                const UINT Stride = sizeof(*Coordinates);
                const UINT Offset = 0;

                DeviceContext->IASetVertexBuffers(0, 1, &Buffer, &Stride, &Offset);

                Buffer->Release();
            }
            {
                D3D11_BUFFER_DESC Descriptor
                {
                    sizeof(float[4][2]),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_VERTEX_BUFFER,
                    D3D11_CPU_ACCESS_WRITE
                };

                MUST(Device->CreateBuffer(&Descriptor, nullptr, &Buffer::Vertex));

                const UINT Stride = sizeof(float[2]);
                const UINT Offset = 0;

                DeviceContext->IASetVertexBuffers(1, 1, &Buffer::Vertex, &Stride, &Offset);
            }
            {
                DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            }
            {
                D3D11_BUFFER_DESC Descriptor
                {
                    sizeof(float[4][4]),
                    D3D11_USAGE_DYNAMIC,
                    D3D11_BIND_CONSTANT_BUFFER,
                    D3D11_CPU_ACCESS_WRITE
                };

                for (UINT8 i = 0; i < 3; ++i)
                {
                    MUST(Device->CreateBuffer(&Descriptor, nullptr, &Buffer::Constant[i]));
                    DeviceContext->VSSetConstantBuffers(i, 1, &Buffer::Constant[i]);
                }
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
                    FIBITMAP* Bitmap = FreeImage_Load(FREE_IMAGE_FORMAT::FIF_PNG, "Player.png");
                    {
                        FreeImage_FlipVertical(Bitmap);

                        D3D11_TEXTURE2D_DESC Descriptor = D3D11_TEXTURE2D_DESC();
                        Descriptor.Width = FreeImage_GetWidth(Bitmap);
                        Descriptor.Height = FreeImage_GetHeight(Bitmap);
                        Descriptor.MipLevels = 1;
                        Descriptor.ArraySize = 1;
                        Descriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                        Descriptor.SampleDesc.Count = 1;
                        Descriptor.Usage = D3D11_USAGE_IMMUTABLE;
                        Descriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                        D3D11_SUBRESOURCE_DATA SubResource = D3D11_SUBRESOURCE_DATA();
                        SubResource.pSysMem = FreeImage_GetBits(Bitmap);
                        SubResource.SysMemPitch = FreeImage_GetPitch(Bitmap);

                        ID3D11Texture2D* Texture2D = nullptr;

                        MUST(Device->CreateTexture2D(&Descriptor, &SubResource, &Texture2D));
                        {
                            ID3D11ShaderResourceView* ShaderResourceView = nullptr;
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
                Viewport.Width = LOWORD(lParam);
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
                ID3D11Texture2D* texture = nullptr;
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
            MUST(SwapChain->Present(0, 0));
            float Color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            DeviceContext->ClearRenderTargetView(RenderTargetView, Color);
            {
                static struct
                {
                    float const Width = 84;
                    float const Height = 120;
                }Frame;

                static unsigned Count = 0;
                static unsigned Motion = 12;
                static unsigned FPM = 700;

                float const Coordinates[4][2]
                {
                    { Frame.Width * (Count / FPM + 0), Frame.Height * 0 }, // 좌상단
                    { Frame.Width * (Count / FPM + 1), Frame.Height * 0 }, // 우상단
                    { Frame.Width * (Count / FPM + 0), Frame.Height * 1 }, // 좌하단
                    { Frame.Width * (Count / FPM + 1), Frame.Height * 1 }  // 우하단
                };

                Count += 1;

                if (FPM * Motion - 1 < Count) Count = 0;

                Buffer::Update(Buffer::Vertex, Coordinates);
            }
            {

                {
                    static float  Scale_W = 84;
                    static float  Scale_H = 120;
                    static float  Location_X = 0;
                    static float  Location_Y = 0;

                    if (Input::Get::Down('W')) Location_Y += 0.1f;
                    if (Input::Get::Down('S')) Location_Y -= 0.1f;
                    if (Input::Get::Down('A')) Location_X -= 0.1f;
                    if (Input::Get::Down('D')) Location_X += 0.1f;

                    float const World[4][4]
                    {
                           Scale_W,          0, 0, Location_X,
                                 0,    Scale_H, 0, Location_Y,
                                 0,          0, 1,          0,
                                 0,          0, 0,          1
                    };

                    float const View[4][4]
                    {
                        1, 0, 0, Location_X * -1,
                        0, 1, 0, Location_Y * -1,
                        0, 0, 1, 0,
                        0, 0, 0, 1
                    };

                    Buffer::Update(Buffer::Constant[0], World);
                    Buffer::Update(Buffer::Constant[1], View);
                }
                {
                    static float const X = 2.0f / 500.0f;
                    static float const Y = 2.0f / 500.0f;

                    float const Transform[4][4]
                    {
                        X, 0, 0, 0,
                        0, Y, 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1
                    };

                    Buffer::Update(Buffer::Constant[2], Transform);
                }
            }
            DeviceContext->Draw(4, 0);
            {
                static float  Scale_W = 84 / 3;
                static float  Scale_H = 120 / 3;

                static float  Location_X = 100;
                static float  Location_Y = 100;

                float const Transform[4][4]
                {
                       Scale_W,          0, 0, Location_X,
                             0,    Scale_H, 0, Location_Y,
                             0,          0, 1,          0,
                             0,          0, 0,          1
                };

                Buffer::Update(Buffer::Constant[0], Transform);
            }
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