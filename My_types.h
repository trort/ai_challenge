#ifndef MY_TYPES_H_
#define MY_TYPES_H_

#include "Location.h"
#include <vector>

/*
    types for my use
*/

struct ant_directions
{
	int ant;
	bool d[5];
	
	ant_directions(int a)
	{
		ant = a;
		d[0] = d[1] = d[2] = d[3] = d[4] = 0;
	}
	
	ant_directions()
	{
		ant = 0;
		d[0] = d[1] = d[2] = d[3] = d[4] = 0;
	}
};

struct ant_loc_pair
{
	Location loc;
	int dist;
	int ant;
	ant_directions ant_ds;
	
	ant_loc_pair(int a, Location &l)
	{
		ant = a;
		loc = l;
		dist = -1;
		ant_ds = ant_directions(a);
	}
};

struct Location_pair
{
	int ant;
	Location end;
	int dist;
	bool d[4];
	
	Location_pair(int a, Location f)
	{
		ant = a;
		end = f;
		dist = -1;
		d[0] = d[1] = d[2] = d[3] = d[4] = 0;
	}
};

#endif //MY_TYPES_H_
