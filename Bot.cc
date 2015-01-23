#include "Bot.h"

using namespace std;

struct myclass
{
	bool operator() (ant_loc_pair a, ant_loc_pair b) {return a.dist<b.dist;}
}pair_comp;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    endTurn();

    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();
        makeMoves();
		//cout<<"All moves done"<<endl;
        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;
	
	/*
	//test functions
	Initialize();
	if(state.hills.size()!=0)
		AttackEasyHills();
	if(state.home.size()!=0 || state.hills.size() == 0)//do it when i have hills or no hill to attack
		FindFood();
	if(state.home.size() <= 2 || state.ants_available >= 20*state.home.size())
	{
		GuardArroundHome();
		GuardLineDefense();
	}
	if(state.home.size()==1)
	{
		RedAlartDefense();
	}
	if(state.ants_available >= 5*state.hack_loc.size())
		HackMap();
	if(state.hills.size()!=0)
		AttackHills();
	if(state.unseen.size()!=0)
		ExploreMap();
	KillEnemies();
	MakeDefaultMove();
	*/
	
	////////////////////////////////INITIALIZE//////////////////////////////////////////
	Initialize();
	//cout<<"Init done"<<endl;
	
	//resource and point, most important
	if(state.hills.size()!=0)
		AttackEasyHills();
	//advanced find food
	if(state.home.size()!=0 || state.hills.size() == 0)//do it when i have hills or no hill to attack
		FindFood();
	//cout<<"Food done"<<endl;
	
	//defend hill, second important thing when only one hill
	//	if(state.myAnts.size()-state.food.size()-state.hills.size() >= 10*state.home.size())	//defend only when there are enough ants and not too many hills
	//		DefendHome();
	if(state.home.size()==1)
	{
		team_size = min((int)floor(state.ants_available*0.55)+1, (int)(3*state.enemyAnts.size()));
		RedAlartDefense();
	}
	if(state.home.size() == 1 || state.ants_available >= 20*state.home.size())
	{
		team_size = min((int)floor(state.ants_available*0.15), (int)floor(1.5*state.enemyAnts.size()));
		GuardLineDefense();
	}
	if(state.home.size() <= 2 || state.ants_available >= 10*state.home.size())
	{
		team_size = (int)floor(state.ants_available*0.1);
		GuardArroundHome();
	}
	//cout<<"Guard done"<<endl;
	
	//hack map
	{
		team_size = (int)floor(state.ants_available*0.15);
		HackMap();
	}
	
	//advanced attack hills
	if(state.hills.size()!=0)
	{
		team_size = min((int)floor(state.ants_available*0.8), (int)(2*state.enemyAnts.size()));
		AttackHills();
	}
	
	//speeded explore the map and kill nearby ants, if nothing better to do
	if(state.unseen.size()!=0)
	{
		team_size = state.ants_available;
		ExploreMap();
	}
	//cout<<"Map done"<<endl;
	
	if(state.ants_available > state.enemyAnts.size())
		KillEnemies();
	
	//try to unblock my own hills and try random moves to be safe
	MakeDefaultMove();
	//cout<<"Random done"<<endl;
	
    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
        state.reset();
    state.turn++;

    cout << "go" << endl;
};

void Bot::Initialize()
{
	state.orders = vector<vector<bool> >(state.rows, vector<bool>(state.cols, false));
	state.foods2go.clear();
	state.foods2go.resize(state.food.size(), false);
	state.ants2move.clear();
	state.ants2move.resize(state.myAnts.size(), false);
	state.enemies2attack.clear();
	state.enemies2attack.resize(state.enemyAnts.size(),false);
	state.my_ant_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));	//my ant map
	state.food_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	state.hill_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	state.enemy_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	//load ant map
	for(int ant=0; ant<(int)state.myAnts.size(); ant++)
	{
		state.my_ant_map[state.myAnts[ant].row][state.myAnts[ant].col] = ant;
	}
	
	//load food map
	for(int f = 0; f < (int)state.food.size(); f++)
	{
		state.food_map[state.food[f].row][state.food[f].col] = f;
	}
	
	//load enemy map
	for(int enemy=0; enemy < (int)state.enemyAnts.size(); enemy++)
	{
		state.enemy_map[state.enemyAnts[enemy].row][state.enemyAnts[enemy].col] = enemy;
	}
	
	//update guard points info
	state.update_guard_points();
	
	//update home info
	state.update_home_info();
	
	//update enemy hill info
	state.update_hill_info();
	
	//load hill map
	for(int h = 0; h < (int)state.hills.size(); h++)
	{
		state.hill_map[state.hills[h].row][state.hills[h].col] = h;
	}
	state.hills2go.clear();
	state.hills2go.resize(state.hills.size(), false);
	
	//update unseen points
	state.update_unseen_points();
	
	//update combat info
	state.update_combat_info();
	
	//count ants
	state.ants_available = state.myAnts.size();
	
	//update hack locations
	state.hack_loc.clear();
	for(int iLoc = 0; iLoc < (int)state.tower_loc.size(); iLoc++)
	{
		if(state.seen_grid[state.tower_loc[iLoc].row][state.tower_loc[iLoc].col]) //tower seen
		{
			state.hack_loc.push_back(state.tower_loc[iLoc]);
		}
	}
	
	state.sacrafice = false;
}

