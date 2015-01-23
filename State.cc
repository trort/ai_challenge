#include "State.h"

using namespace std;

//constructor
State::State()
{
    gameover = 0;
    turn = 0;
    bug.open("./debug.txt");
};

//deconstructor
State::~State()
{
    bug.close();
};

//sets the state up
void State::setup()
{
    grid = vector<vector<Square> >(rows, vector<Square>(cols, Square()));
	
	//MY PART
	//myHillNo = 0;
	guard_points = vector<vector<Location> >(6, vector<Location>(0, Location()));
	//unseen grids
	unseen.clear();
	for(int row=0; row<rows; row++)
	{
		for(int col=0;col<cols; col++)
		{
			Location tmp_loc(row, col);
			unseen.push_back(tmp_loc);
		}
	}
	std::random_shuffle(unseen.begin(),unseen.end());
	seen_grid = vector<vector<bool> >(rows, vector<bool>(cols, 0));
	//seen hills
	home.clear();
	hills.clear();
	hills_owner.clear();
	//status for combat
	//status = std::vector<std::vector<int> >(rows, std::vector<int>(cols, -1));
	noPlayers = 2;
	
	//creat tower location list
	tower_loc.clear();
	Location nLoc;
	int row_grid = (int)floor(((double) rows)/viewradius) + 1;
	int col_grid = (int)floor(((double) cols)/viewradius) + 1;
	for(int r = 0; r < row_grid; r++) for(int c = 0; c < col_grid; c++)
	{
		nLoc.row = (int)floor(r*viewradius);
		nLoc.col = (int)floor(c*viewradius);
		tower_loc.push_back(nLoc);
	}
	random_shuffle(tower_loc.begin(),tower_loc.end());
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    myAnts.clear();
    enemyAnts.clear();
    myHills.clear();
    enemyHills.clear();
    food.clear();
    for(int row=0; row<rows; row++)
        for(int col=0; col<cols; col++)
            if(!grid[row][col].isWater)
                grid[row][col].reset();
};

//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
	if(direction < 4)
	{
		cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;
		//cout << "o " << loc.row << " " << loc.col << " " << direction << endl;

		Location nLoc = getLocation(loc, direction);
		grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
		grid[loc.row][loc.col].ant = -1;
	}
};

//NEW FUNCTION
bool State::makeMove_direction(const int &ant, int direction)
{
	Location loc = myAnts[ant];
	Location nLoc = getLocation(loc, direction);
	bool possible = true;
	if(grid[nLoc.row][nLoc.col].isWater || grid[nLoc.row][nLoc.col].isFood)		//impossible to go
		possible = false;
	else if(direction != 4 && grid[nLoc.row][nLoc.col].ant==0)		//if i ll not stay, i ll not step on my buddy
		possible = false;
	else if(grid[nLoc.row][nLoc.col].hillPlayer==0)	//do not block hill
		possible = false;
	else if(orders[nLoc.row][nLoc.col])		//collision
		possible = false;
	else if(bad_move(ant, direction))
		possible = false;
	
    if(possible)
	{
        ants2move[ant]=true;
		ants_available --;
		update_local_combat(ant, direction);
		makeMove(loc, direction);	//actually make the move
		orders[nLoc.row][nLoc.col]=true;
		
		return true;
	}
	else
		return false;
}

//NEW FUNCTION
bool State::makeMove_ant_directions(const ant_directions &ant_ds)
{
	
	std::vector<int> directions;
	directions.clear();
	for(int i_direct=0; i_direct<TDIRECTIONS; i_direct++)
	{
		if(ant_ds.d[i_direct])
		{
			directions.push_back(i_direct);
		}
	}
	std::random_shuffle(directions.begin(), directions.end());
	for(int d=0; d<directions.size(); d++)
	{
		if(makeMove_direction(ant_ds.ant, directions[d]))
		{
			return true;
		}
	}
	
	/*
	for(int direct = TDIRECTIONS-1; direct >= 0; direct--)
	//for(int direct=0; direct<TDIRECTIONS; direct++)
	{
		if(ant_ds.d[direct])
		{
			if(makeMove_direction(ant_ds.ant, direct))
			{
				return true;
			}
		}
	}
	*/
}

//NEW FUNCTION
bool State::random_move(const int &ant)
{
	int d=rand()%TDIRECTIONS;
	if(makeMove_direction(ant,d)) return true;
	else return false;
}

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);
    return sqrt(dr*dr + dc*dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction)
{
    return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};

