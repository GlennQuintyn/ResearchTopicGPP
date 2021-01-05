#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;

class Flock
{
public:
	Flock(int flockSize = 50, float worldSize = 100.f, SteeringAgent* pAgentToEvade = nullptr, bool trimWorld = false);

	~Flock();

	void Update(float deltaT, float worldSize, const TargetData& targetData);
	void UpdateAndRenderUI();
	void Render(float deltaT) const;

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }

	Elite::Vector2 GetAverageNeighborPos(const Elite::Color& color) const;
	Elite::Vector2 GetAverageNeighborVelocity(const Elite::Color& color) const;

private:
	//Space partitioning
	CellSpace* m_pSpacePartitioning{ nullptr };
	std::vector<Elite::Vector2> m_OldAgentPosVec{};

	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents{};

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	//Behaviors
	Seperation* m_pSperationBehavior;
	Cohesion* m_pCohesionBehavior;
	VelocityMatch* m_pVelMathcBehavior;
	Seek* m_pSeekBehavior;
	Wander* m_pWanderBehavior;
	Evade* m_pEvadeBehavior;
	Wander* m_pWanderEvader;

	//spawn zones
	Elite::Rect m_BlueSpawnZone{};
	Elite::Rect m_RedSpawnZone{};

	Elite::Color m_Blue{ 0, 0, 1.f };
	Elite::Color m_Red{ 1.f, 0, 0 };

	//UI bools
	bool m_DebugRenderSteering{};
	bool m_DebugRenderNeighborhood{};
	bool m_DebugRenderPartitions{ true };
	bool m_EnablePartitioning{};

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};