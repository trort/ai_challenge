#ifndef BOT_H_
#define BOT_H_

#include "State.h"

/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves
	
	//My part
	int team_size;
	
	void Initialize();		//initialize everything
	void AttackEasyHills();		//within attackradius steps
	void FindFood();
	void DefendHome();			//put ants at guard points
	void RedAlartDefense();		//all defend when enemy too close | team size done
	void AttackHills();			//attack seen hills | team size done
	void ExploreMap();			//try to reach unseen points | team size done
	void KillEnemies();			//kill all other enemies
	void MakeDefaultMove();

	void GuardLineDefense();	//kill ant coming close, send one ant for each enemy | team size done
	void GuardArroundHome();	//put at least one ant near each home | team size done
	void HackMap();			//at least one ant for each region | team size done
};

#endif //BOT_H_