std::vector<Location> State::points_in_rad(const Location &loc, double rad)
{
	int row = loc.row;
	int col = loc.col;
	//double rad = sqrt(rad_2);
	int iRad = (int)floor(rad);
	std::vector<Location> ans;
	ans.clear();
	for(int r = row-iRad; r<=row+iRad; r++)
	{
		for(int c = col-iRad; c<=col+iRad; c++)
		{
			Location nLoc((r+rows)%rows, (c+cols)%cols);
			if(distance(loc, nLoc)<=rad) ans.push_back(nLoc);
		}
	}
	return ans;
}

void State::update_guard_points()
{
	//guard_points.clear();
	//level 1
	const int no_lv[6] = {4, 8, 4, 8, 8, 4};
	const int level[6][8][2] = {	{ {1, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}, {-1, -1}, {-1, 1}, {1, -1} },
								{ {1, 2}, {-1, -2}, {2, -1}, {-2, 1}, {1, -2}, {-1, 2}, {2, 1}, {-2, -1} },
								{ {2, 2}, {-2, -2}, {2, -2}, {-2, 2}, {2, 2}, {-2, -2}, {2, -2}, {-2, 2} },
								{ {1, 3}, {-1, -3}, {3, -1}, {-3, 1}, {1, -3}, {-1, 3}, {3, 1}, {-3, -1} },
								{ {3, 2}, {-3, -2}, {2, -3}, {-2, 3}, {3, -2}, {-3, 2}, {2, 3}, {-2, -3} },
								{ {3, 3}, {-3, -3}, {3, -3}, {-3, 3}, {3, 3}, {-3, -3}, {3, -3}, {-3, 3} } };
	
	for(int lv = 0; lv < 6; lv++)
	{
		guard_points[lv].clear();
		for(int hill = 0; hill<(int)myHills.size(); hill++)
		{
			Location hLoc = myHills[hill];
			for(int p = 0; p < no_lv[lv]; p++)
			{
				Location nLoc(((hLoc.row+level[lv][p][0]+rows)%rows), ((hLoc.col+level[lv][p][1]+cols)%cols));
				if(!grid[nLoc.row][nLoc.col].isWater) guard_points[lv].push_back(nLoc);
			}
		}
		//random_shuffle(guard_points[lv].begin(), guard_points[lv].end());
	}
	
}

void State::update_player_no()
{
	for(int enemy = 0; enemy < (int)enemyAnts.size(); enemy++)
	{
		if(grid[enemyAnts[enemy].row][enemyAnts[enemy].col].ant + 1 > noPlayers) noPlayers = grid[enemyAnts[enemy].row][enemyAnts[enemy].col].ant + 1;
	}
	for(int hill=0; hill<(int)enemyHills.size(); hill++)
	{
		if(grid[enemyHills[hill].row][enemyHills[hill].col].hillPlayer + 1 > noPlayers) noPlayers = grid[enemyHills[hill].row][enemyHills[hill].col].hillPlayer + 1;
	}
}

