////////////////////////////////////////////////////////////////////////////////
  // Filename: color.vs
  ////////////////////////////////////////////////////////////////////////////////

  /////////////
  // GLOBALS //
  /////////////
  // ���ȶ���shader�е�ȫ�ֱ�������Щ��������c++��������ֵ����������ͳһ������һ��buffer�ṹ��
  cbuffer MatrixBuffer            // cbuffer�ṹ
  {
      matrix worldMatrix;
      matrix viewMatrix;
      matrix projectionMatrix;
  };

  // ������������ṹ
  //////////////
  // TYPEDEFS //
  //////////////
  // �ýṹ��Ҫ����C++�ж����Vertex Buffer�е����ݸ�ʽ
  struct VertexInputType
  {
       // POSITIONΪ���壬����GPU����������ñ���POSITION�Ǹ�������ɫ���õ�
      float4 position : POSITION;
      float4 color : COLOR;       // �����Ҫ�����ɫ����ʹ��COLOR0��COLOR1����
  };

  struct PixelInputType
  {
      float4 position : SV_POSITION; // �����pixel shader��
      float4 color : COLOR;
  };

  ////////////////////////////////////////////////////////////////////////////////
  // Vertex Shader
  ////////////////////////////////////////////////////////////////////////////////
  PixelInputType ColorVertexShader(VertexInputType input)
  {
      PixelInputType output;
      

      // Change the position vector to be 4 units for proper matrix calculations.
      input.position.w = 1.0f;

      // Calculate the position of the vertex against the world, view, and projection matrices.
      output.position = mul(input.position, worldMatrix);
      output.position = mul(output.position, viewMatrix);
      output.position = mul(output.position, projectionMatrix);
      
      // Store the input color for the pixel shader to use.
      output.color = input.color;
      
      return output;
  }