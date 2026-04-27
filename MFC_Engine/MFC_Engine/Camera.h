#pragma once
#include <DirectXMath.h>

/**
 * @class CCamera
 * @brief 카메라의 위치, 회전 및 행렬 계산을 담당하는 클래스입니다.
 */
class CCamera
{
public:
    CCamera();
    ~CCamera();

    /**
     * @brief 카메라를 이동시킵니다.
     * @param forward 앞뒤 이동 계수
     * @param right 좌우 이동 계수
     * @param up 상하 이동 계수
     * @param deltaTime 프레임 간격
     */
    void Move(float forward, float right, float up, float deltaTime);

    /**
     * @brief 카메라를 회전시킵니다.
     * @param pitch 상하 회전 (라디안)
     * @param yaw 좌우 회전 (라디안)
     */
    void Rotate(float pitch, float yaw);

    /**
     * @brief 현재 카메라 상태를 바탕으로 뷰 행렬을 반환합니다.
     */
    DirectX::XMMATRIX GetViewMatrix() const;

    // Getter
    DirectX::XMFLOAT3 GetPosition() const { return m_cameraPos; }
    float GetPitch() const { return m_cameraPitch; }
    float GetYaw() const { return m_cameraYaw; }

    DirectX::XMVECTOR GetForward() const;
    DirectX::XMVECTOR GetRight() const;
    DirectX::XMVECTOR GetUp() const;

private:
    DirectX::XMFLOAT3 m_cameraPos; // 카메라 위치
    float m_cameraPitch;          // 상하 회전 값
    float m_cameraYaw;            // 좌우 회전 값
};
