#include "pch.h"
#include "TimeManager.h"

CTimeManager::CTimeManager()
    : m_fDeltaTime(0.0)
    , m_fTotalTime(0.0)
    , m_fFPS(0.0f)
    , m_nFrameCount(0)
    , m_fFPSTimer(0.0)
{
    m_nTimerFrequency.QuadPart = 0;
    m_nLastTimestamp.QuadPart = 0;
}

CTimeManager::~CTimeManager()
{
}

void CTimeManager::Initialize()
{
    QueryPerformanceFrequency(&m_nTimerFrequency);
    QueryPerformanceCounter(&m_nLastTimestamp);
}

void CTimeManager::Update()
{
    LARGE_INTEGER currentTimestamp;
    QueryPerformanceCounter(&currentTimestamp);

    // DeltaTime 계산 (초 단위)
    m_fDeltaTime = (double)(currentTimestamp.QuadPart - m_nLastTimestamp.QuadPart) / m_nTimerFrequency.QuadPart;
    m_nLastTimestamp = currentTimestamp;

    // 누적 시간 및 FPS 계산
    m_fTotalTime += m_fDeltaTime;
    m_fFPSTimer += m_fDeltaTime;
    m_nFrameCount++;

    if (m_fFPSTimer >= 1.0)
    {
        m_fFPS = (float)m_nFrameCount / (float)m_fFPSTimer;
        m_nFrameCount = 0;
        m_fFPSTimer = 0.0;
    }
}
