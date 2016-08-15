
#include "SSTeamData.h"

tfteam to_tfteam( int teamid )
{

	switch (teamid) {
		TFTEAM_SPECTATOR:
			return TFTEAM_SPECTATOR;
		TFTEAM_RED:
			return TFTEAM_RED;
		TFTEAM_BLUE:
			return TFTEAM_BLUE;
		default:
			return TFTEAM_UNASSIGNED;
	}
}

void TeamScoreData::Reset()
{
	m_AverageMedicDistance.Reset();
}

CTeamDataManager::CTeamDataManager()
{

}

CTeamDataManager::~CTeamDataManager()
{
}

void CTeamDataManager::Reset()
{
	TeamScoreData *tm_data = nullptr;
	
	if ((tm_data = GetTeamStats(TFTEAM_RED)) != nullptr)
	{
		tm_data->Reset();
	}

	if ((tm_data = GetTeamStats(TFTEAM_BLUE)) != nullptr)
	{
		tm_data->Reset();
	}

}

void CTeamDataManager::ResetTeamStatsData()
{
}

void CTeamDataManager::AddStatSample( tfteam team, TeamStat stat, float sample_point )
{

	if (stat == TeamStat::MedicCohesion)
	{
		TeamScoreData *team_data = GetTeamStats(team);
		if (!team_data)
			return;

		team_data->m_AverageMedicDistance.AddNumber(sample_point);
	}
}

double CTeamDataManager::GetStat( tfteam team, TeamStat stat )
{
	if (stat == TeamStat::MedicCohesion)
	{
		TeamScoreData *team_data = GetTeamStats(team);
		if (!team_data)
			return 0.0;
		return team_data->m_AverageMedicDistance;
	}

	return 0.0;
}


TeamScoreData *CTeamDataManager::GetTeamStats( tfteam team )
{
	switch (team)
	{
		case TFTEAM_RED:
			return &m_TeamScoreData[0];
		case TFTEAM_BLUE:
			return &m_TeamScoreData[1];
		default:
			return nullptr;
	}
}

