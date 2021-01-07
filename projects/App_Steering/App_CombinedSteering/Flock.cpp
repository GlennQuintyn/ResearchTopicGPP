#include "stdafx.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

using namespace Elite;

const char* g_pFormations[]{ "Form A", "Form B", "Form C" };

//Constructor & Destructor
Flock::Flock(int blueAgents, int redAgents, float worldSize, SteeringAgent* pAgentToEvade, bool trimWorld)
	: m_WorldSize{ worldSize }
	, m_BlueGroupSize{ blueAgents }
	, m_RedGroupSize{ redAgents }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15 }
	, m_BlueNeighbors{ blueAgents }//make neighbors memory pool
	, m_RedNeighbors{ redAgents }//make neighbors memory pool
	, m_NrOfBlueNeighbors{ 0 }
	, m_NrOfRedNeighbors{ 0 }
	, m_AttackRange{ 10.f }
{
	//m_pWanderEvader = nullptr;

	float spawnWidth{ worldSize / 5.f };
	float spawnHeight{ worldSize / 4.f };

	m_BlueSpawnZone = { {worldSize / 8.f, (worldSize / 2.f) - (spawnHeight / 2.f)},spawnWidth, spawnHeight };
	m_RedSpawnZone = { {(worldSize * 7.f / 8.f) - spawnWidth, (worldSize / 2.f) - (spawnHeight / 2.f)}, spawnWidth, spawnHeight };

	//evade agenet andd its behavior
	//m_pWanderEvader = new Wander();
	//m_pAgentToEvade = new SteeringAgent();
	//m_pAgentToEvade->SetSteeringBehavior(m_pWanderEvader);
	//m_pAgentToEvade->SetMaxLinearSpeed(30.f);
	//m_pAgentToEvade->SetAutoOrient(true);
	//m_pAgentToEvade->SetMass(1.f);
	//m_pAgentToEvade->SetBodyColor({ 1,0,0 });

	//behaviors for blended steering
	m_pSperationBehavior = new Seperation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMathcBehavior = new VelocityMatch(this);
	m_pBlueSeekBehavior = new Seek();
	m_pRedSeekBehavior = new Seek();
	m_pAttackBehavior = new Attack(this);

	m_pAttackBehavior->SetAttackRadius(30.f);

	//blended steering
	//m_pBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.25f }, { m_pCohesionBehavior,0.25f }
	//	, { m_pVelMathcBehavior,0.25f } , { m_pSeekBehavior,0.25f } , {m_pWanderBehavior ,0.25f } });//implicit vetor of weighted behavior

	m_pBlueBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.54f }, { m_pCohesionBehavior, 0.55f }
		, { m_pVelMathcBehavior, 0.75f } , { m_pBlueSeekBehavior, 0.9f } });//implicit vetor of weighted behavior

	m_pRedBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.54f }, { m_pCohesionBehavior, 0.55f }
		, { m_pVelMathcBehavior, 0.75f } , { m_pRedSeekBehavior, .9f } });//implicit vetor of weighted behavior

	//behaviors for priority steering
	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(25.f);

	//priority steering
	m_pBluePrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pBlueBlendedSteering} });
	m_pRedPrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pRedBlendedSteering} });

	//spacial partitioning
	m_pSpacePartitioning = new CellSpace(worldSize, worldSize, int(worldSize / m_NeighborhoodRadius), int(worldSize / m_NeighborhoodRadius), m_BlueGroupSize);

	//init each agent
	for (int i = 0; i < m_BlueGroupSize; i++)
	{
		m_BlueAgents.push_back(new SteeringAgent{});
		m_BlueAgents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + spawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + spawnHeight) });
		m_BlueAgents[i]->SetMaxLinearSpeed(35.f);
		m_BlueAgents[i]->SetAutoOrient(true);
		m_BlueAgents[i]->SetMass(1.f);
		m_BlueAgents[i]->SetSteeringBehavior(m_pBluePrioritySteering);
		m_BlueAgents[i]->SetBodyColor(m_Blue);
		m_pSpacePartitioning->AddAgent(m_BlueAgents[i]);
		m_OldAgentPosVec.push_back(m_BlueAgents[i]->GetPosition());
	}

	for (int i = 0; i < m_RedGroupSize; i++)
	{
		m_RedAgents.push_back(new SteeringAgent{});
		m_RedAgents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + spawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + spawnHeight) });
		m_RedAgents[i]->SetMaxLinearSpeed(35.f);
		m_RedAgents[i]->SetAutoOrient(true);
		m_RedAgents[i]->SetMass(1.f);
		m_RedAgents[i]->SetSteeringBehavior(m_pRedPrioritySteering);
		m_RedAgents[i]->SetBodyColor(m_Red);
		m_pSpacePartitioning->AddAgent(m_RedAgents[i]);
		m_OldAgentPosVec.push_back(m_RedAgents[i]->GetPosition());
	}
}