void State::update_combat_info()
{
	//update noPlayers;
	update_player_no();
	
	influence = std::vector<std::vector<std::vector<int> > >(noPlayers, std::vector<std::vector<int> >(rows, std::vector<int>(cols, 0)));
	total = std::vector<std::vector<int> >(rows, std::vector<int>(cols, 0));
	fighting = std::vector<std::vector<std::vector<int> > >(noPlayers, std::vector<std::vector<int> >(rows, std::vector<int>(cols, -1)));
	
	occupy = std::vector<std::vector<std::vector<int> > >(noPlayers, std::vector<std::vector<int> >(rows, std::vector<int>(cols, 0)));
	status = std::vector<std::vector<int> >(rows, std::vector<int>(cols, 0));
	
	int player;
	
	//cout<<"Init"<<endl;
	
	player = 0;		//my ants first
	for(int ant = 0; ant < (int)myAnts.size(); ant++)
	{
		std::vector<std::vector<bool> > influ_area(rows, std::vector<bool>(cols, false));
		Location nLoc;
		for(int direction = 0; direction < TDIRECTIONS; direction++)
		{
			nLoc = getLocation(myAnts[ant], direction);
			if(/*!occupy[player][nLoc.row][nLoc.col] &&*/ !grid[nLoc.row][nLoc.col].isWater)
			{
				std::vector<Location> influ_points = points_in_rad(nLoc, attackradius);
				for(int point = 0; point < influ_points.size(); point++)
				{
					if(!influ_area[influ_points[point].row][influ_points[point].col])
					{
						influ_area[influ_points[point].row][influ_points[point].col] = true;
						influence[player][influ_points[point].row][influ_points[point].col] += 1;
					}
				}
				occupy[player][nLoc.row][nLoc.col] += 1;
			}
		}
	}
	//cout<<"mine"<<endl;
	
	for(int enemy = 0; enemy < (int)enemyAnts.size(); enemy++)		//for all enemy ants
	{
		std::vector<std::vector<bool> > influ_area(rows, std::vector<bool>(cols, false));
		Location nLoc;
		player = grid[enemyAnts[enemy].row][enemyAnts[enemy].col].ant;
		for(int direction = 0; direction < TDIRECTIONS; direction++)
		{
			nLoc = getLocation(enemyAnts[enemy], direction);
			if(/*!occupy[player][nLoc.row][nLoc.col] &&*/ !grid[nLoc.row][nLoc.col].isWater)
			{
				std::vector<Location> influ_points = points_in_rad(nLoc, attackradius);
				for(int point = 0; point < influ_points.size(); point++)
				{
					if(!influ_area[influ_points[point].row][influ_points[point].col])
					{
						influ_area[influ_points[point].row][influ_points[point].col] = true;
						influence[player][influ_points[point].row][influ_points[point].col] += 1;
					}
				}
				occupy[player][nLoc.row][nLoc.col] += 1;
			}
		}
	}
	//cout<<"enemy"<<endl;
	
	//update total, fighting and status
	for(int row = 0; row < rows; row++) for(int col = 0; col < cols; col++)
	{
		total[row][col] = 0;
		for(int p = 0; p < noPlayers; p++) total[row][col] += influence[p][row][col];
		for(int p = 0; p < noPlayers; p++)
		{
			if(occupy[p][row][col] != 0)		//fighting is reasonable only when there is an ant to fight
			{
				fighting[p][row][col] = total[row][col] - influence[p][row][col];
			}
		}
	}
	
	for(int row = 0; row < rows; row++) for(int col = 0; col < cols; col++)
	{
		//cout<<"fighting "<<row<<" "<<col<<" "<<fighting[0][row][col]<<endl;
		/*
		for(int p = 1; p < noPlayers; p++)		//not include mine
		{
			if(fighting[p][row][col] > max) max = fighting[p][row][col];
			if(fighting[p][row][col] < min) min = fighting[p][row][col];
		}
		*/
		
		//consider all points in attack range
		if(occupy[0][row][col]!=0)	//only consider points my ant can go
		{
			int min = 999999, max = 0;
			Location loc(row, col);
			std::vector<Location> fighting_range = points_in_rad(loc, attackradius);
			for(int point = 0; point<(int)fighting_range.size(); point++)
			{
				for(int p = 1; p < noPlayers; p++)
				{
					if(occupy[p][fighting_range[point].row][fighting_range[point].col] != 0)
					{
						if(fighting[p][fighting_range[point].row][fighting_range[point].col] > max) max = fighting[p][fighting_range[point].row][fighting_range[point].col];
						if(fighting[p][fighting_range[point].row][fighting_range[point].col] < min) min = fighting[p][fighting_range[point].row][fighting_range[point].col];
					}
				}
			}
			
			//cout<<"for "<<row<<" "<<col<<endl;
			if(fighting[0][row][col] == 0 || fighting[0][row][col] < min) status[row][col] = 1;
			else if(max != 0 && fighting[0][row][col] > max) status[row][col] = 0;
			else status[row][col] = 2+fighting[0][row][col]-min;
			//cout<<"end "<<row<<" "<<col<<endl;
		}
	}
	/*
	cout<<"influence 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<influence[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"influence 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<influence[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"total"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<total[r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"fighting 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<fighting[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"fighting 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<fighting[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"status"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<status[r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"occupy 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<occupy[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"occupy 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<occupy[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	*/
	//cout<<min<<" "<<max<<endl;
	//cout<<"all"<<endl;
	/*
	cout<<"status"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<status[r][c]<<"  ";
		}
		cout<<endl;
	}
	*/
}

