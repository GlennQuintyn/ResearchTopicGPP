#include "stdafx.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(int flockSize, float worldSize, SteeringAgent* pAgentToEvade, bool trimWorld)
	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15 }
	, m_Neighbors{ flockSize }//make neighbors memory pool
	, m_NrOfNeighbors{ 0 }
{
	m_pWanderEvader = nullptr;

	float spawnWidth{ worldSize / 5.f };
	float spawnHeight{ worldSize / 2.f };

	m_BlueSpawnZone = { {worldSize / 8.f, worldSize / 4.f},spawnWidth,  spawnHeight };
	m_RedSpawnZone = { {(worldSize * 7.f / 8.f) - spawnWidth, worldSize / 4.f}, spawnWidth, spawnHeight };

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
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();

	//blended steering
	//m_pBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.25f }, { m_pCohesionBehavior,0.25f }
	//	, { m_pVelMathcBehavior,0.25f } , { m_pSeekBehavior,0.25f } , {m_pWanderBehavior ,0.25f } });//implicit vetor of weighted behavior

	m_pBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0 }, { m_pCohesionBehavior, 0 }
		, { m_pVelMathcBehavior, 0 } , { m_pSeekBehavior, 0 } , {m_pWanderBehavior , 0 } });//implicit vetor of weighted behavior

	//behaviors for priority steering
	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(25.f);

	//priority steering
	m_pPrioritySteering = new PrioritySteering({ {m_pEvadeBehavior},{m_pBlendedSteering} });

	//spacial partitioning
	m_pSpacePartitioning = new CellSpace(worldSize, worldSize, int(worldSize / m_NeighborhoodRadius), int(worldSize / m_NeighborhoodRadius), m_FlockSize);

	//init each agent
	for (int i = 0; i < m_FlockSize / 2; i++)
	{
		m_Agents.push_back(new SteeringAgent{});
		m_Agents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + spawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + spawnHeight) });
		m_Agents[i]->SetMaxLinearSpeed(35.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(1.f);
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetBodyColor(m_Blue);
		m_pSpacePartitioning->AddAgent(m_Agents[i]);
		m_OldAgentPosVec.push_back(m_Agents[i]->GetPosition());
	}

	for (int i = m_FlockSize / 2; i < m_FlockSize; i++)
	{
		m_Agents.push_back(new SteeringAgent{});
		m_Agents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + spawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + spawnHeight) });
		m_Agents[i]->SetMaxLinearSpeed(35.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(1.f);
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetBodyColor(m_Red);
		m_pSpacePartitioning->AddAgent(m_Agents[i]);
		m_OldAgentPosVec.push_back(m_Agents[i]->GetPosition());
	}
}

Flock::~Flock()
{
	for (SteeringAgent* steeringAgent : m_Agents)
	{
		SAFE_DELETE(steeringAgent);
	}
	SAFE_DELETE(m_pWanderEvader);

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pSperationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMathcBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);

	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pSpacePartitioning);
}