Flock::~Flock()
{
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		SAFE_DELETE(steeringAgent);
	}

	for (SteeringAgent* steeringAgent : m_RedAgents)
	{
		SAFE_DELETE(steeringAgent);
	}

	//SAFE_DELETE(m_pWanderEvader);

	SAFE_DELETE(m_pBlueBlendedSteering);
	SAFE_DELETE(m_pRedBlendedSteering);
	SAFE_DELETE(m_pBluePrioritySteering);
	SAFE_DELETE(m_pRedPrioritySteering);

	SAFE_DELETE(m_pSperationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMathcBehavior);
	SAFE_DELETE(m_pBlueSeekBehavior);
	SAFE_DELETE(m_pRedSeekBehavior);
	SAFE_DELETE(m_pAttackBehavior);

	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pSpacePartitioning);
}

void Flock::Update(float deltaT, float worldSize, const TargetData& targetDataLclick, const TargetData& targetDataRclick)
{
	bool firstTime{ true };
	TargetData agentToEvadeDate{};
	//agentToEvadeDate.Position = m_pAgentToEvade->GetPosition();
	//agentToEvadeDate.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();

	/*for (SteeringAgent* steeringAgent : m_Agents)
	{
		steeringAgent->SetBodyColor({ 1,1,0 });
	}*/

	Elite::Vector2 m_BlueCenterPos{ m_BlueSpawnZone.bottomLeft.x + m_BlueSpawnZone.width / 2.f,m_BlueSpawnZone.bottomLeft.y + m_BlueSpawnZone.height / 2.f };
	Elite::Vector2 m_RedCenterPos{ m_RedSpawnZone.bottomLeft.x + m_RedSpawnZone.width / 2.f,m_RedSpawnZone.bottomLeft.y + m_RedSpawnZone.height / 2.f };

	m_BlueCenterPos = targetDataLclick.Position;
	m_RedCenterPos = targetDataRclick.Position;

	TargetData blueTargetData{};
	TargetData redTargetData{};
	blueTargetData.Position = { m_BlueCenterPos };
	redTargetData.Position = { m_RedCenterPos };

	//m_pBlueSeekBehavior->SetTarget(blueTargetData);
	//m_pBlueSeekBehavior->SetTarget(targetData);
	//m_pRedSeekBehavior->SetTarget(targetDataLclick);
	//m_pAgentToEvade->Update(deltaT);
	//m_pAgentToEvade->TrimToWorld({ 0,0 }, { worldSize ,worldSize });
	//m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());

	DEBUGRENDERER2D->DrawPoint(blueTargetData.Position, 4.5f, m_Blue, .01f);
	DEBUGRENDERER2D->DrawPoint(redTargetData.Position, 4.5f, m_Red, .01f);


	float agentSize{ m_BlueAgents[0]->GetRadius() * 4 };
	int index{};
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		m_NrOfBlueNeighbors = 0;

		blueTargetData.Position = { m_BlueCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_BlueCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };

		SetAgentTarget(steeringAgent, blueTargetData);

		DEBUGRENDERER2D->DrawPoint(blueTargetData.Position, 2, { 1,1,1 }, .8f);

		RegisterBlueNeighbours(steeringAgent);


		steeringAgent->Update(deltaT);
		steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });

		//if (m_EnablePartitioning)//bool to toggle spacial partition
		//{
		//	m_pSpacePartitioning->RegisterNeighbors(steeringAgent->GetPosition(), m_NeighborhoodRadius, m_DebugRenderPartitions, firstTime);
		//	m_Neighbors = m_pSpacePartitioning->GetNeighbors();
		//	m_NrOfNeighbors = m_pSpacePartitioning->GetNrOfNeighbors();
		//}
		//else
		//{
		//}
		//if (m_EnablePartitioning)//bool to toggle spacial partition
		//{
		//	m_pSpacePartitioning->UpdateAgentCell(steeringAgent, m_OldAgentPosVec[index]);
		//	m_OldAgentPosVec[index] = steeringAgent->GetPosition();
		//}

		steeringAgent->SetRenderBehavior(m_DebugRenderSteering);
		if (firstTime)
		{
			if (m_DebugRenderNeighborhood)
			{
				DEBUGRENDERER2D->DrawCircle(m_BlueAgents[0]->GetPosition(), m_NeighborhoodRadius, { 1,1,1 }, .5f);
				m_BlueAgents[0]->SetRenderBehavior(true);
				for (int index{}; index < m_NrOfBlueNeighbors; ++index)
				{
					//m_Neighbors[index]->SetBodyColor({ 0, 1, 0 });
				}
			}
			firstTime = false;
		}

		index++;
	}

	index = 0;
	for (SteeringAgent* steeringAgent : m_RedAgents)
	{
		m_NrOfRedNeighbors = 0;

		redTargetData.Position = { m_RedCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_RedCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };

		SetAgentTarget(steeringAgent, redTargetData);

		DEBUGRENDERER2D->DrawPoint(redTargetData.Position, 2, { 1,1,1 }, .8f);

		RegisterRedNeighbours(steeringAgent);



		//m_pEvadeBehavior->SetTarget(agentToEvadeDate);

		steeringAgent->Update(deltaT);
		steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });


		steeringAgent->SetRenderBehavior(m_DebugRenderSteering);
		index++;
	}



}

