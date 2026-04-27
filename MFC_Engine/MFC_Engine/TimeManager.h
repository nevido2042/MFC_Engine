#pragma once

/**
 * @class CTimeManager
 * @brief 엔진의 시간 측정, DeltaTime 계산 및 FPS 관리를 담당하는 클래스입니다.
 */
class CTimeManager
{
public:
    CTimeManager();
    ~CTimeManager();

    /** @brief 타이머 초기화 */
    void Initialize();
    
    /** @brief 매 프레임 호출되어 시간을 갱신 */
    void Update();

    /** @brief 현재 FPS 반환 */
    float GetFPS() const { return m_fFPS; }
    
    /** @brief 프레임 사이의 경과 시간(DeltaTime) 반환 */
    float GetDeltaTime() const { return (float)m_fDeltaTime; }
    
    /** @brief 엔진 시작 후 총 누적 시간 반환 */
    float GetTotalTime() const { return (float)m_fTotalTime; }

private:
    LARGE_INTEGER m_nTimerFrequency;     // 타이머 주파수
    LARGE_INTEGER m_nLastTimestamp;      // 이전 프레임 타임스탬프
    
    double m_fDeltaTime;                 // 프레임 간 경과 시간
    double m_fTotalTime;                 // 누적 시간
    
    float m_fFPS;                        // 계산된 FPS
    int m_nFrameCount;                   // 프레임 카운트
    double m_fFPSTimer;                  // FPS 갱신용 타이머
};