void Bot::FindFood()
{
	std::vector<ant_loc_pair> target_dist;
	target_dist.clear();
	
	for(int i_food=0; i_food<(int)state.food.size(); i_food++)
	{
		std::queue<Location> search_queue;
		while(!search_queue.empty()) search_queue.pop();
		std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
		
		search_queue.push(state.food[i_food]);
		search_map[state.food[i_food].row][state.food[i_food].col] = 0;
		/*
		std::vector<Location> food_gather_points = state.points_in_rad(state.food[i_food], state.spawnradius);
		for(int p=0; p<(int)food_gather_points.size(); p++)
		{
			if(search_map[food_gather_points[p].row][food_gather_points[p].col] != 0
				&& !state.grid[food_gather_points[p].row][food_gather_points[p].col].isWater)
			{
				search_queue.push(food_gather_points[p]);
				search_map[food_gather_points[p].row][food_gather_points[p].col] = 0;
				//cout<<"one point added"<<endl;
			}
		}
		*/
		while(!search_queue.empty())
		{
			Location loc = search_queue.front();
			search_queue.pop();
			
			if(state.my_ant_map[loc.row][loc.col] != -1  && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an ant
			{
				//cout<<"found ant!"<<endl;
				ant_loc_pair new_pair(state.my_ant_map[loc.row][loc.col], state.food[i_food]);
				new_pair.dist = search_map[loc.row][loc.col];
				for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
				{
					Location nLoc = state.getLocation(loc,i_direct);
					if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
						new_pair.ant_ds.d[i_direct] = 1;
				}
				target_dist.push_back(new_pair);
				//cout<<new_pair.ant_ds.d[0]<<new_pair.ant_ds.d[1]<<new_pair.ant_ds.d[2]<<new_pair.ant_ds.d[3]<<new_pair.ant_ds.d[4]<<endl;
			}
			
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				
				if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
					&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
				{
					search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
					search_queue.push(nLoc);
				}
			}
		}
	}
	
	std::sort(target_dist.begin(), target_dist.end(), pair_comp);
	for(int i_dist=0; i_dist<(int)target_dist.size(); i_dist++)
	{
		ant_loc_pair pair = target_dist[i_dist];
		if(!state.ants2move[pair.ant])
		{
			if(state.food_map[pair.loc.row][pair.loc.col] != -1 && !state.foods2go[state.food_map[pair.loc.row][pair.loc.col]])
			{
				if(state.makeMove_ant_directions(pair.ant_ds)) state.foods2go[state.food_map[pair.loc.row][pair.loc.col]]=true;
			}
		}
	}
}

