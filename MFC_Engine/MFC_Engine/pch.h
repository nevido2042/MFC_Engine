// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

// 여기에 미리 컴파일하려는 헤더 추가
#include "framework.h"

// DirectX 12 관련 헤더
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft::WRL;
using namespace DirectX;

#include <exception>
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <map>
#include "d3dx12.h"

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        CString strError;
        strError.Format(_T("DirectX 12 Error: 0x%08X"), hr);
        AfxMessageBox(strError, MB_ICONERROR);
        throw std::exception();
    }
}

#endif //PCH_H
