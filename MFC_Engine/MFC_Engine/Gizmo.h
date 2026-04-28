#pragma once

#include "pch.h"

class CGameObject;
class CConstantBuffer;

class CGizmo {
public:
  CGizmo();
  ~CGizmo();

  void Initialize(ID3D12Device *pDevice);
  ID3D12RootSignature* GetRootSignature() const { return nullptr; }

  // 일반 렌더링용
  void Render(ID3D12GraphicsCommandList *pCommandList,
              ID3D12RootSignature* pRootSignature,
              CGameObject *pSelectedObj, int nWidth, int nHeight,
              CConstantBuffer *pCB);

  // 피킹용 렌더링
  void RenderForPicking(ID3D12GraphicsCommandList *pCommandList,
                        CGameObject *pSelectedObj, int nWidth, int nHeight,
                        ID3D12Resource *pConstantBuffer, UINT8 *pCbvDataBegin);

private:
  struct AxisInfo 
  {
    DirectX::XMFLOAT3 vScale;
    DirectX::XMFLOAT3 vOffset;
    DirectX::XMFLOAT4 vColor;
    UINT nPickedID;
  };

  void DrawAxes(ID3D12GraphicsCommandList *pCommandList,
                CGameObject *pSelectedObj, int nWidth, int nHeight,
                bool bForPicking, CConstantBuffer *pCB, ID3D12Resource *pRawCB,
                UINT8 *pRawData);

private:
  Microsoft::WRL::ComPtr<ID3D12Device> m_pDevice;
};
