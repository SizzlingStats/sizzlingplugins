
#ifndef TEAMPLAY_GAMERULE_STATES_H
#define TEAMPLAY_GAMERULE_STATES_H

namespace Teamplay_GameRule_States
{

	enum gamerules_winreason
	{
		WINREASON_NONE =0,
		WINREASON_ALL_POINTS_CAPTURED,
		WINREASON_OPPONENTS_DEAD,
		WINREASON_FLAG_CAPTURE_LIMIT,
		WINREASON_DEFEND_UNTIL_TIME_LIMIT,
		WINREASON_STALEMATE,
		WINREASON_TIMELIMIT,
		WINREASON_WINLIMIT,
	};

	//-----------------------------------------------------------------------------
	// Round states
	//-----------------------------------------------------------------------------
	enum gamerules_roundstate_t
	{
		// initialize the game, create teams
		GR_STATE_INIT = 0,

		//Before players have joined the game. Periodically checks to see if enough players are ready
		//to start a game. Also reverts to this when there are no active players
		GR_STATE_PREGAME,

		//The game is about to start, wait a bit and spawn everyone
		GR_STATE_STARTGAME,

		//All players are respawned, frozen in place
		GR_STATE_PREROUND,

		//Round is on, playing normally
		GR_STATE_RND_RUNNING,

		//Someone has won the round
		GR_STATE_TEAM_WIN,

		//Noone has won, manually restart the game, reset scores
		GR_STATE_RESTART,

		//Noone has won, restart the game
		GR_STATE_STALEMATE,

		//Game is over, showing the scoreboard etc
		GR_STATE_GAME_OVER,

		GR_NUM_ROUND_STATES
	};

	static const char *GetStateName(gamerules_roundstate_t state);
	inline bool IsValidState(gamerules_roundstate_t state);

	static const char *GetStateName(gamerules_roundstate_t state)
	{
		static const char *pszStateNames[] = 
		{
			"GR_STATE_INIT",
			"GR_STATE_PREGAME",
			"GR_STATE_STARTGAME",
			"GR_STATE_PREROUND",
			"GR_STATE_RND_RUNNING",
			"GR_STATE_TEAM_WIN",
			"GR_STATE_RESTART",
			"GR_STATE_STALEMATE",
			"GR_STATE_GAME_OVER"
		};

		if (IsValidState(state))
		{
			return pszStateNames[state];
		}
		else
		{
			return "error: state out of range";
		}
	}

	inline bool IsValidState(gamerules_roundstate_t state)
	{
		return (state >= 0 && state < GR_NUM_ROUND_STATES);
	}

}

#endif // TEAMPLAY_GAMERULE_STATES_H
