namespace Layout
{
	struct Vertex
	{
		// Sementic : HLSL ���� �������� �����̳� �ǹ̸� �ο��ϴ� ����Դϴ�.
		//            ��ǻ�ʹ� �ش� �ø�ƽ ������ ���� � �������� Ȯ���� �����մϴ�.
		float4 Position : POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	struct Pixel
	{
		// SV : System Value �� ��ǻ�Ͱ� ó���� �����Ͷ�� ���� ����ϴ� �ø�ƽ�����Դϴ�.
		float4 Position : SV_POSITION;
		float4 Texcoord : TEXCOORD;
	};
	
	typedef float4 Color;
}