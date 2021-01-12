#ifndef STEERINGBEHAVIORS_APPLICATION_H
#define STEERINGBEHAVIORS_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "../SteeringBehaviors.h"

class SteeringAgent;
#include "Flock.h"

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Flocking final : public IApp
{
public:
	//Constructor & Destructor
	App_Flocking();
	virtual ~App_Flocking();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	TargetData m_MouseTargetLclick = {};
	TargetData m_MouseTargetRclick = {};
	bool m_UseMouseTarget = true;
	bool m_VisualizeMouseTarget = true;
	float m_TrimWorldSize = 200.f;

	SteeringAgent* m_pAgentToEvade = nullptr;

	float m_SpawnWidth = { (m_TrimWorldSize / 5.f) };
	float m_SpawnHeight = { m_TrimWorldSize / 3.f };

	Elite::Rect m_BlueSpawnZone = { {m_TrimWorldSize / 8.f, (m_TrimWorldSize / 2.f) - (m_SpawnHeight / 2.f)},m_SpawnWidth, m_SpawnHeight };
	Elite::Rect m_RedSpawnZone = { {(m_TrimWorldSize * 7.f / 8.f) - m_SpawnWidth, (m_TrimWorldSize / 2.f) - (m_SpawnHeight / 2.f)}, m_SpawnWidth, m_SpawnHeight };
		/*//because both spawn zones have the same dimension they are just general
	//
	
	m_SpawnWidth = blueSpawnZone.width;//{ worldSize / 5.f };
	m_SpawnHeight = blueSpawnZone.height;//{ worldSize / 3.f };

	//m_BlueSpawnZone = { {worldSize / 8.f, (worldSize / 2.f) - (m_SpawnHeight / 2.f)},m_SpawnWidth, m_SpawnHeight };
	//m_RedSpawnZone = { {(worldSize * 7.f / 8.f) - m_SpawnWidth, (worldSize / 2.f) - (m_SpawnHeight / 2.f)}, m_SpawnWidth, m_SpawnHeight };*/

	//flock object but now used as a mini battle simulator
	Flock m_Flock;// {m_TrimWorldSize, m_pAgentToEvade, true };

	//C++ make the class non-copyable
	App_Flocking(const App_Flocking&) = delete;
	App_Flocking& operator=(const App_Flocking&) = delete;
};
#endif