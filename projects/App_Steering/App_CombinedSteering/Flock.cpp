#include "stdafx.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"
#include <iomanip>

using namespace Elite;

const char* g_pFormations[]{ "Half Phalanx", "Flying Wedge", "Wedge Phalanx", "Test" };

//Constructor & Destructor
Flock::Flock(const Elite::Rect& blueSpawnZone, const Elite::Rect& redSpawnZone, float worldSize, bool trimWorld)
	: m_WorldSize{ worldSize }
	, m_BlueGroupSize{}
	, m_RedGroupSize{}
	, m_TrimWorld{ trimWorld }
	//, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15 }
	, m_BlueNeighbors{}//make neighbors memory pool but only gets re-ajusted in the actual spawning of the agents
	, m_RedNeighbors{}//make neighbors memory pool but only gets re-ajusted in the actual spawning of the agents
	, m_NrOfBlueNeighbors{ 0 }
	, m_NrOfRedNeighbors{ 0 }
	, m_AttackRange{ 10.f }
{
	//because both spawn zones have the same dimension they are just general
	//m_SpawnWidth = { worldSize / 5.f };
	//m_SpawnHeight = { worldSize / 3.f };

	m_SpawnWidth = blueSpawnZone.width;//{ worldSize / 5.f };
	m_SpawnHeight = blueSpawnZone.height;//{ worldSize / 3.f };

	//m_BlueSpawnZone = { {worldSize / 8.f, (worldSize / 2.f) - (m_SpawnHeight / 2.f)},m_SpawnWidth, m_SpawnHeight };
	//m_RedSpawnZone = { {(worldSize * 7.f / 8.f) - m_SpawnWidth, (worldSize / 2.f) - (m_SpawnHeight / 2.f)}, m_SpawnWidth, m_SpawnHeight };

	m_BlueSpawnZone = blueSpawnZone;
	m_RedSpawnZone = redSpawnZone;

	//behaviors for blended steering
	m_pSperationBehavior = new Seperation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMathcBehavior = new VelocityMatch(this);
	m_pBlueSeekBehavior = new Seek();
	m_pRedSeekBehavior = new Seek();
	//priority behaviour
	m_pAttackBehavior = new Attack(this);

	m_pAttackBehavior->SetAttackRadius(30.f);

	//blended steering
	//m_pBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.25f }, { m_pCohesionBehavior,0.25f }
	//	, { m_pVelMathcBehavior,0.25f } , { m_pSeekBehavior,0.25f } , {m_pWanderBehavior ,0.25f } });//implicit vetor of weighted behavior

	m_pBlueBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.55f }, { m_pCohesionBehavior, 0.55f }
		, { m_pVelMathcBehavior, 0.35f } , { m_pBlueSeekBehavior, 0.9f } });//implicit vetor of weighted behavior

	m_pRedBlendedSteering = new BlendedSteering({ { m_pSperationBehavior, 0.55f }, { m_pCohesionBehavior, 0.55f }
		, { m_pVelMathcBehavior, 0.35f } , { m_pRedSeekBehavior, .9f } });//implicit vetor of weighted behavior

	//behaviors for priority steering
	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(25.f);

	//priority steering
	m_pBluePrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pBlueBlendedSteering} });
	m_pRedPrioritySteering = new PrioritySteering({ {m_pAttackBehavior},{m_pRedBlendedSteering} });

	//spacial partitioning
	m_pSpacePartitioning = new CellSpace(worldSize, worldSize, int(worldSize / m_NeighborhoodRadius), int(worldSize / m_NeighborhoodRadius), m_BlueGroupSize);

	SpawnBlueFormation();
	SpawnRedFormation();
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

	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pSpacePartitioning);
}

