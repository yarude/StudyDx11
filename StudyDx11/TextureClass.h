////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <stdio.h>


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureClass
////////////////////////////////////////////////////////////////////////////////
class TextureClass
{
private:

	// tag�ļ�ͷ�ṹ
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	// ���벢����tag�ļ�
	bool LoadTarga(char*, int&, int&);

private:

	unsigned char* m_targaData; // tagԭʼ����
	ID3D11Texture2D* m_texture; // dx������Ⱦ����������
	ID3D11ShaderResourceView* m_textureView; // shader�ڻ���ʱʹ�õ�

};


#endif

