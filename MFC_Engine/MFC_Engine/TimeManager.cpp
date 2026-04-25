#include "pch.h"
#include "TimeManager.h"

CTimeManager::CTimeManager()
    : m_deltaTime(0.0)
    , m_totalTime(0.0)
    , m_fps(0.0f)
    , m_frameCount(0)
    , m_fpsTimer(0.0)
{
    m_timerFrequency.QuadPart = 0;
    m_lastTimestamp.QuadPart = 0;
}

CTimeManager::~CTimeManager()
{
}

void CTimeManager::Initialize()
{
    QueryPerformanceFrequency(&m_timerFrequency);
    QueryPerformanceCounter(&m_lastTimestamp);
}

void CTimeManager::Update()
{
    LARGE_INTEGER currentTimestamp;
    QueryPerformanceCounter(&currentTimestamp);

    // DeltaTime 계산 (초 단위)
    m_deltaTime = (double)(currentTimestamp.QuadPart - m_lastTimestamp.QuadPart) / m_timerFrequency.QuadPart;
    m_lastTimestamp = currentTimestamp;

    // 누적 시간 및 FPS 계산
    m_totalTime += m_deltaTime;
    m_fpsTimer += m_deltaTime;
    m_frameCount++;

    if (m_fpsTimer >= 1.0)
    {
        m_fps = (float)m_frameCount / (float)m_fpsTimer;
        m_frameCount = 0;
        m_fpsTimer = 0.0;
    }
}
