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
    float GetFPS() const { return m_fps; }
    
    /** @brief 프레임 사이의 경과 시간(DeltaTime) 반환 */
    float GetDeltaTime() const { return (float)m_deltaTime; }
    
    /** @brief 엔진 시작 후 총 누적 시간 반환 */
    float GetTotalTime() const { return (float)m_totalTime; }

private:
    LARGE_INTEGER m_timerFrequency;     // 타이머 주파수
    LARGE_INTEGER m_lastTimestamp;      // 이전 프레임 타임스탬프
    
    double m_deltaTime;                 // 프레임 간 경과 시간
    double m_totalTime;                 // 누적 시간
    
    float m_fps;                        // 계산된 FPS
    int m_frameCount;                   // 프레임 카운트
    double m_fpsTimer;                  // FPS 갱신용 타이머
};