void Flock::Update(float deltaT, float worldSize, const TargetData& targetDataLclick, const TargetData& targetDataRclick)
{
	bool firstTime{ true };
	TargetData agentToEvadeDate{};

	//Elite::Vector2 m_BlueCenterPos{ m_BlueSpawnZone.bottomLeft.x + m_BlueSpawnZone.width / 2.f,m_BlueSpawnZone.bottomLeft.y + m_BlueSpawnZone.height / 2.f };
	//Elite::Vector2 m_RedCenterPos{ m_RedSpawnZone.bottomLeft.x + m_RedSpawnZone.width / 2.f,m_RedSpawnZone.bottomLeft.y + m_RedSpawnZone.height / 2.f };

	if (!m_Attack)
	{
		m_BlueCenterPos = targetDataLclick.Position;
		m_RedCenterPos = targetDataRclick.Position;
	}
	else
	{
		m_BlueCenterPos = { m_WorldSize / 2.f,m_WorldSize / 2.f };
		m_RedCenterPos = { m_WorldSize / 2.f ,m_WorldSize / 2.f };
	}


	TargetData blueTargetData{};
	TargetData redTargetData{};
	blueTargetData.Position = { m_BlueCenterPos };
	redTargetData.Position = { m_RedCenterPos };

	DEBUGRENDERER2D->DrawPoint(blueTargetData.Position, 4.5f, m_Blue, .01f);
	DEBUGRENDERER2D->DrawPoint(redTargetData.Position, 4.5f, m_Red, .01f);

	Formations blueFormation{ Formations(m_BlueFormationIdx) };
	Formations redFormation{ Formations(m_RedFormationIdx) };

	float agentSize{ SteeringAgent{}.GetRadius()/*  m_BlueAgents[0]->GetRadius() */ * 4 };//get base agent
	int index{};
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		m_NrOfBlueNeighbors = 0;

		//if agent is dead go to the next agent
		if (!steeringAgent->IsAlive())
		{
			steeringAgent->Update(deltaT);
			steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });
			continue;
		}

		switch (blueFormation)
		{
		case Formations::Test:
			blueTargetData.Position = { m_BlueCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_BlueCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };
			break;
		case Formations::HalfPhalanx:
			blueTargetData.Position = { m_BlueCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_BlueCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };
			break;
		case Formations::FlyingWedge:
		{
			float xDistance{};
			float yDistance{};

			CalcFlyingWedgeXY(index, xDistance, yDistance, false);

#pragma region MyRegion
			/*
			if (index == 0)
			{
				xDistance = 3;
			}
			else if (index < 4)
			{
				xDistance = 2;
				yDistance = -index + 2;
			}
			else if (index < 9)
			{
				xDistance = 1;
				yDistance = -(index - 2) + 4;
			}
			else if (index < 16)
			{
				yDistance = -(index - 6) + 6;
			}
			else if (index < 25)
			{
				xDistance = -1;
				yDistance = -(index - 12) + 8;
			}
			else if (index < 36)
			{
				xDistance = -2;
				yDistance = -(index - 20) + 10;
			}
			*/
#pragma endregion

			blueTargetData.Position = { m_BlueCenterPos.x + (agentSize * xDistance) , m_BlueCenterPos.y - (agentSize * yDistance) };
		}
		break;
		case Formations::WedgePhalanx:
		{
			float xDistance{};
			float yDistance{};

			CalcWedgePhalanxXY(index, xDistance, yDistance, false);

#pragma region MyRegion
			/*
			if (index < 2)
			{
				xDistance = 3;

				(index & 1) ? yDistance = -.5f : yDistance = .5f;
			}
			else if (index < 6)
			{
				xDistance = 2;
				yDistance = -index + 3.5f;
			}
			else if (index < 12)
			{
				xDistance = 1;
				yDistance = -index + 8.5f;
			}
			else if (index < 18)
			{
				(index < 15) ? yDistance = -index + 15.5f : yDistance = -index + 13.5f;
			}
			else if (index < 24)
			{
				xDistance = -1;
				(index < 21) ? yDistance = -index + 22.5f : yDistance = -index + 18.5f;//gap of 4
			}
			else if (index < 30)
			{
				xDistance = -2;
				(index < 27) ? yDistance = -index + 29.5f : yDistance = -index + 23.5f;//gap of 6
			}
			else if (index < 36)
			{
				xDistance = -3;
				(index < 33) ? yDistance = -index + 36.5f : yDistance = -index + 28.5f;//gap of 8
			}
			*/
#pragma endregion

			blueTargetData.Position = { m_BlueCenterPos.x + (agentSize * xDistance) , m_BlueCenterPos.y - (agentSize * yDistance) };
		}
		break;
		}

		SetAgentTarget(steeringAgent, blueTargetData);

		DEBUGRENDERER2D->DrawPoint(blueTargetData.Position, 2, { 1,1,1 }, .8f);

		RegisterBlueNeighbours(steeringAgent);


		steeringAgent->Update(deltaT);
		steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });

		steeringAgent->SetRenderBehavior(m_DebugRenderSteering);

		index++;
	}

	index = 0;
	for (SteeringAgent* steeringAgent : m_RedAgents)
	{
		m_NrOfRedNeighbors = 0;

		//if agent is dead do small update and go to the next agent
		if (!steeringAgent->IsAlive())
		{
			steeringAgent->Update(deltaT);
			steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });
			continue;
		}

		switch (redFormation)
		{
		case Formations::Test:
			redTargetData.Position = { m_RedCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_RedCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };
			break;
		case Formations::HalfPhalanx:
			redTargetData.Position = { m_RedCenterPos.x - (agentSize * 2.5f) + (agentSize * (index % 6)) ,m_RedCenterPos.y + (agentSize * 2.5f) - (agentSize * (index / 6)) };
			break;
		case Formations::FlyingWedge:
		{
			float xDistance{};
			float yDistance{};
			CalcFlyingWedgeXY(index, xDistance, yDistance, true);

			/*
			#pragma region MyRegion
						if (index == 0)
						{
							xDistance = -3;
						}
						else if (index < 4)
						{
							xDistance = -2;
							yDistance = -index + 2;
						}
						else if (index < 9)
						{
							xDistance = -1;
							yDistance = -(index - 2) + 4;
						}
						else if (index < 16)
						{
							yDistance = -(index - 6) + 6;
						}
						else if (index < 25)
						{
							xDistance = 1;
							yDistance = -(index - 12) + 8;
						}
						else if (index < 36)
						{
							xDistance = 2;
							yDistance = -(index - 20) + 10;
						}
			#pragma endregion
			*/

			redTargetData.Position = { m_RedCenterPos.x + (agentSize * xDistance) , m_RedCenterPos.y - (agentSize * yDistance) };
		}
		break;
		case Formations::WedgePhalanx:
		{
			float xDistance{};
			float yDistance{};

			CalcWedgePhalanxXY(index, xDistance, yDistance, true);
			/*
#pragma region MyRegion
			if (index < 2)
			{
				xDistance = -3;

				(index & 1) ? yDistance = -.5f : yDistance = .5f;
			}
			else if (index < 6)
			{
				xDistance = -2;
				yDistance = -index + 3.5f;
			}
			else if (index < 12)
			{
				xDistance = -1;
				yDistance = -index + 8.5f;
			}
			else if (index < 18)
			{
				(index < 15) ? yDistance = -index + 15.5f : yDistance = -index + 13.5f;
			}
			else if (index < 24)
			{
				xDistance = 1;
				(index < 21) ? yDistance = -index + 22.5f : yDistance = -index + 18.5f;//gap of 4
			}
			else if (index < 30)
			{
				xDistance = 2;
				(index < 27) ? yDistance = -index + 29.5f : yDistance = -index + 23.5f;//gap of 6
			}
			else if (index < 36)
			{
				xDistance = 3;
				(index < 33) ? yDistance = -index + 36.5f : yDistance = -index + 28.5f;//gap of 8
			}
#pragma endregion
			*/

			redTargetData.Position = { m_RedCenterPos.x + (agentSize * xDistance) , m_RedCenterPos.y - (agentSize * yDistance) };
		}
		break;
		}


		SetAgentTarget(steeringAgent, redTargetData);

		DEBUGRENDERER2D->DrawPoint(redTargetData.Position, 2, { 1,1,1 }, .8f);

		RegisterRedNeighbours(steeringAgent);

		steeringAgent->Update(deltaT);
		steeringAgent->TrimToWorld({ 0,0 }, { worldSize ,worldSize });

		steeringAgent->SetRenderBehavior(m_DebugRenderSteering);
		index++;
	}

	//if battle not over keep counting battle time
	if (m_BlueAgents.size() != 0 && m_RedAgents.size() != 0)
		m_TotalBattleTime += deltaT;

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
	int blueOldFormationIndx{ m_BlueFormationIdx };
	int redOldFormationIndx{ m_RedFormationIdx };

