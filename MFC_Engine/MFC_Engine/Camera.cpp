#include "pch.h"
#include "Camera.h"

CCamera::CCamera()
    : m_cameraPos(0.0f, 2.0f, -5.0f)
    , m_cameraPitch(0.0f)
    , m_cameraYaw(0.0f)
{
}

CCamera::~CCamera()
{
}

void CCamera::Move(float forward, float right, float up, float deltaTime)
{
    float speed = 5.0f * deltaTime;

    // 현재 회전 상태를 바탕으로 이동 방향 계산
    DirectX::XMMATRIX matCamRot = DirectX::XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);

    DirectX::XMVECTOR camForward = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), matCamRot);
    DirectX::XMVECTOR camRight   = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), matCamRot);
    DirectX::XMVECTOR camUp      = DirectX::XMVectorSet(0, 1, 0, 0); // 월드 Up 기준 고정

    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_cameraPos);

    // 이동 벡터 계산 및 합산
    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(camForward, forward * speed));
    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(camRight, right * speed));
    pos = DirectX::XMVectorAdd(pos, DirectX::XMVectorScale(camUp, up * speed));

    DirectX::XMStoreFloat3(&m_cameraPos, pos);
}

void CCamera::Rotate(float pitch, float yaw)
{
    float sensitivity = 0.005f;

    m_cameraPitch += pitch * sensitivity;
    m_cameraYaw   += yaw * sensitivity;

    // Pitch 제한 (고개 꺾임 방지)
    const float limit = DirectX::XM_PIDIV2 - 0.1f;
    if (m_cameraPitch > limit)  m_cameraPitch = limit;
    if (m_cameraPitch < -limit) m_cameraPitch = -limit;
}

DirectX::XMMATRIX CCamera::GetViewMatrix() const
{
    // 회전 행렬 생성
    DirectX::XMMATRIX matCamRot = DirectX::XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);

    // 위치 벡터 로드
    DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&m_cameraPos);

    // 회전된 방향 벡터 계산
    DirectX::XMVECTOR camForward = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), matCamRot);
    DirectX::XMVECTOR camUp      = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), matCamRot);

    // LookAt 행렬 생성 (Left-Handed)
    return DirectX::XMMatrixLookToLH(camPos, camForward, camUp);
}

DirectX::XMVECTOR CCamera::GetForward() const
{
    DirectX::XMMATRIX matCamRot = DirectX::XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
    return DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), matCamRot);
}

DirectX::XMVECTOR CCamera::GetRight() const
{
    DirectX::XMMATRIX matCamRot = DirectX::XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
    return DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), matCamRot);
}

DirectX::XMVECTOR CCamera::GetUp() const
{
    DirectX::XMMATRIX matCamRot = DirectX::XMMatrixRotationRollPitchYaw(m_cameraPitch, m_cameraYaw, 0.0f);
    return DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), matCamRot);
}

DirectX::XMMATRIX CCamera::GetProjectionMatrix(float aspectRatio) const
{
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 1000.0f);
}