void State::update_local_combat(const int &ant, int set_direction)
{
	Location loc = myAnts[ant];
	Location setLoc = getLocation(loc, set_direction);
	//std::vector<Location> wont_occupy;
	//wont_occupy.clear();
	
	//update_my_influence, occupy, total, fighting
	std::vector<std::vector<bool> > tried_area(rows, std::vector<bool>(cols, false));
	Location nLoc;
	for(int direction = 0; direction < TDIRECTIONS; direction++) //all directions
	{
		if(direction != set_direction)		//expect the set one
		{
			nLoc = getLocation(myAnts[ant], direction);
			
			if(!grid[nLoc.row][nLoc.col].isWater && occupy[0][nLoc.row][nLoc.col] == 1)		//nLoc can only be occupied by this
			{
				std::vector<Location> influ_points = points_in_rad(nLoc, attackradius);
				for(int point = 0; point < influ_points.size(); point++)
				{
					if(!tried_area[influ_points[point].row][influ_points[point].col])
					{
						tried_area[influ_points[point].row][influ_points[point].col] = true;
						if(distance(influ_points[point], setLoc) > attackradius)	//will no longer influence
						{
							influence[0][influ_points[point].row][influ_points[point].col] --;	//will not influence
							total[influ_points[point].row][influ_points[point].col] --;
							for(int p = 1; p < noPlayers; p++)		//only change how many other need to fight
							{
								if(occupy[p][influ_points[point].row][influ_points[point].col] != 0)		//fighting is reasonable only when there is an ant to fight
								{
									fighting[p][influ_points[point].row][influ_points[point].col] --;
								}
							}
						}
					}
				}
			}
			
			occupy[0][nLoc.row][nLoc.col] --;	//less ant can occupy that point
		}
	}
	
	//update local status
	std::vector<Location> update_region = points_in_rad(setLoc, 2*attackradius+4);	//only possible to change this region
	for(int i_loc =0; i_loc < (int)update_region.size(); i_loc++)
	{
		Location update_loc = update_region[i_loc];
		if(occupy[0][update_loc.row][update_loc.col]!=0)	//only consider points my ant can go
		{
			int min = 999999, max = 0;
			std::vector<Location> fighting_range = points_in_rad(update_loc, attackradius);
			for(int point = 0; point<(int)fighting_range.size(); point++)
			{
				for(int p = 1; p < noPlayers; p++)
				{
					if(occupy[p][fighting_range[point].row][fighting_range[point].col] != 0)
					{
						if(fighting[p][fighting_range[point].row][fighting_range[point].col] > max) max = fighting[p][fighting_range[point].row][fighting_range[point].col];
						if(fighting[p][fighting_range[point].row][fighting_range[point].col] < min) min = fighting[p][fighting_range[point].row][fighting_range[point].col];
					}
				}
			}
			
			//cout<<"for "<<row<<" "<<col<<endl;
			if(fighting[0][update_loc.row][update_loc.col] == 0 || fighting[0][update_loc.row][update_loc.col] < min) status[update_loc.row][update_loc.col] = 1;	//live
			else if(max != 0 && fighting[0][update_loc.row][update_loc.col] > max) status[update_loc.row][update_loc.col] = 0;	//die
			else status[update_loc.row][update_loc.col] = 2+fighting[0][update_loc.row][update_loc.col]-min;	//die but kill
			//cout<<"end "<<row<<" "<<col<<endl;
		}
	}
	/*
	cout<<"ant: "<<ant<<endl;
	cout<<"influence 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<influence[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"influence 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<influence[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"total"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<total[r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"fighting 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<fighting[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"fighting 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<fighting[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"status"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<status[r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"occupy 0"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<occupy[0][r][c]<<"  ";
		}
		cout<<endl;
	}
	cout<<"occupy 1"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<occupy[1][r][c]<<"  ";
		}
		cout<<endl;
	}
	*/
	/*
	cout<<"status"<<endl;
	cout<<"     14 15 16 17 18 19 20 21"<<endl;
	for(int r = 13; r<=17; r++)
	{
		cout<<r<<" : ";
		for(int c = 14; c<=21; c++)
		{
			cout<<status[r][c]<<"  ";
		}
		cout<<endl;
	}
	*/
}

bool State::bad_move(const int &ant, int direction)
{
	Location nLoc = getLocation(myAnts[ant], direction);
	if(status[nLoc.row][nLoc.col] == 0) return true;
	else if(status[nLoc.row][nLoc.col] == 1) return false;
	else
	{
		if(!sacrafice) return true;
		else if(sacrafice && status[nLoc.row][nLoc.col] == 2) return false;
	}
}

void State::update_home_info()
{
	for(int i_hill=0; i_hill<(int)myHills.size(); i_hill++)	//add new from myHills
	{
		if(std::find(home.begin(),home.end(),myHills[i_hill])==home.end())
		{
			home.push_back(myHills[i_hill]);
		}
	}
	
	for(int i_hill=0; i_hill<(int)home.size(); i_hill++)		//remove dead hills
	{
		Square hill_grid=grid[home[i_hill].row][home[i_hill].col];
		if(hill_grid.isVisible)
		{
			if((!hill_grid.isHill) || (hill_grid.ant!=-1 && hill_grid.ant!=hill_grid.hillPlayer))
			{
				home.erase(home.begin()+i_hill);
				//state.hills_owner.erase(state.hills_owner.begin()+i_hill);
				i_hill--;
			}
		}
	}
}