#pragma region Left Side Window
	//Setup
	int leftMenuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - leftMenuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)leftMenuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place blue Team Center");
	ImGui::Text("MMB: place red Team Center");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("FORMATION SIM");
	ImGui::Spacing();
	ImGui::Spacing();

	int tempBlueIndx{ m_BlueFormationIdx };
	int tempRedIndx{ m_RedFormationIdx };

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

	//if attacking mode
	if (m_Attack)
	{
		m_BlueFormationIdx = tempBlueIndx;
		m_RedFormationIdx = tempRedIndx;
	}

	bool fightButtonReturn = ImGui::Button("ATTACK", { 75.f,25.f });
	ImGui::SameLine(90.f);
	bool resetButtonReturn = ImGui::Button("Reset", { 75.f,25.f });


	ImGui::Spacing();
	ImGui::Text("Debuggin Params");
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

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
#pragma endregion


	if (m_BlueAgents.size() == 0 && m_RedAgents.size() != 0)
	{
		//Setup
		int endScreenMenuWidth = 250;
		int endScreenMenuHeight = 125;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth() - leftMenuWidth;
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(((float)width / 2.f) - (endScreenMenuWidth / 2.f), ((float)height / 2.f) - endScreenMenuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)endScreenMenuWidth, (float)endScreenMenuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);


		//ImGui::Text("Blue Team formation");
		ImGui::Indent();
		ImGui::Indent();
		ImGui::Spacing();
		ImGui::Text("RED TEAM HAS WON!");
		ImGui::Spacing();
		ImGui::Unindent();
		ImGui::Unindent();
		ImGui::Separator();

		ImGui::Spacing();
		ImGui::Text("STATS:");
		ImGui::Spacing();
		ImGui::Indent();
		std::stringstream endStatsStream{};
		endStatsStream << "Survivor Count: " << m_RedAgents.size();
		ImGui::Text(endStatsStream.str().c_str());
		endStatsStream.str(std::string());
		endStatsStream << "Battle Duration: " << std::fixed << std::setprecision(2) << m_TotalBattleTime << " seconds";
		ImGui::Text(endStatsStream.str().c_str());
		ImGui::Unindent();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}

	if (m_BlueAgents.size() != 0 && m_RedAgents.size() == 0)
	{
		//Setup
		int endScreenMenuWidth = 250;
		int endScreenMenuHeight = 125;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth() - leftMenuWidth;
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(((float)width / 2.f) - (endScreenMenuWidth / 2.f), ((float)height / 2.f) - endScreenMenuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)endScreenMenuWidth, (float)endScreenMenuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);


		//ImGui::Text("Blue Team formation");
		ImGui::Indent();
		ImGui::Indent();
		ImGui::Spacing();
		ImGui::Text("BLUE TEAM HAS WON!");
		ImGui::Spacing();
		ImGui::Unindent();
		ImGui::Unindent();
		ImGui::Separator();

		ImGui::Spacing();
		ImGui::Text("STATS:");
		ImGui::Spacing();
		ImGui::Indent();
		std::stringstream endStatsStream{};
		endStatsStream << "Survivor Count: " << m_BlueAgents.size();
		ImGui::Text(endStatsStream.str().c_str());
		endStatsStream.str(std::string());
		endStatsStream << "Battle Duration: " << std::fixed << std::setprecision(2) << m_TotalBattleTime << " seconds";
		ImGui::Text(endStatsStream.str().c_str());
		ImGui::Unindent();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}


	//flip attack button to true if button was pressed once
	if (fightButtonReturn)
	{
		m_Attack = true;
		std::cout << "ATTACK!\n";
		m_TotalBattleTime = 0;
	}

	if (resetButtonReturn)
	{
		m_Attack = false;
		std::cout << "Nope Reset\n";
		SpawnBlueFormation();
		SpawnRedFormation();
	}

	if (m_BlueFormationIdx != blueOldFormationIndx)
	{
		SpawnBlueFormation();
	}

	if (m_RedFormationIdx != redOldFormationIndx)
	{
		SpawnRedFormation();
	}
}