void Bot::DefendHome()
{
	int guardNo;
	int antsLeft = state.myAnts.size()-state.food.size()-state.hills.size();
	if(antsLeft <= 100) guardNo = antsLeft / 10; 
	else guardNo = 10 + (antsLeft - 100)/20;
	//if(guardNo > state.guard_points.size()) guardNo = state.guard_points.size();
	
	for(int lv = 0; lv < 6; lv++)
	{
		int guardNoLevel = state.guard_points[lv].size();
		if(guardNoLevel > guardNo) guardNoLevel = guardNo;		//not enough ants to finish this level
		
		for(int guard = 0; guard < guardNoLevel; guard ++)
		{
			int ant = state.my_ant_map[state.guard_points[lv][guard].row][state.guard_points[lv][guard].col];
			
			if(ant != -1)
			{
				state.ants2move[ant] = true;
				state.orders[state.guard_points[lv][guard].row][state.guard_points[lv][guard].col] = true;
			}
			else
			{
				std::queue<Location> search_queue;
				while(!search_queue.empty()) search_queue.pop();
				std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
		
				search_queue.push(state.guard_points[lv][guard]);
				search_map[state.guard_points[lv][guard].row][state.guard_points[lv][guard].col] = 0;
				while(!search_queue.empty())
				{
					Location loc = search_queue.front();
					search_queue.pop();
					
					if(state.my_ant_map[loc.row][loc.col] != -1  && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
					{
						ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
						for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
						{
							Location nLoc = state.getLocation(loc,i_direct);
							if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
								new_ant_ds.d[i_direct] = 1;
						}
						state.makeMove_ant_directions(new_ant_ds);
						break;		//one ant to go is enough
					}
					
					for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
					{
						Location nLoc = state.getLocation(loc,i_direct);
						if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
							&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
						{
							search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
							search_queue.push(nLoc);
						}
					}
				}
			}
		}
		
		guardNo-=guardNoLevel;
	}
}

void Bot::AttackHills()
{
	std::queue<Location> search_queue;
	while(!search_queue.empty()) search_queue.pop();
	std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	//plan attack	
	if(state.hills.size()!=0)
	{
		for(int i_hill=0; i_hill<(int)state.hills.size(); i_hill++)
		{
			search_queue.push(state.hills[i_hill]);
			search_map[state.hills[i_hill].row][state.hills[i_hill].col] = 0;
		}
	}
		
	while(!search_queue.empty())
	{
		if(team_size <=0) break;
		
		Location loc = search_queue.front();
		search_queue.pop();
		
		if(state.my_ant_map[loc.row][loc.col] != -1  && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
		{
			ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
					new_ant_ds.d[i_direct] = 1;
			}
			if(state.makeMove_ant_directions(new_ant_ds))
			{
				team_size --;
			}
		}
		
		for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
		{
			Location nLoc = state.getLocation(loc,i_direct);
			if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
				&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
			{
				search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
				search_queue.push(nLoc);
			}
		}
	}
	//cout<<"Attack done"<<endl;
}

void Bot::ExploreMap()
{
	std::queue<Location> search_queue;
	while(!search_queue.empty()) search_queue.pop();
	std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	for(int i_loc=0; i_loc<(int)state.unseen.size(); i_loc++)	//explore unseen
	{
		search_queue.push(state.unseen[i_loc]);
		search_map[state.unseen[i_loc].row][state.unseen[i_loc].col] = 0;
	}
	
	while(!search_queue.empty())
	{
		if(team_size <= 0) break;
		
		Location loc = search_queue.front();
		search_queue.pop();
		
		if(state.my_ant_map[loc.row][loc.col] != -1 && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
		{
			ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
					new_ant_ds.d[i_direct] = 1;
			}
			if(state.makeMove_ant_directions(new_ant_ds)) team_size --;
		}
		
		for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
		{
			Location nLoc = state.getLocation(loc,i_direct);
			if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
				&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
			{
				search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
				search_queue.push(nLoc);
			}
		}
	}
}

void Bot::MakeDefaultMove()
{
	for(int ant=0; ant<(int)state.myAnts.size(); ant++)
	{
        if(!state.ants2move[ant] /*&& state.grid[state.myAnts[ant].row][state.myAnts[ant].col].hillPlayer==0*/)//ant blocking my hill
		{
			/*
			if(state.grid[state.myAnts[ant].row][state.myAnts[ant].col].hillPlayer==0)	//ant blocking my hill
			{
				for(int i=0; i<4; i++)
				{
					if(state.random_move(ant)) break;
				}
			}
			else state.random_move(ant);
			*/
			ant_directions new_ant_ds(ant);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				new_ant_ds.d[i_direct] = 1;
			}
			state.makeMove_ant_directions(new_ant_ds);
		}
    }
}