void Flock::Render(float deltaT) const
{
	//m_pAgentToEvade->Render(deltaT);
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		steeringAgent->Render(deltaT);
	}

	for (SteeringAgent* steeringAgent : m_RedAgents)
	{
		steeringAgent->Render(deltaT);
	}

	/*if (m_EnablePartitioning)
	{
		m_pSpacePartitioning->RenderCells();
	}*/


	//drawing their spawn zones
	std::vector<Elite::Vector2> Vec(4);//max of 4 point to polygon available

	Vec[0] = m_BlueSpawnZone.bottomLeft;
	Vec[1] = { m_BlueSpawnZone.bottomLeft.x + m_BlueSpawnZone.width,m_BlueSpawnZone.bottomLeft.y };
	Vec[2] = { m_BlueSpawnZone.bottomLeft.x + m_BlueSpawnZone.width,m_BlueSpawnZone.bottomLeft.y + m_BlueSpawnZone.height };
	Vec[3] = { m_BlueSpawnZone.bottomLeft.x ,m_BlueSpawnZone.bottomLeft.y + m_BlueSpawnZone.height };

	DEBUGRENDERER2D->DrawPolygon(&Vec[0], 4, { 0, 0, .7f }, .9f);

	Vec[0] = m_RedSpawnZone.bottomLeft;
	Vec[1] = { m_RedSpawnZone.bottomLeft.x + m_RedSpawnZone.width,m_RedSpawnZone.bottomLeft.y };
	Vec[2] = { m_RedSpawnZone.bottomLeft.x + m_RedSpawnZone.width,m_RedSpawnZone.bottomLeft.y + m_RedSpawnZone.height };
	Vec[3] = { m_RedSpawnZone.bottomLeft.x ,m_RedSpawnZone.bottomLeft.y + m_RedSpawnZone.height };

	DEBUGRENDERER2D->DrawPolygon(&Vec[0], 4, { 0.7f, 0, 0 }, .9f);

}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	//ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	//ImGui::Spacing();

	//ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Formation Sim");
	ImGui::Spacing();
	ImGui::Spacing();


	ImGui::Text("Blue Team formation");
	ImGui::Indent();
	ImGui::Combo("Blue", &m_BlueFormationIdx, g_pFormations, std::size(g_pFormations), 3);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Red Team formation");
	ImGui::Indent();
	ImGui::Combo("Red", &m_RedFormationIdx, g_pFormations, std::size(g_pFormations), 3);
	ImGui::Unindent();

	ImGui::Spacing();

	// Implement checkboxes and sliders here
	//ImGui::Checkbox("Debug render steering", &m_DebugRenderSteering);
	//ImGui::Checkbox("Debug render neighborhood", &m_DebugRenderNeighborhood);
	//ImGui::Checkbox("Debug render partitions", &m_DebugRenderPartitions);
	//ImGui::Checkbox("Enable partitions", &m_EnablePartitioning);


	ImGui::Spacing();
	ImGui::SliderFloat("Speration", GetBlueWeight(m_pSperationBehavior), 0.f, 1.f, "%.2f");
	*GetRedWeight(m_pSperationBehavior) = *GetBlueWeight(m_pSperationBehavior);

	ImGui::SliderFloat("cohesion", GetBlueWeight(m_pCohesionBehavior), 0.f, 1.f, "%.2f");
	*GetRedWeight(m_pCohesionBehavior) = *GetBlueWeight(m_pCohesionBehavior);

	ImGui::SliderFloat("Vel matching", GetBlueWeight(m_pVelMathcBehavior), 0.f, 1.f, "%.2f");
	*GetRedWeight(m_pVelMathcBehavior) = *GetBlueWeight(m_pVelMathcBehavior);

	ImGui::SliderFloat("Seek B", GetBlueWeight(m_pBlueSeekBehavior), 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Seek R", GetRedWeight(m_pRedSeekBehavior), 0.f, 1.f, "%.2f");
	//*GetRedWeight(m_pBlueSeekBehavior) = *GetBlueWeight(m_pBlueSeekBehavior);
	//ImGui::SliderFloat("Wander", GetWeight(m_pWanderBehavior), 0.f, 1.f, "%.2f");
	ImGui::Spacing();
	bool buttonReturn = ImGui::Button("ATTACK", { 75.f,25.f });

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

	//flip attack button to true if button was pressed once
	if (buttonReturn)
	{
		m_Attack = true;
	}
}