void Flock::RegisterBlueNeighbours(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	for (SteeringAgent* steeringAgent : m_BlueAgents)
	{
		if (Elite::DistanceSquared(pAgent->GetPosition(), steeringAgent->GetPosition()) <= m_NeighborhoodRadius * m_NeighborhoodRadius &&
			(pAgent->GetPosition() != steeringAgent->GetPosition()) && steeringAgent->IsAlive())
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
			(pAgent->GetPosition() != steeringAgent->GetPosition()) && steeringAgent->IsAlive())
		{
			m_RedNeighbors[m_NrOfRedNeighbors] = steeringAgent;
			m_NrOfRedNeighbors++;
		}
	}
}

SteeringAgent* Flock::GetClosestEnemy(const Elite::Vector2& agentPos, const Elite::Color& color) const
{
	float shortestDistance{ FLT_MAX };
	int index{};// , count{};
	SteeringAgent* pReturnAgent{ nullptr };

	if (color == m_Blue)
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			float squaredDistance{ Elite::DistanceSquared(agentPos, steeringAgent->GetPosition()) };
			if (squaredDistance <= shortestDistance * shortestDistance && steeringAgent->IsAlive())
			{
				shortestDistance = squaredDistance;
				pReturnAgent = m_RedAgents[index];
			}
			++index;
		}

		return pReturnAgent;
	}
	else
	{
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			float squaredDistance{ Elite::DistanceSquared(agentPos, steeringAgent->GetPosition()) };
			if (squaredDistance <= shortestDistance * shortestDistance && steeringAgent->IsAlive())
			{
				shortestDistance = squaredDistance;
				pReturnAgent = m_BlueAgents[index];
			}
			++index;
		}

		return pReturnAgent;
	}
}

