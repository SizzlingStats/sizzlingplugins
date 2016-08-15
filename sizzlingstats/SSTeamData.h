#ifndef SS_TEAM_DATA
#define SS_TEAM_DATA

#include "ComputationalTools.h"

class CSizzPluginContext;
/**
 * Maintains the round statistics for the team as a whole.
 */


struct TeamScoreData
{
	MovingAverage<double, 64> m_AverageMedicDistance;
	void Reset();
};

enum tfteam
{
	TFTEAM_UNASSIGNED = 0,
	TFTEAM_SPECTATOR = 1,
	TFTEAM_RED = 2,
	TFTEAM_BLUE = 3
};

enum class TeamStat
{
	MedicCohesion
};


tfteam to_tfteam( int );

class CTeamDataManager
{
	public:
		CTeamDataManager();
		~CTeamDataManager();

		void AddStatSample( tfteam team, TeamStat stat, float sample_point );
		double GetStat( tfteam team, TeamStat stat );

		void ResetTeamStatsData();
		void Reset();

	private:
		TeamScoreData *GetTeamStats( tfteam team );
		TeamScoreData m_TeamScoreData[2];
};



#endif