void Flock::RegisterBlueNeighbours(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		if (Elite::DistanceSquared(pAgent->GetPosition(), steeringAgent->GetPosition()) <= m_NeighborhoodRadius * m_NeighborhoodRadius &&
			(pAgent->GetPosition() != steeringAgent->GetPosition()))
		{
			m_BlueNeighbors[m_NrOfBlueNeighbors] = steeringAgent;
			m_NrOfBlueNeighbors++;
		}
	}
}

void Flock::RegisterRedNeighbours(SteeringAgent* pAgent)
{
	for (SteeringAgent* steeringAgent : m_RedAgents)
	{
		if (Elite::DistanceSquared(pAgent->GetPosition(), steeringAgent->GetPosition()) <= m_NeighborhoodRadius * m_NeighborhoodRadius &&
			(pAgent->GetPosition() != steeringAgent->GetPosition()))
		{
			m_RedNeighbors[m_NrOfRedNeighbors] = steeringAgent;
			m_NrOfRedNeighbors++;
		}
	}
}

Elite::Vector2 Flock::GetClosestEnemyLocation(const Elite::Vector2& agentPos, const Elite::Color& color) const
{
	float shortestDistance{ FLT_MAX };
	int index{}, count{};

	if (color == m_Blue)
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			float squaredDistance{ Elite::DistanceSquared(agentPos, steeringAgent->GetPosition()) };
			if (squaredDistance <= shortestDistance * shortestDistance)
			{
				shortestDistance = squaredDistance;
				index = count;
			}
			++count;
		}

		return m_RedAgents[index]->GetPosition();
	}
	else
	{
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			float squaredDistance{ Elite::DistanceSquared(agentPos, steeringAgent->GetPosition()) };
			if (squaredDistance <= shortestDistance * shortestDistance)
			{
				shortestDistance = squaredDistance;
				index = count;
			}
			++count;
		}

		return m_BlueAgents[index]->GetPosition();
	}
}