void Flock::Update(float deltaT, float worldSize, const TargetData& targetData)
{
	bool firstTime{ true };
	TargetData agentToEvadeDate{};
	//agentToEvadeDate.Position = m_pAgentToEvade->GetPosition();
	//agentToEvadeDate.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();

	/*for (SteeringAgent* steeringAgent : m_Agents)
	{
		steeringAgent->SetBodyColor({ 1,1,0 });
	}*/

	m_pSeekBehavior->SetTarget(targetData);
	//TargetData selfMadeTargetData{};
	//selfMadeTargetData.Position = { m_WorldSize / 2.f ,m_WorldSize / 2.f };
	//m_pSeekBehavior->SetTarget(selfMadeTargetData);

	//m_pAgentToEvade->Update(deltaT);
	//m_pAgentToEvade->TrimToWorld({ 0,0 }, { worldSize ,worldSize });

	//m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());

	int index{};
	for (SteeringAgent* steeringAgent : m_Agents)
	{
		m_NrOfNeighbors = 0;

		if (m_EnablePartitioning)//bool to toggle spacial partition
		{
			m_pSpacePartitioning->RegisterNeighbors(steeringAgent->GetPosition(), m_NeighborhoodRadius, m_DebugRenderPartitions, firstTime);
			m_Neighbors = m_pSpacePartitioning->GetNeighbors();
			m_NrOfNeighbors = m_pSpacePartitioning->GetNrOfNeighbors();
		}
		else
		{
			RegisterNeighbors(steeringAgent);
		}


		m_pEvadeBehavior->SetTarget(agentToEvadeDate);

		steeringAgent->Update(deltaT);
		steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });

		if (m_EnablePartitioning)//bool to toggle spacial partition
		{
			m_pSpacePartitioning->UpdateAgentCell(steeringAgent, m_OldAgentPosVec[index]);
			m_OldAgentPosVec[index] = steeringAgent->GetPosition();
		}

		steeringAgent->SetRenderBehavior(m_DebugRenderSteering);
		if (firstTime)
		{
			if (m_DebugRenderNeighborhood)
			{
				DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 1,1,1 }, .5f);
				m_Agents[0]->SetRenderBehavior(true);
				for (int index{}; index < m_NrOfNeighbors; ++index)
				{
					//m_Neighbors[index]->SetBodyColor({ 0, 1, 0 });
				}
			}
			firstTime = false;
		}

		index++;
	}
	// loop over all the boids	
	// register its neighbors
	// update it
	// trim it to the world
}

void Flock::Render(float deltaT) const
{
	//m_pAgentToEvade->Render(deltaT);
	for (SteeringAgent* steeringAgent : m_Agents)
	{
		steeringAgent->Render(deltaT);
	}

	if (m_EnablePartitioning)
	{
		m_pSpacePartitioning->RenderCells();
	}

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
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	ImGui::Checkbox("Debug render steering", &m_DebugRenderSteering);
	ImGui::Checkbox("Debug render neighborhood", &m_DebugRenderNeighborhood);
	ImGui::Checkbox("Debug render partitions", &m_DebugRenderPartitions);
	ImGui::Checkbox("Enable partitions", &m_EnablePartitioning);


	ImGui::Spacing();
	ImGui::SliderFloat("Speration", GetWeight(m_pSperationBehavior), 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("cohesion", GetWeight(m_pCohesionBehavior), 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Vel matching", GetWeight(m_pVelMathcBehavior), 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Seek", GetWeight(m_pSeekBehavior), 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Wander", GetWeight(m_pWanderBehavior), 0.f, 1.f, "%.2f");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();

}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	for (SteeringAgent* steeringAgent : m_Agents)
	{
		if (Elite::DistanceSquared(pAgent->GetPosition(), steeringAgent->GetPosition()) <= m_NeighborhoodRadius * m_NeighborhoodRadius &&
			(pAgent->GetPosition() != steeringAgent->GetPosition()))
		{
			m_Neighbors[m_NrOfNeighbors] = steeringAgent;
			m_NrOfNeighbors++;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos(const Elite::Color& color) const
{
	Elite::Vector2 averagePos{};
	int count{};

	for (int index{}; index < m_NrOfNeighbors; ++index)
	{
		//check if colors are equal
		if (m_Neighbors[index]->GetBodyColor() == color)
		{
			averagePos += m_Neighbors[index]->GetPosition();
			++count;
		}		
	}

	if (count != 0)
	{
		averagePos /= float(count);
	}

	return averagePos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity(const Elite::Color& color) const
{
	Elite::Vector2 averageVelocity{};
	int count{};

	for (int index{}; index < m_NrOfNeighbors; ++index)
	{
		//check if colors are equal
		if (m_Neighbors[index]->GetBodyColor() == color)
		{
			averageVelocity += m_Neighbors[index]->GetLinearVelocity();
			++count;
		}
	}

	if (count != 0)
	{
		averageVelocity /= float(count);
	}

	return averageVelocity;
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
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
