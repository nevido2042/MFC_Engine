#pragma once
#include <memory>
#include "Scene.h"
#include "Camera.h"
#include <mutex>

class CSceneManager
{
public:
	static CSceneManager& GetInstance()
	{
		static CSceneManager instance;
		return instance;
	}

	void SetActiveScene(std::shared_ptr<CScene> pScene) { m_pActiveScene = pScene; }
	std::shared_ptr<CScene> GetActiveScene() { return m_pActiveScene; }

	CCamera& GetEditorCamera() { return m_editorCamera; }
	std::mutex& GetCameraMutex() { return m_cameraMutex; }

private:
	CSceneManager() {
		m_pActiveScene = std::make_shared<CScene>();
		m_pActiveScene->SetName(L"SampleScene");
	}
	~CSceneManager() {}

	std::shared_ptr<CScene> m_pActiveScene;
	CCamera m_editorCamera;
	std::mutex m_cameraMutex;
};
