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

	//flock object
	Flock m_Flock{ 36, 36, m_TrimWorldSize, m_pAgentToEvade, true };

	//C++ make the class non-copyable
	App_Flocking(const App_Flocking&) = delete;
	App_Flocking& operator=(const App_Flocking&) = delete;
};
#endif