#ifndef LOCATION_H_
#define LOCATION_H_

/*
    struct for representing locations in the grid.
*/
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(int r, int c)
    {
        row = r;
        col = c;
    };
	
	bool operator==(const Location &another){
		if(row==another.row && col==another.col)
			return true;
		else
			return false;
	};
};

#endif //LOCATION_H_
