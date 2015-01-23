#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <algorithm>
#include <stdint.h>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"
#include "My_types.h"

/*
    constants
*/
const int TDIRECTIONS = 5;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[5][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1}, {0, 0} };      //{N, E, S, W, Stay}

/*
    struct to store current state information
*/
struct State
{
    /*
        Variables
    */
    int rows, cols,
        turn, turns,
        noPlayers;
    double attackradius, spawnradius, viewradius;
    double loadtime, turntime;
    std::vector<double> scores;
    bool gameover;
    int64_t seed;

    std::vector<std::vector<Square> > grid;
    std::vector<Location> myAnts, enemyAnts, myHills, enemyHills, food;

    Timer timer;
    Bug bug;

    /*
        Functions
    */
    State();
    ~State();

    void setup();
    void reset();

    void makeMove(const Location &loc, int direction);

    double distance(const Location &loc1, const Location &loc2);
    Location getLocation(const Location &startLoc, int direction);

    void updateVisionInformation();
	
	//My part
	//int myHillNo;
	std::vector<std::vector<Location> > guard_points;
	bool sacrafice;
	
	std::vector<std::vector<bool> > orders;//save the locations after orders
	std::vector<bool> foods2go;//save the food targeted
	std::vector<bool> hills2go;//save the hills targeted
	std::vector<bool> ants2move;//ant [] goint to move or not
	std::vector<bool> enemies2attack;//enemies planned to attack
	std::vector<std::vector<int> > my_ant_map;
	std::vector<std::vector<int> > food_map;
	std::vector<std::vector<int> > hill_map;
	std::vector<std::vector<int> > enemy_map;
	
	int ants_available;
	
	//for combat
	std::vector<std::vector<std::vector<int> > > influence;
	std::vector<std::vector<int> > total;
	std::vector<std::vector<std::vector<int> > > fighting;
	std::vector<std::vector<std::vector<int> > > occupy;
	std::vector<std::vector<int> > status;	//0=die, 1=live, 2=exchange, 3+ =die, one enemy die, third alive, only for my ants
	
	std::vector<Location> unseen;//unseen points
	std::vector<Location> hills;//seen enemy hills
	std::vector<Location> home;//my hills
	std::vector<int> hills_owner;//owners of the hills
	std::vector<std::vector<bool> > seen_grid;//quick check for if a grid was seen
	
	std::vector<Location> tower_loc;//points where i should keep ants nearby
	std::vector<Location> hack_loc;//only the towers visited
	
	//MY functions
	bool bad_move(const int &ant, int direction);
	std::vector<Location> points_in_rad(const Location &loc, double rad);//points in rad
	
	void update_guard_points();
	void update_player_no();
	void update_combat_info();
	void update_local_combat(const int &ant, int set_direction);
	
	bool random_move(const int &ant);
	bool makeMove_direction(const int &ant, int direction);
	bool makeMove_ant_directions(const ant_directions &ant_ds);
	
	//initial functions
	void update_home_info();	//my hills
	void update_hill_info();	//enemy hills
	void update_unseen_points();
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
