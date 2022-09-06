#include "Layout.hlsli"

namespace Shader
{
	const Texture2D Resource : register(T0);
	
	Layout::Color Pixel(const Layout::Pixel Input) : SV_TARGET
	{
		Layout::Color Output = 
		{
			Resource.Load(int3(Input.Texcoord.x, Input.Texcoord.y, 0))
		};
	
		return Output;
	}
}