Elite::Vector2 Flock::GetAverageBlueNeighborPos(const Elite::Color& color) const
{
	Elite::Vector2 averagePos{};
	int count{};

	for (int index{}; index < m_NrOfBlueNeighbors; ++index)
	{
		//check if colors are equal
		if (m_BlueNeighbors[index]->GetBodyColor() == color && m_BlueNeighbors[index]->IsAlive())
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
		if (m_RedNeighbors[index]->GetBodyColor() == color && m_RedNeighbors[index]->IsAlive())
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
		if (m_BlueNeighbors[index]->GetBodyColor() == color && m_BlueNeighbors[index]->IsAlive())
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
		if (m_RedNeighbors[index]->GetBodyColor() == color && m_RedNeighbors[index]->IsAlive())
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

void Flock::SpawnBlueFormation()
{
	Formations blueFormation{ Formations(m_BlueFormationIdx) };

	m_BlueCenterPos = { m_BlueSpawnZone.bottomLeft.x + (m_BlueSpawnZone.width / 2.f),m_BlueSpawnZone.bottomLeft.y + (m_BlueSpawnZone.height / 2.f) };

	switch (blueFormation)
	{
	case Formations::Test://4 units
	{
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_BlueAgents.clear();

		m_BlueGroupSize = 4;
		m_BlueNeighbors.resize(4);
		//init each agent
		for (int i = 0; i < m_BlueGroupSize; i++)
		{
			m_BlueAgents.push_back(new SteeringAgent{});
			m_BlueAgents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_BlueAgents[i]->SetMaxLinearSpeed(35.f);
			m_BlueAgents[i]->SetAutoOrient(true);
			m_BlueAgents[i]->SetMass(1.f);
			m_BlueAgents[i]->SetSteeringBehavior(m_pBluePrioritySteering);
			m_BlueAgents[i]->SetBodyColor(m_Blue);
			m_pSpacePartitioning->AddAgent(m_BlueAgents[i]);
			m_OldAgentPosVec.push_back(m_BlueAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::HalfPhalanx://30 units
	{
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_BlueAgents.clear();

		m_BlueGroupSize = 36;
		m_BlueNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_BlueGroupSize; i++)
		{
			m_BlueAgents.push_back(new SteeringAgent{});
			m_BlueAgents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_BlueAgents[i]->SetMaxLinearSpeed(35.f);
			m_BlueAgents[i]->SetAutoOrient(true);
			m_BlueAgents[i]->SetMass(1.f);
			m_BlueAgents[i]->SetSteeringBehavior(m_pBluePrioritySteering);
			m_BlueAgents[i]->SetBodyColor(m_Blue);
			m_pSpacePartitioning->AddAgent(m_BlueAgents[i]);
			m_OldAgentPosVec.push_back(m_BlueAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::FlyingWedge:
	{
		//cleanup old
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_BlueAgents.clear();

		m_BlueGroupSize = 36;
		m_RedNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_BlueGroupSize; i++)
		{
			m_BlueAgents.push_back(new SteeringAgent{});
			m_BlueAgents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_BlueAgents[i]->SetMaxLinearSpeed(35.f);
			m_BlueAgents[i]->SetAutoOrient(true);
			m_BlueAgents[i]->SetMass(1.f);
			m_BlueAgents[i]->SetSteeringBehavior(m_pBluePrioritySteering);
			m_BlueAgents[i]->SetBodyColor(m_Blue);
			m_pSpacePartitioning->AddAgent(m_BlueAgents[i]);
			m_OldAgentPosVec.push_back(m_BlueAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::WedgePhalanx:
	{
		for (SteeringAgent* steeringAgent : m_BlueAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_BlueAgents.clear();

		m_BlueGroupSize = 36;
		m_BlueNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_BlueGroupSize; i++)
		{
			m_BlueAgents.push_back(new SteeringAgent{});
			m_BlueAgents[i]->SetPosition({ Elite::randomFloat(m_BlueSpawnZone.bottomLeft.x,m_BlueSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_BlueSpawnZone.bottomLeft.y,m_BlueSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_BlueAgents[i]->SetMaxLinearSpeed(35.f);
			m_BlueAgents[i]->SetAutoOrient(true);
			m_BlueAgents[i]->SetMass(1.f);
			m_BlueAgents[i]->SetSteeringBehavior(m_pBluePrioritySteering);
			m_BlueAgents[i]->SetBodyColor(m_Blue);
			m_pSpacePartitioning->AddAgent(m_BlueAgents[i]);
			m_OldAgentPosVec.push_back(m_BlueAgents[i]->GetPosition());
		}
	}
	break;
	}
}

void Flock::SpawnRedFormation()
{
	Formations redFormation{ Formations(m_RedFormationIdx) };

	m_RedCenterPos = { m_RedSpawnZone.bottomLeft.x + m_RedSpawnZone.width / 2.f,m_RedSpawnZone.bottomLeft.y + m_RedSpawnZone.height / 2.f };

	switch (redFormation)
	{
	case Formations::Test://30 units
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_RedAgents.clear();

		m_RedGroupSize = 4;
		m_RedNeighbors.resize(4);
		//init each agent
		for (int i = 0; i < m_RedGroupSize; i++)
		{
			m_RedAgents.push_back(new SteeringAgent{});
			m_RedAgents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_RedAgents[i]->SetMaxLinearSpeed(35.f);
			m_RedAgents[i]->SetAutoOrient(true);
			m_RedAgents[i]->SetMass(1.f);
			m_RedAgents[i]->SetSteeringBehavior(m_pRedPrioritySteering);
			m_RedAgents[i]->SetBodyColor(m_Red);
			m_pSpacePartitioning->AddAgent(m_RedAgents[i]);
			m_OldAgentPosVec.push_back(m_RedAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::HalfPhalanx://30 units
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_RedAgents.clear();

		m_RedGroupSize = 36;
		m_RedNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_RedGroupSize; i++)
		{
			m_RedAgents.push_back(new SteeringAgent{});
			m_RedAgents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_RedAgents[i]->SetMaxLinearSpeed(35.f);
			m_RedAgents[i]->SetAutoOrient(true);
			m_RedAgents[i]->SetMass(1.f);
			m_RedAgents[i]->SetSteeringBehavior(m_pRedPrioritySteering);
			m_RedAgents[i]->SetBodyColor(m_Red);
			m_pSpacePartitioning->AddAgent(m_RedAgents[i]);
			m_OldAgentPosVec.push_back(m_RedAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::FlyingWedge:
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_RedAgents.clear();

		m_RedGroupSize = 36;
		m_RedNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_RedGroupSize; i++)
		{
			m_RedAgents.push_back(new SteeringAgent{});
			m_RedAgents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_RedAgents[i]->SetMaxLinearSpeed(35.f);
			m_RedAgents[i]->SetAutoOrient(true);
			m_RedAgents[i]->SetMass(1.f);
			m_RedAgents[i]->SetSteeringBehavior(m_pRedPrioritySteering);
			m_RedAgents[i]->SetBodyColor(m_Red);
			m_pSpacePartitioning->AddAgent(m_RedAgents[i]);
			m_OldAgentPosVec.push_back(m_RedAgents[i]->GetPosition());
		}
	}
	break;
	case Formations::WedgePhalanx:
	{
		for (SteeringAgent* steeringAgent : m_RedAgents)
		{
			SAFE_DELETE(steeringAgent);
		}
		m_RedAgents.clear();

		m_RedGroupSize = 36;
		m_RedNeighbors.resize(36);
		//init each agent
		for (int i = 0; i < m_RedGroupSize; i++)
		{
			m_RedAgents.push_back(new SteeringAgent{});
			m_RedAgents[i]->SetPosition({ Elite::randomFloat(m_RedSpawnZone.bottomLeft.x,m_RedSpawnZone.bottomLeft.x + m_SpawnWidth), Elite::randomFloat(m_RedSpawnZone.bottomLeft.y,m_RedSpawnZone.bottomLeft.y + m_SpawnHeight) });
			m_RedAgents[i]->SetMaxLinearSpeed(35.f);
			m_RedAgents[i]->SetAutoOrient(true);
			m_RedAgents[i]->SetMass(1.f);
			m_RedAgents[i]->SetSteeringBehavior(m_pRedPrioritySteering);
			m_RedAgents[i]->SetBodyColor(m_Red);
			m_pSpacePartitioning->AddAgent(m_RedAgents[i]);
			m_OldAgentPosVec.push_back(m_RedAgents[i]->GetPosition());
		}
	}
	break;
	}
}

void Flock::CalcFlyingWedgeXY(int index, float& xDistance, float& yDistance, bool Inverted)
{
	if (index == 0)
	{
		xDistance = 3;
	}
	else if (index < 4)
	{
		xDistance = 2;
		yDistance = -index + 2;
	}
	else if (index < 9)
	{
		xDistance = 1;
		yDistance = -(index - 2) + 4;
	}
	else if (index < 16)
	{
		yDistance = -(index - 6) + 6;
	}
	else if (index < 25)
	{
		xDistance = -1;
		yDistance = -(index - 12) + 8;
	}
	else if (index < 36)
	{
		xDistance = -2;
		yDistance = -(index - 20) + 10;
	}

	if (Inverted)
	{
		xDistance *= -1;
	}
}

void Flock::CalcWedgePhalanxXY(int index, float& xDistance, float& yDistance, bool Inverted)
{
	if (index < 2)
	{
		xDistance = 3;

		(index & 1) ? yDistance = -.5f : yDistance = .5f;
	}
	else if (index < 6)
	{
		xDistance = 2;
		yDistance = -index + 3.5f;
	}
	else if (index < 12)
	{
		xDistance = 1;
		yDistance = -index + 8.5f;
	}
	else if (index < 18)
	{
		(index < 15) ? yDistance = -index + 15.5f : yDistance = -index + 13.5f;
	}
	else if (index < 24)
	{
		xDistance = -1;
		(index < 21) ? yDistance = -index + 22.5f : yDistance = -index + 18.5f;//gap of 4
	}
	else if (index < 30)
	{
		xDistance = -2;
		(index < 27) ? yDistance = -index + 29.5f : yDistance = -index + 23.5f;//gap of 6
	}
	else if (index < 36)
	{
		xDistance = -3;
		(index < 33) ? yDistance = -index + 36.5f : yDistance = -index + 28.5f;//gap of 8
	}

	if (Inverted)
	{
		xDistance *= -1;
	}
}

//this functions set the seek target of a specific agent
void Flock::SetAgentTarget(SteeringAgent* pSteeringAgent, const TargetData& targetdata)
{
	auto prioritySteering = pSteeringAgent->GetSteeringBehavior();
	auto blendedSteering = prioritySteering->As<PrioritySteering>()->GetBehaviourByIndex(1);
	auto seekBehaviour = blendedSteering->As<BlendedSteering>()->GetBehaviorByIndex(3);
	seekBehaviour->SetTarget(targetdata);
}
