---
trigger: always_on
---

# 🛠 Programming Convention Guide

본 문서는 안티그래비티(Antigravity) 프로젝트의 코드 가독성과 유지보수성을 높이기 위해 정의된 명명 규칙(Naming Convention) 가이드라인입니다.

---

## 1. 클래스 및 파일 명명 규칙 (Classes & Files)

클래스 이름과 물리적 파일 이름의 명명 규칙을 명확히 구분합니다.

* **클래스 이름 (Class Name):** 대문자 **`C`**를 접두어로 사용합니다.
    * **규칙:** `C` + `PascalCase`
    * **예시:** `CPlayerManager`, `CNetworkSession`, `CWeaponSystem`
* **파일 이름 (File Name):** 클래스 접두어 `C`를 **제외**하고 작성합니다.
    * **규칙:** `PascalCase` (파일 확장자와 결합)
    * **예시:** `PlayerManager.h / .cpp`, `NetworkSession.h / .cpp`

---

## 2. 멤버 변수 명명 규칙 (Member Variables)

클래스의 멤버 변수(Field)는 **`m_`** 접두어를 사용하며, 데이터 타입에 맞는 헝가리안 표기법 접두어를 결합합니다.

* **규칙:** `m_` + `Prefix` + `Name`
* **예시:** `m_nHealth`, `m_fDeltaTime`, `m_pOwner`

---

## 3. 헝가리안 표기법 접두어 (Hungarian Notation)

변수의 역할을 명확히 하기 위해 사용하는 표준 접두어입니다.

| 접두어 | 의미 (Type/Usage) | 예시 |
| :--- | :--- | :--- |
| **`n`** | Integer (정수형) | `nCount`, `nIndex` |
| **`f`** | Float (실수형) | `fSpeed`, `fDistance` |
| **`b`** | Boolean (논리형) | `bIsActive`, `bEnable` |
| **`p`** | Pointer (포인터) | `pBuffer`, `pTarget` |
| **`sz`** | String / Char Array | `szName`, `szPath` |
| **`str`** | String Object (std::string) | `strMessage` |
| **`h`** | Handle (핸들) | `hWindow`, `hTexture` |

---

## 4. 적용 예시 (Sample Code)

```cpp
class CCharacterBase
{
private:
    int     m_nLevel;        // 정수형 멤버 변수
    float   m_fWalkSpeed;    // 실수형 멤버 변수
    bool    m_bIsDead;       // 논리형 멤버 변수
    CItem* m_pEquippedItem; // 클래스 포인터 멤버 변수

public:
    void Update()
    {
        float fCurrentTime = GetTime(); // 지역 변수 (m_ 없음)
        // ... logic
    }
};