void Bot::AttackEasyHills()
{
	std::vector<ant_loc_pair> target_dist;
	target_dist.clear();
	
	for(int i_hill=0; i_hill<(int)state.hills.size(); i_hill++)
	{
		std::queue<Location> search_queue;
		while(!search_queue.empty()) search_queue.pop();
		std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
		
		search_queue.push(state.hills[i_hill]);
		search_map[state.hills[i_hill].row][state.hills[i_hill].col] = 0;
		while(!search_queue.empty())
		{
			Location loc = search_queue.front();
			search_queue.pop();
			
			if(state.my_ant_map[loc.row][loc.col] != -1  && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an ant
			{
				//cout<<"found ant!"<<endl;
				ant_loc_pair new_pair(state.my_ant_map[loc.row][loc.col], state.hills[i_hill]);
				new_pair.dist = search_map[loc.row][loc.col];
				if(new_pair.dist < (int)state.attackradius)
				{
					for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
					{
						Location nLoc = state.getLocation(loc,i_direct);
						if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
							new_pair.ant_ds.d[i_direct] = 1;
					}
					target_dist.push_back(new_pair);
				}
				//cout<<new_pair.ant_ds.d[0]<<new_pair.ant_ds.d[1]<<new_pair.ant_ds.d[2]<<new_pair.ant_ds.d[3]<<new_pair.ant_ds.d[4]<<endl;
			}
			
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				
				if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
					&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
				{
					search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
					search_queue.push(nLoc);
				}
			}
		}
	}
	
	std::sort(target_dist.begin(), target_dist.end(), pair_comp);
	for(int i_dist=0; i_dist<(int)target_dist.size(); i_dist++)
	{
		ant_loc_pair pair = target_dist[i_dist];
		if(!state.ants2move[pair.ant])
		{
			if(state.hill_map[pair.loc.row][pair.loc.col] != -1 && !state.hills2go[state.hill_map[pair.loc.row][pair.loc.col]])
			{
				if(state.makeMove_ant_directions(pair.ant_ds)) state.hills2go[state.hill_map[pair.loc.row][pair.loc.col]]=true;
			}
		}
	}
}

void Bot::RedAlartDefense()
{
	std::queue<Location> search_queue;
	while(!search_queue.empty()) search_queue.pop();
	std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	for(int hill = 0; hill < (int)state.home.size(); hill++)
	{
		std::vector<Location> alart_region = state.points_in_rad(state.home[hill], max(state.viewradius-state.attackradius, state.attackradius));
		for(int i_loc = 0; i_loc < (int)alart_region.size(); i_loc++)
		{
			Location loc = alart_region[i_loc];
			if(state.enemy_map[loc.row][loc.col] != -1)		//found enenmy approaching
			{
				search_queue.push(loc);
				search_map[loc.row][loc.col] = 0;
				state.enemies2attack[state.enemy_map[loc.row][loc.col]] = true;
			}
		}
	}
	
	while(!search_queue.empty())
	{
		if(team_size <= 0) break;
		
		Location loc = search_queue.front();
		search_queue.pop();
		
		if(state.my_ant_map[loc.row][loc.col] != -1  && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
		{
			ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
					new_ant_ds.d[i_direct] = 1;
			}
			if(state.makeMove_ant_directions(new_ant_ds))
			{
				team_size --;
			}
		}
		
		for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
		{
			Location nLoc = state.getLocation(loc,i_direct);
			if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
				&& !state.orders[nLoc.row][nLoc.col])	//water is the only thing truely block the way
			{
				search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
				search_queue.push(nLoc);
			}
		}
	}
}

void Bot::GuardLineDefense()
{
	std::queue<Location> search_queue;
	while(!search_queue.empty()) search_queue.pop();
	std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	for(int hill = 0; hill < (int)state.home.size(); hill++)
	{
		std::vector<Location> guardline = state.points_in_rad(state.home[hill], state.viewradius + state.attackradius);
		for(int point = 0; point < (int)guardline.size(); point++)
		{
			//if(team_size <= 0) break;
			
			if(state.enemy_map[guardline[point].row][guardline[point].col]!=-1) 
				//&& !state.enemies2attack[state.enemy_map[guardline[point].row][guardline[point].col]])	//found enemy!
			{
				search_queue.push(guardline[point]);
				search_map[guardline[point].row][guardline[point].col] = 0;
			}
		}
	}
	
	while(!search_queue.empty())
	{
		if(team_size <= 0) break;
		
		Location loc = search_queue.front();
		search_queue.pop();
		
		if(state.my_ant_map[loc.row][loc.col] != -1 && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
		{
			ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
					new_ant_ds.d[i_direct] = 1;
			}
			if(state.makeMove_ant_directions(new_ant_ds))
			{
				//state.enemies2attack[state.enemy_map[guardline[point].row][guardline[point].col]] = true;
				team_size --;
				break;		//stop when found one ant to attack
			}
		}
		
		for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
		{
			Location nLoc = state.getLocation(loc,i_direct);
			if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
				&& !state.orders[nLoc.row][nLoc.col])	//water, my ant and hill block the way
			{
				search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
				search_queue.push(nLoc);
			}
		}
	}
}