void State::update_hill_info()
{
	for(int i_hill=0; i_hill<(int)enemyHills.size(); i_hill++)	//add new from enemyHills
	{
		if(std::find(hills.begin(),hills.end(),enemyHills[i_hill])==hills.end())
		{
			hills.push_back(enemyHills[i_hill]);
			hills_owner.push_back(grid[enemyHills[i_hill].row][enemyHills[i_hill].col].hillPlayer);
		}
	}
	
	for(int i_hill=0; i_hill<(int)hills.size(); i_hill++)		//remove closed from hills
	{
		Square hill_grid=grid[hills[i_hill].row][hills[i_hill].col];
		if(hill_grid.isVisible)
		{
			if((!hill_grid.isHill) || (hill_grid.ant!=-1 && hill_grid.ant!=hill_grid.hillPlayer))
			{
				hills.erase(hills.begin()+i_hill);
				hills_owner.erase(hills_owner.begin()+i_hill);
				i_hill--;
			}
		}
	}
}

void State::update_unseen_points()
{
	for(int i_loc=0; i_loc<(int)unseen.size(); i_loc++)	//update unseen points
	{
		if(grid[unseen[i_loc].row][unseen[i_loc].col].isVisible)
		{
			unseen.erase(unseen.begin()+i_loc);
			seen_grid[unseen[i_loc].row][unseen[i_loc].col]=1;
			i_loc--;
		}
	}
}

/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.

    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for(int a=0; a<(int) myAnts.size(); a++)
    {
        sLoc = myAnts[a];
        locQueue.push(sLoc);

        std::vector<std::vector<bool> > visited(rows, std::vector<bool>(cols, 0));
        grid[sLoc.row][sLoc.col].isVisible = 1;
        visited[sLoc.row][sLoc.col] = 1;

        while(!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for(int d=0; d<TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);

                if(!visited[nLoc.row][nLoc.col] && distance(sLoc, nLoc) <= viewradius)
                {
                    grid[nLoc.row][nLoc.col].isVisible = 1;
                    locQueue.push(nLoc);
                }
                visited[nLoc.row][nLoc.col] = 1;
            }
        }
    }
};

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.

    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
    for(int row=0; row<state.rows; row++)
    {
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].isWater)
                os << '%';
            else if(state.grid[row][col].isFood)
                os << '*';
            else if(state.grid[row][col].isHill)
                os << (char)('A' + state.grid[row][col].hillPlayer);
            else if(state.grid[row][col].ant >= 0)
                os << (char)('a' + state.grid[row][col].ant);
            else if(state.grid[row][col].isVisible)
                os << '.';
            else
                os << '?';
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            state.gameover = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if(state.turn == 0)
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.loadtime;
            else if(inputType == "turntime")
                is >> state.turntime;
            else if(inputType == "rows")
                is >> state.rows;
            else if(inputType == "cols")
                is >> state.cols;
            else if(inputType == "turns")
                is >> state.turns;
			else if(inputType == "player_seed")
                is >> state.seed;
            else if(inputType == "players")
                is >> state.noPlayers;
            else if(inputType == "viewradius2")
            {
                is >> state.viewradius;
                state.viewradius = sqrt(state.viewradius);
            }
            else if(inputType == "attackradius2")
            {
                is >> state.attackradius;
                state.attackradius = sqrt(state.attackradius);
            }
            else if(inputType == "spawnradius2")
            {
                is >> state.spawnradius;
                state.spawnradius = sqrt(state.spawnradius);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer.start();
                break;
            }
            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.grid[row][col].isWater = 1;
            }
            else if(inputType == "f") //food square
            {
                is >> row >> col;
                state.grid[row][col].isFood = 1;
                state.food.push_back(Location(row, col));
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].ant = player;
                if(player == 0)
                    state.myAnts.push_back(Location(row, col));
                else
                    state.enemyAnts.push_back(Location(row, col));
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;
                state.grid[row][col].isHill = 1;
                state.grid[row][col].hillPlayer = player;
                if(player == 0)
                    state.myHills.push_back(Location(row, col));
                else
                    state.enemyHills.push_back(Location(row, col));

            }
            else if(inputType == "players") //player information
                is >> state.noPlayers;
            else if(inputType == "scores") //score information
            {
                state.scores = vector<double>(state.noPlayers, 0.0);
                for(int p=0; p<state.noPlayers; p++)
                    is >> state.scores[p];
            }
            else if(inputType == "go") //end of turn input
            {
                if(state.gameover)
                    is.setstate(std::ios::failbit);
                else
                    state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }

    return is;
};
