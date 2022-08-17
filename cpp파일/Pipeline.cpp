#include <d3d11.h>
#include <iostream>
#include <cassert>

#if not defined _DEBUG
#define MUST(Expression) (      (         (Expression)))
#else 
#define MUST(Expression) (assert(SUCCEEDED(Expression)))
#endif

namespace Pipeline
{
	// Rendering Pipeline
	// 화면에 그래픽이 그려지는 단계를 의미합니다.
	// IA (Input Assembler) Stage
	// VS (Vertex Shader)   Stage
	// HS (Hull Shader)     Stage
	// TS (Tessellator)     Stage
	// DS (Domain Shader)   Stage
	// GS (Geometry Shader) Stage
	// SO (Stream Output)   Stage
	// RS (Rasterizer)      Stage
	// PS (Pixel Shader)    Stage
	// OM (Output Merger)   Stage

	// IA -> VS -> RS -> PS -> OM

	// Input Assembler
	// 가장 기본적인 데이터를 입력하는 단계입니다.
	// 그래픽 연산의 가장 기초적인 데이터는 Vertex(정점)을 의미합니다.


	namespace
	{
		ID3D11Device *		    Device;        
		ID3D11DeviceContext*    DeviceContext; 
		IDXGISwapChain*         SwapChain;
		ID3D11RenderTargetView* RenderTargetView;
		ID3D11InputLayout*      InputLayout;
		ID3D11VertexShader*     VertexShader;

		namespace Buffer
		{ ID3D11Buffer * Vertex; }
	}

	LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParam, LPARAM const lParam)
	{
		switch(uMessage)
		{
			case WM_CREATE :
			{
				{
					DXGI_SWAP_CHAIN_DESC Descriptor = DXGI_SWAP_CHAIN_DESC();

					Descriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					Descriptor.SampleDesc.Count  = 1;
					Descriptor.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					Descriptor.BufferCount       = 1;
					Descriptor.OutputWindow      = hWindow;
					Descriptor.Windowed          = true;

					MUST((D3D11CreateDeviceAndSwapChain
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
					)));
				}

				{
					struct Vertex final
					{
						float Position[4];
						float    Color[4];
					};

					Vertex const Vertecies[4]
					{
					{ { -0.5f, +0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
					{ { +0.5f, +0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
					{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
					{ { +0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }
					};

					D3D11_BUFFER_DESC Descriptor = D3D11_BUFFER_DESC();

					Descriptor.ByteWidth           = sizeof(Vertecies);
					Descriptor.Usage               = D3D11_USAGE_IMMUTABLE;
					Descriptor.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
					Descriptor.CPUAccessFlags      = 0;
					Descriptor.MiscFlags           = 0;
					Descriptor.StructureByteStride = 0; 

					D3D11_SUBRESOURCE_DATA SubResource = D3D11_SUBRESOURCE_DATA();
					SubResource.pSysMem = Vertecies; 

					MUST(Device->CreateBuffer(&Descriptor, &SubResource, &Buffer::Vertex));

					const UINT Stride = sizeof(Vertex);
					const UINT Offset = sizeof(Vertex);

					DeviceContext->IASetVertexBuffers(0, 1,	&Buffer::Vertex, &Stride, &Offset);		
				}
				{
					// 정점을 어떻게 연결할 것인지를 설정합니다.
					DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				}
				{
					#include "Shader/Bytecode/Vertex.h"
					{
						D3D11_INPUT_ELEMENT_DESC Descriptor[2]
						{
							D3D11_INPUT_ELEMENT_DESC(),
							D3D11_INPUT_ELEMENT_DESC()
						};

						Descriptor[0].SemanticName;
						Descriptor[0].SemanticIndex;
						Descriptor[0].Format;
						Descriptor[0].InputSlot;
						Descriptor[0].AlignedByteOffset;
						Descriptor[0].InputSlotClass;

						Descriptor[1].SemanticName;
						Descriptor[1].SemanticIndex;
						Descriptor[1].Format;
						Descriptor[1].InputSlot;
						Descriptor[1].AlignedByteOffset;
						Descriptor[1].InputSlotClass;

						MUST(Device->CreateInputLayout(Descriptor, 2, Bytecode, sizeof(Bytecode), &InputLayout));

						DeviceContext->IASetInputLayout(InputLayout);
					}
					{
						MUST(Device->CreateVertexShader(Bytecode, sizeof(Bytecode), nullptr, &VertexShader));
						DeviceContext->VSSetShader(VertexShader, nullptr, 0);
					}
				}
				return 0;
			}
			case WM_APP :
			{
				MUST(SwapChain->Present(0, 0));

				static float a = 0;
				static float delta = 0.001f;

				a += delta;

				float Color[4] = { 0.0f, a, 0.0f, 1.0f };

				if (a >= 1.0f || a <= 0.0f) delta *= -1.0f;

				DeviceContext->ClearRenderTargetView(RenderTargetView, Color);

				return 0;
			}
			case WM_DESTROY :
			{
				PostQuitMessage(0);

				return 0;
			}
			case WM_SIZE :
			{
				{
					D3D11_VIEWPORT Viewport = D3D11_VIEWPORT();
					Viewport.Width  = LOWORD(lParam);
					Viewport.Height = HIWORD(lParam);

					DeviceContext->RSSetViewports(1, &Viewport);
				}
				{
					if (RenderTargetView != nullptr)
					{
						RenderTargetView->Release();

						MUST(SwapChain->ResizeBuffers
						(
							1,
							LOWORD(lParam),
							HIWORD(lParam),
							DXGI_FORMAT_R8G8B8A8_UNORM,
							0
						));
					}
				}
				{
					ID3D11Texture2D * Texture2D = nullptr;

					MUST(SwapChain->GetBuffer(0, IID_PPV_ARGS(&Texture2D)));
					{
						MUST(Device->CreateRenderTargetView(Texture2D, nullptr, &RenderTargetView));
					}
					Texture2D->Release();

					DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);			
				}
				return 0;
			}
			default :
			{ return DefWindowProc(hWindow, uMessage, wParam, lParam); }
		}
	}
}