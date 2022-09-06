namespace Layout
{
	struct Vertex
	{
		// Sementic : HLSL 에서 데이터의 역할이나 의미를 부여하는 기능입니다.
		//            컴퓨터는 해당 시멘틱 네임을 통해 어떤 역할인지 확인이 가능합니다.
		float4 Position : POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	struct Pixel
	{
		// SV : System Value 로 컴퓨터가 처리할 데이터라는 것을 명시하는 시멘틱네임입니다.
		float4 Position : SV_POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	typedef float4 Color;
}