Elite::Vector2 Flock::GetAverageBlueNeighborPos(const Elite::Color& color) const
{
	Elite::Vector2 averagePos{};
	int count{};

	for (int index{}; index < m_NrOfBlueNeighbors; ++index)
	{
		//check if colors are equal
		if (m_BlueNeighbors[index]->GetBodyColor() == color)
		{
			averagePos += m_BlueNeighbors[index]->GetPosition();
			++count;
		}
	}

	if (count != 0)
	{
		averagePos /= float(count);
	}

	return averagePos;
}

Elite::Vector2 Flock::GetAverageRedNeighborPos(const Elite::Color& color) const
{
	Elite::Vector2 averagePos{};
	int count{};

	for (int index{}; index < m_NrOfRedNeighbors; ++index)
	{
		//check if colors are equal
		if (m_RedNeighbors[index]->GetBodyColor() == color)
		{
			averagePos += m_RedNeighbors[index]->GetPosition();
			++count;
		}
	}

	if (count != 0)
	{
		averagePos /= float(count);
	}

	return averagePos;
}

Elite::Vector2 Flock::GetAverageBlueNeighborVelocity(const Elite::Color& color) const
{
	Elite::Vector2 averageVelocity{};
	int count{};

	for (int index{}; index < m_NrOfBlueNeighbors; ++index)
	{
		//check if colors are equal
		if (m_BlueNeighbors[index]->GetBodyColor() == color)
		{
			averageVelocity += m_BlueNeighbors[index]->GetLinearVelocity();
			++count;
		}
	}

	if (count != 0)
	{
		averageVelocity /= float(count);
	}

	return averageVelocity;
}

Elite::Vector2 Flock::GetAverageRedNeighborVelocity(const Elite::Color& color) const
{
	Elite::Vector2 averageVelocity{};
	int count{};

	for (int index{}; index < m_NrOfRedNeighbors; ++index)
	{
		//check if colors are equal
		if (m_RedNeighbors[index]->GetBodyColor() == color)
		{
			averageVelocity += m_RedNeighbors[index]->GetLinearVelocity();
			++count;
		}
	}

	if (count != 0)
	{
		averageVelocity /= float(count);
	}

	return averageVelocity;
}


float* Flock::GetBlueWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlueBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlueBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

float* Flock::GetRedWeight(ISteeringBehavior* pBehavior)
{
	if (m_pRedBlendedSteering)
	{
		auto& weightedBehaviors = m_pRedBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

//this functions set the seek target of a specific agent
void Flock::SetAgentTarget(SteeringAgent* pSteeringAgent, const TargetData& targetdata)
{
	auto prioritySteering = pSteeringAgent->GetSteeringBehavior();
	auto blendedSteering = prioritySteering->As<PrioritySteering>()->GetBehaviourByIndex(1);
	auto seekBehaviour = blendedSteering->As<BlendedSteering>()->GetBehaviorByIndex(3);
	seekBehaviour->SetTarget(targetdata);
}