void Bot::GuardArroundHome()
{
	for(int hill = 0; hill < (int)state.home.size(); hill++)
	{
		if(team_size <= 0) break;
		
		std::vector<Location> guardline = state.points_in_rad(state.home[hill], state.attackradius);
		bool guarded = false;
		for(int point = 0; point < (int)guardline.size(); point++)
		{
			if(state.my_ant_map[guardline[point].row][guardline[point].col]!=-1)
				guarded = true;
		}
		if(!guarded)
		{
			std::queue<Location> search_queue;
			while(!search_queue.empty()) search_queue.pop();
			std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
			
			search_queue.push(state.home[hill]);
			search_map[state.home[hill].row][state.home[hill].col] = 0;
			
			while(!search_queue.empty())
			{
				Location loc = search_queue.front();
				search_queue.pop();
				
				if(state.my_ant_map[loc.row][loc.col] != -1 && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
				{
					ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
					for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
					{
						Location nLoc = state.getLocation(loc,i_direct);
						if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
							new_ant_ds.d[i_direct] = 1;
					}
					if(state.makeMove_ant_directions(new_ant_ds))
					{
						//state.enemies2attack[state.enemy_map[guardline[point].row][guardline[point].col]] = true;
						team_size --;
						break;		//stop when found one ant to attack
					}
				}
				
				for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
				{
					Location nLoc = state.getLocation(loc,i_direct);
					if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
						&& !state.orders[nLoc.row][nLoc.col])	//water, my ant and hill block the way
					{
						search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
						search_queue.push(nLoc);
					}
				}
			}
		}
	}
}

void Bot::HackMap()
{
	for(int hack = 0; hack < (int)state.hack_loc.size(); hack ++)
	{
		if(team_size <= 0) break;
		
		std::vector<Location> guardline = state.points_in_rad(state.hack_loc[hack], state.viewradius);
		bool guarded = false;
		for(int point = 0; point < (int)guardline.size(); point++)
		{
			if(state.my_ant_map[guardline[point].row][guardline[point].col]!=-1)
				guarded = true;
		}
		if(!guarded)
		{
			std::queue<Location> search_queue;
			while(!search_queue.empty()) search_queue.pop();
			std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
			
			search_queue.push(state.hack_loc[hack]);
			search_map[state.hack_loc[hack].row][state.hack_loc[hack].col] = 0;
			
			while(!search_queue.empty())
			{
				Location loc = search_queue.front();
				search_queue.pop();
				
				if(state.my_ant_map[loc.row][loc.col] != -1 && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
				{
					ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
					for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
					{
						Location nLoc = state.getLocation(loc,i_direct);
						if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
							new_ant_ds.d[i_direct] = 1;
					}
					if(state.makeMove_ant_directions(new_ant_ds))
					{
						//state.enemies2attack[state.enemy_map[guardline[point].row][guardline[point].col]] = true;
						team_size --;
						break;		//stop when found one ant to attack
					}
				}
				
				for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
				{
					Location nLoc = state.getLocation(loc,i_direct);
					if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
						&& !state.orders[nLoc.row][nLoc.col])	//water, my ant and hill block the way
					{
						search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
						search_queue.push(nLoc);
					}
				}
			}
		}
	}
}

void Bot::KillEnemies()
{
	std::queue<Location> search_queue;
	while(!search_queue.empty()) search_queue.pop();
	std::vector<std::vector<int> > search_map = vector<vector<int> >(state.rows, vector<int>(state.cols, -1));
	
	//add enemies left to attack
	for(int enemy = 0; enemy < (int)state.enemyAnts.size(); enemy++)	//attack enemies
	{
		if(!state.enemies2attack[enemy])
		{
			search_queue.push(state.enemyAnts[enemy]);
			search_map[state.enemyAnts[enemy].row][state.enemyAnts[enemy].col] = 0;
		}
	}
	
	while(!search_queue.empty())
	{
		Location loc = search_queue.front();
		search_queue.pop();
		
		if(state.my_ant_map[loc.row][loc.col] != -1 && !state.ants2move[state.my_ant_map[loc.row][loc.col]])	//see an idol ant
		{
			ant_directions new_ant_ds(state.my_ant_map[loc.row][loc.col]);
			for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
			{
				Location nLoc = state.getLocation(loc,i_direct);
				if(search_map[nLoc.row][nLoc.col] == search_map[loc.row][loc.col]-1 || search_map[nLoc.row][nLoc.col] == 0)
					new_ant_ds.d[i_direct] = 1;
			}
			state.makeMove_ant_directions(new_ant_ds);
		}
		
		for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
		{
			Location nLoc = state.getLocation(loc,i_direct);
			if((search_map[nLoc.row][nLoc.col] == -1) && !state.grid[nLoc.row][nLoc.col].isWater
				&& !state.orders[nLoc.row][nLoc.col])	//water, my ant and hill block the way
			{
				search_map[nLoc.row][nLoc.col] = search_map[loc.row][loc.col]+1;
				search_queue.push(nLoc);
			}
		}
	}
}
