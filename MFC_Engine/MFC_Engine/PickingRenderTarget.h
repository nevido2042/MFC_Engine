#pragma once


class CPickingRenderTarget {
public:
  CPickingRenderTarget();
  ~CPickingRenderTarget();

  void Initialize(ID3D12Device *device, int width, int height);
  void Resize(ID3D12Device *device, int width, int height);

  // Reads a pixel from a source texture (e.g. GBuffer[3]) to the readback buffer.
  void ReadPixelAsync(ID3D12GraphicsCommandList *commandList, int x, int y, ID3D12Resource* pSource);
  UINT GetPickedID();

private:
  ComPtr<ID3D12Resource> m_readbackBuffer;

  int m_width;
  int m_height;
};
