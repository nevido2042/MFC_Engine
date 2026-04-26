#pragma once
#include <memory>
#include "Scene.h"

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

private:
	CSceneManager() {
		m_pActiveScene = std::make_shared<CScene>();
		m_pActiveScene->SetName(L"SampleScene");
	}
	~CSceneManager() {}

	std::shared_ptr<CScene> m_pActiveScene;
};
