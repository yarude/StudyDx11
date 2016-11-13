////////////////////////////////////////////////////////////////////////////////
  // Filename: color.vs
  ////////////////////////////////////////////////////////////////////////////////

  /////////////
  // GLOBALS //
  /////////////
  // 首先定义shader中的全局变量，这些变量将有c++程序来赋值，变量建议统一定义在一个buffer结构里
  cbuffer MatrixBuffer            // cbuffer结构
  {
      matrix worldMatrix;
      matrix viewMatrix;
      matrix projectionMatrix;
  };

  // 定义输入输出结构
  //////////////
  // TYPEDEFS //
  //////////////
  // 该结构需要符合C++中定义的Vertex Buffer中的数据格式
  struct VertexInputType
  {
       // POSITION为语义，告诉GPU将怎样处理该变量POSITION是给顶点着色器用的
      float4 position : POSITION;
      float4 color : COLOR;       // 如果需要多个颜色可以使用COLOR0，COLOR1语义
  };

  struct PixelInputType
  {
      float4 position : SV_POSITION; // 输出给pixel shader的
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