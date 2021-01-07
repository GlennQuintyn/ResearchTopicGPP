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
	Flock(int blueAgents = 50, int redAgents = 50, float worldSize = 100.f, SteeringAgent* pAgentToEvade = nullptr, bool trimWorld = false);

	~Flock();

	void Update(float deltaT, float worldSize, const TargetData& targetDataLclick, const TargetData& targetDataRclick);
	void UpdateAndRenderUI();
	void Render(float deltaT) const;

	void RegisterBlueNeighbours(SteeringAgent* pAgent);
	void RegisterRedNeighbours(SteeringAgent* pAgent);
	
	int GetNrOfBlueNeighbors() const { return m_NrOfBlueNeighbors; }
	int GetNrOfRedNeighbors() const { return m_NrOfRedNeighbors; }
	
	Elite::Vector2 GetClosestEnemyLocation(const Elite::Vector2& agentPos,const Elite::Color& color) const;
	//Elite::Vector2 GetNrOfRedNeighbors() const { return m_NrOfRedNeighbors; }

	const vector<SteeringAgent*>& GetBlueNeighbors() const { return m_BlueNeighbors; }
	const vector<SteeringAgent*>& GetRedNeighbors() const { return m_RedNeighbors; }

	Elite::Vector2 GetAverageBlueNeighborPos(const Elite::Color& color) const;
	Elite::Vector2 GetAverageRedNeighborPos(const Elite::Color& color) const;
	
	Elite::Vector2 GetAverageBlueNeighborVelocity(const Elite::Color& color) const;
	Elite::Vector2 GetAverageRedNeighborVelocity(const Elite::Color& color) const;



private:
	//Space partitioning
	CellSpace* m_pSpacePartitioning{ nullptr };
	std::vector<Elite::Vector2> m_OldAgentPosVec{};

	
	int m_BlueGroupSize = 0;
	int m_RedGroupSize = 0;
	vector<SteeringAgent*> m_BlueAgents{};
	vector<SteeringAgent*> m_RedAgents{};

	// neighborhood agents
	vector<SteeringAgent*> m_BlueNeighbors;
	vector<SteeringAgent*> m_RedNeighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfBlueNeighbors = 0;
	int m_NrOfRedNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;

	// steering Behaviors
	BlendedSteering* m_pBlueBlendedSteering = nullptr;
	BlendedSteering* m_pRedBlendedSteering = nullptr;
	PrioritySteering* m_pBluePrioritySteering = nullptr;
	PrioritySteering* m_pRedPrioritySteering = nullptr;

	//Behaviors
	Seperation* m_pSperationBehavior;
	Cohesion* m_pCohesionBehavior;
	VelocityMatch* m_pVelMathcBehavior;
	Seek* m_pBlueSeekBehavior;
	Seek* m_pRedSeekBehavior;
	//Wander* m_pWanderBehavior;
	//Wander* m_pWanderEvader;
	Evade* m_pEvadeBehavior;
	Attack* m_pAttackBehavior;

	//spawn zones
	Elite::Rect m_BlueSpawnZone{};
	Elite::Rect m_RedSpawnZone{};


	//body/team colors
	Elite::Color m_Blue{ 0, 0, 1.f };
	Elite::Color m_Red{ 1.f, 0, 0 };

	float m_AttackRange{};

	//static inline const char* m_pFormations[]{ "Form A", "Form B", "Form C" };

	int m_BlueFormationIdx{};
	int m_RedFormationIdx{};

	bool m_Attack{};

	//UI bools
	bool m_DebugRenderSteering{};
	bool m_DebugRenderNeighborhood{};
	bool m_DebugRenderPartitions{ true };
	bool m_EnablePartitioning{};

	// private functions
	float* GetBlueWeight(ISteeringBehavior* pBehaviour);
	float* GetRedWeight(ISteeringBehavior* pBehaviour);
	
	void SetAgentTarget(SteeringAgent* pSteeringAgent,const TargetData& targetdata);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};