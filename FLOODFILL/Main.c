#include <stdio.h>

#include "API.h"


#define DIMENSIONS_X   16
#define DIMENSIONS_Y   16

#define TARG_X   8
#define TARG_Y   8

#define HOME_X     0
#define HOME_Y     0

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAX4(a,b,c,d) MAX(MAX(a,b),MAX(c,d))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MIN4(a,b,c,d) MIN(MIN(a,b),MIN(c,d))

typedef unsigned char  uint8_t;
typedef unsigned int   uint16_t;
#define true 1
#define false 0
typedef unsigned char  bool;

uint8_t                 by_TARG_X;
uint8_t                 by_TARG_Y;

typedef enum {
    MAZE_WALL_PRESENT = 0,
    MAZE_WALL_ABSENT,
    MAZE_WALL_UNKNOWN,
} WallState;

//Rat Direction Enum
typedef enum {
    NORTH = 0,
    EAST,
    SOUTH,
    WEST,
} RatDirection;

// Define the maze cells
typedef struct 
{
    uint8_t Weight;
    uint16_t History;
    bool UpdateFlag;
} MyMaze_Cells;

// Define the rat co-ordinates
typedef struct 
{
    uint8_t x;
    uint8_t y;
    RatDirection heading;
} MyRat_Co_ordinates;

//Maze Wall - Horizontal
WallState       Wall_H[DIMENSIONS_X+1][DIMENSIONS_Y];
//Maze Wall - Vertical
WallState       Wall_V[DIMENSIONS_X][DIMENSIONS_Y+1];
//Maze Cells
MyMaze_Cells    Maze_Cells[DIMENSIONS_X][DIMENSIONS_Y];
//Rat Coordinates
MyRat_Co_ordinates myRat;

// Declare a fixed-size buffer
char buffer[100];


typedef enum 
{
MAZE_CALIBRATE,                  
MAZE_EXPLORE,                   
MAZE_RETURN_HOME,                 
MAZE_FEED_THE_RAT_SOME_CHEESE,    
}Maze_State_E; 


Maze_State_E                  Maze_State                           = MAZE_CALIBRATE;


void initializeMaze_H(void); 
void initializeMaze_V(void); 
void Maze_Init(void);
void Maze_Solve(void);
void Maze_Reset(void);
void Maze_SetTarget(void);
void Maze_FloodFill( int, int);
void Flood_Fill_Value(int, int, int);
void Maze_StoreWalls(void);
void Maze_DisplayWalls(void);
RatDirection Rat_FindDirection(void);
void Rat_TurnLeft(void);
void Rat_TurnRight(void);
void Rat_MoveForward(void);


void log_out(char* text) 
{
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}

// Function to Reset the Maze Cells
void Maze_Reset()
{
    for (int i = 0; i < DIMENSIONS_X; i++) 
    {
        for (int j = 0; j < DIMENSIONS_Y; j++) 
        {
            Maze_Cells[i][j].Weight     = 255;
            Maze_Cells[i][j].History    = 0;
            Maze_Cells[i][j].UpdateFlag = false;
            //Display the value
            snprintf(buffer, sizeof(buffer), "%d", Maze_Cells[i][j].Weight);
            API_setText(i, j, buffer);
        }
    }    
    //log_out("MESSAGE - Maze_Reset");
}

// Function to initialize the maze to MAZE_WALL_UNKNOWN
void initializeMaze_H() 
{
    for (int i = 0; i < (DIMENSIONS_X+1); i++) 
    {
        for (int j = 0; j < DIMENSIONS_Y; j++) 
        {
            Wall_H[i][j] = MAZE_WALL_UNKNOWN;
        }
    }
}
void initializeMaze_V() 
{
    for (int i = 0; i < DIMENSIONS_X; i++) 
    {
        for (int j = 0; j < (DIMENSIONS_Y+1); j++) 
        {
            Wall_V[i][j] = MAZE_WALL_UNKNOWN;
        }
    }
}

// Function to mark all MAZE_WALL_UNKNOWN to MAZE_WALL_PRESENT
void normalize_Maze()
{

    for (int i = 0; i < (DIMENSIONS_X+1); i++) 
    {
        for (int j = 0; j < DIMENSIONS_Y; j++) 
        {
            if(Wall_H[i][j] == MAZE_WALL_UNKNOWN)
            {
                Wall_H[i][j] = MAZE_WALL_PRESENT;
            }
        }
    }

    for (int i = 0; i < DIMENSIONS_X; i++) 
    {
        for (int j = 0; j < (DIMENSIONS_Y+1); j++) 
        {
            if(Wall_V[i][j] == MAZE_WALL_UNKNOWN)
            {
                Wall_V[i][j] = MAZE_WALL_PRESENT;
            }
        }
    }

    Maze_DisplayWalls();

    sprintf(buffer, "MESSAGE - Maze Solved and Normalized");
    log_out(buffer);     

}


// Update rat heading after a left turn
void Rat_TurnLeft()
{
    if(myRat.heading == NORTH)
        myRat.heading = WEST;
    else
    if(myRat.heading == WEST)
        myRat.heading = SOUTH;
    else
    if(myRat.heading == SOUTH)
        myRat.heading = EAST;
    else
    if(myRat.heading == EAST)
        myRat.heading = NORTH;
}

// Update rat heading after a Right turn
void Rat_TurnRight()
{
    if(myRat.heading == NORTH)
        myRat.heading = EAST;
    else
    if(myRat.heading == EAST)
        myRat.heading = SOUTH;
    else
    if(myRat.heading == SOUTH)
        myRat.heading = WEST;
    else
    if(myRat.heading == WEST)
        myRat.heading = NORTH;
}

//Update the rat X and Y
void Rat_MoveForward()
{
    if(myRat.heading == NORTH)
    {
        myRat.x = myRat.x;
        myRat.y = myRat.y+1;
    }
        
    if(myRat.heading == EAST)
    {
        myRat.x = myRat.x+1;
        myRat.y = myRat.y;
    }

    if(myRat.heading == SOUTH)
    {
        myRat.x = myRat.x;
        myRat.y = myRat.y-1;
    }
    
    if(myRat.heading == WEST)
    {
        myRat.x = myRat.x-1;
        myRat.y = myRat.y;
    }
    
}


// Store the walls to the wall arrays
void Maze_StoreWalls(void)
{
    if(myRat.heading == NORTH)
    {
        if(API_wallLeft())
        {
            //Maze West
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - NORTH : West"); 
        }
        else
        {
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }
        if(API_wallRight())
        {
            //Maze East
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - NORTH : East");
        }
        else
        {
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_ABSENT;
        }        
        if(API_wallFront())
        {
            //Maze North
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - NORTH : North");
        }
        else
        {
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_ABSENT;
        }
    }

    if(myRat.heading == SOUTH)
    {
        if(API_wallLeft())
        {
            //Maze East
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - SOUTH : East");
        }
        else
        {
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_ABSENT;
        }
        if(API_wallRight())
        {
            //Maze West
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - SOUTH : West"); 
        }
        else
        {
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }
        if(API_wallFront())
        {
            //Maze South 
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - SOUTH : South");
        }        
        else
        {
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }
    }

    if(myRat.heading == EAST)
    {
        if(API_wallLeft())
        {
            //Maze North
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - EAST  : North");
        }
        else
        {
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_ABSENT;
        }
        if(API_wallRight())
        {
            //Maze South 
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - EAST  : South");
        }        
        else
        {
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }
        if(API_wallFront())
        {
            //Maze East
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - EAST  : East");
        }
        else
        {
            Wall_H[myRat.x+1][myRat.y] = MAZE_WALL_ABSENT;
        }
    }    

    if(myRat.heading == WEST)
    {
        if(API_wallLeft())
        {
            //Maze South 
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - WEST  : South");
        }        
        else
        {
            Wall_V[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }       
        if(API_wallRight())
        {
            //Maze North
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - WEST  : North");
        }
        else
        {
            Wall_V[myRat.x][myRat.y+1] = MAZE_WALL_ABSENT;
        }
        if(API_wallFront())
        {
            //Maze West
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_PRESENT;
            //log_out("MESSAGE - WALL - WEST  : West"); 
        }
        else
        {
            Wall_H[myRat.x][myRat.y] = MAZE_WALL_ABSENT;
        }
    }

}


//Display Walls on the maze
void Maze_DisplayWalls()
{
    //Horizontal
    for (int i = 0; i < DIMENSIONS_X+1; i++) 
    {
        for (int j = 0; j < DIMENSIONS_Y; j++) 
        {
            //East wall
            if(Wall_H[i+1][j] == MAZE_WALL_PRESENT)
            {
                API_setWall(i, j, 'e');                        
                //log_out("MESSAGE - DISP - EAST");  
                //sprintf(buffer, "MESSAGE - DispWall - East - (i, j) = (%d,%d)", i, j);
                //log_out(buffer);                
            }

            //West wall
            if(Wall_H[i][j] == MAZE_WALL_PRESENT)
            {
                API_setWall(i, j, 'w');
                //sprintf(buffer, "MESSAGE - DispWall - West - (i, j) = (%d,%d)", i, j);
                //log_out("MESSAGE - DISP - WEST");  
            }
        }
    }

    //Vertical
    for (int i = 0; i < DIMENSIONS_X; i++) 
    {
        for (int j = 0; j < DIMENSIONS_Y+1; j++) 
        {
            //North wall
            if(Wall_V[i][j+1] == MAZE_WALL_PRESENT)
            {
                API_setWall(i, j, 'n');
                //log_out("MESSAGE - DISP - NORTH");  
            }

            //South wall
            if(Wall_V[i][j] == MAZE_WALL_PRESENT)
            {
                API_setWall(i, j, 's');  
                //log_out("MESSAGE - DISP - SOUTH");  
            }
        }
    }    

    //log_out("MESSAGE - Maze_DisplayWalls");    

}


// Find the neighbour with the max value
void Flood_Fill_Value(int x, int y, int loopSize)
{
    int Least_Neighbour;
    uint8_t x_plus;
    uint8_t y_plus;
    uint8_t x_minus;
    uint8_t y_minus;

    //Has it already been updated?
    //if(Maze_Cells[x][y].UpdateFlag == false)
    {

        //Manage Maze borders
        x_plus  = constrain((x+1) , 0, (DIMENSIONS_X-1));
        y_plus  = constrain((y+1) , 0, (DIMENSIONS_Y-1)); 
        x_minus = constrain((x-1) , 0, (DIMENSIONS_X-1));
        y_minus = constrain((y-1) , 0, (DIMENSIONS_Y-1));   

        if(loopSize == 2)
        {
            //sprintf(buffer, "MESSAGE - Maze_Cells[x][y].Weight       = %d" , Maze_Cells[x][y].Weight);
            //log_out(buffer);                
        }

        //Initialize Least_Neighbour
        Least_Neighbour = Maze_Cells[x][y].Weight;

        //Maze North
        if(Wall_V[x][y_plus] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Least_Neighbour, Maze_Cells[x][y_plus].Weight);

            if(loopSize == 2)
            {
                //sprintf(buffer, "MESSAGE - Maze_Cells[x][y_plus].Weight  = %d , Least_Neighbour = %d" , Maze_Cells[x][y_plus].Weight, Least_Neighbour);
                //log_out(buffer);                
            }
        }

        //Maze South
        if(Wall_V[x][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Least_Neighbour, Maze_Cells[x][y_minus].Weight);
            if(loopSize == 2)
            {
                //sprintf(buffer, "MESSAGE - Maze_Cells[x][y_minus].Weight = %d , Least_Neighbour = %d" , Maze_Cells[x][y_minus].Weight, Least_Neighbour);
                //log_out(buffer);                
            }            
        }

        //Maze East
        if(Wall_H[x_plus][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Least_Neighbour, Maze_Cells[x_plus][y].Weight);
            if(loopSize == 2)
            {
                //sprintf(buffer, "MESSAGE - Maze_Cells[x_plus][y].Weight  = %d , Least_Neighbour = %d" , Maze_Cells[x_plus][y].Weight, Least_Neighbour);
                //log_out(buffer);                
            }            
        }

        //Maze West
        if(Wall_H[x][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Least_Neighbour, Maze_Cells[x_minus][y].Weight);
            if(loopSize == 2)
            {
                //sprintf(buffer, "MESSAGE - Maze_Cells[x_minus][y].Weight = %d , Least_Neighbour = %d" , Maze_Cells[x_minus][y].Weight, Least_Neighbour);
                //log_out(buffer);                
            }            
        }

        if(loopSize == 2)
        {
            //sprintf(buffer, "MESSAGE - Cell(%d,%d) Least_Neighbour = %d" , x, y, Least_Neighbour);
            //log_out(buffer);
        }

        //Since we are looping outwards, always put in least value
        if(Least_Neighbour < Maze_Cells[x][y].Weight)    
        {
            Maze_Cells[x][y].Weight = Least_Neighbour + 1;
            Maze_Cells[x][y].UpdateFlag = true; 
            //sprintf(buffer, "MESSAGE - Least_Neighbour = %d" , Least_Neighbour);
            //log_out(buffer);            
        }

        //Display the value
        snprintf(buffer, sizeof(buffer), "%d", Maze_Cells[x][y].Weight);
        API_setText(x, y, buffer);
    }
}


//Maze FloodFill
void Maze_FloodFill(int x, int y)
{
    //The passed params set the square we start floodfilling from
    bool    byReturnFlag = true;
    uint8_t byLeastValue = 255;

    //Loop params for flood fill
    uint8_t loopSize;
    uint8_t loopLimit;
    uint8_t x_Local;
    uint8_t y_Local;

    //Sanity check
    if( (x < DIMENSIONS_X) && (y < DIMENSIONS_Y) )
    {
        //Set Target Square
        Maze_Cells[x][y].Weight     = 0;
        snprintf(buffer, sizeof(buffer), "%d", Maze_Cells[x][y].Weight);
        API_setText(x, y, buffer); 

        //We start with 1x1 squares
        loopSize = 2;

        //How many loops need to be executed
        //Is the max distance from any side
        loopLimit = MAX4(x, y, (DIMENSIONS_X - x - 1), (DIMENSIONS_Y - y - 1));
        //sprintf(buffer, "MESSAGE - loopLimit = %d", loopLimit);
        //log_out(buffer); 

        for(int i = 1 ; i <= loopLimit ; i++)
        {
            //sprintf(buffer, "MESSAGE - loopSize = %d", loopSize);
            //log_out(buffer); 

            //North
            //Prime the loopers
            x_Local = constrain((x-i) , 0, (DIMENSIONS_X-1));
            y_Local = constrain((y-i) , 0, (DIMENSIONS_Y-1));            
            //Loop
            for(int j = 0 ; j < loopSize ; j++)
            {
                //Move to next cell
                y_Local = y_Local + 1;
                x_Local = constrain(x_Local , 0, (DIMENSIONS_X-1));
                y_Local = constrain(y_Local , 0, (DIMENSIONS_X-1));

                //Update Cell Value
                Flood_Fill_Value(x_Local, y_Local, loopSize);

                //Debug Messages
                //API_setColor(x_Local, y_Local, 'o'); 
                //sprintf(buffer, "MESSAGE - North - (x_Local, y_Local) = (%d,%d)", x_Local, y_Local);
                //log_out(buffer);
            }

            //East
            //Prime the loopers
            x_Local = constrain((x-i)   , 0, (DIMENSIONS_X-1));
            y_Local = constrain((y+i) , 0, (DIMENSIONS_Y-1));            
            //Loop
            for(int j = 0 ; j < loopSize ; j++)
            {
                //Move to next cell
                x_Local = x_Local + 1;
                x_Local = constrain(x_Local , 0, (DIMENSIONS_X-1));
                y_Local = constrain(y_Local , 0, (DIMENSIONS_X-1));

                //Update Cell Value
                Flood_Fill_Value(x_Local, y_Local, loopSize);

                //Debug Messages
                //API_setColor(x_Local, y_Local, 'c'); 
                //sprintf(buffer, "MESSAGE - East  - (x_Local, y_Local) = (%d,%d)", x_Local, y_Local);
                //log_out(buffer);
            }

            //South
            //Prime the loopers
            x_Local = constrain((x+i) , 0, (DIMENSIONS_X-1));
            y_Local = constrain((y+i) , 0, (DIMENSIONS_Y-1));            
            //Loop
            for(int j = 0 ; j < loopSize ; j++)
            {
                //Move to next cell
                y_Local = y_Local - 1;
                x_Local = constrain(x_Local , 0, (DIMENSIONS_X-1));
                y_Local = constrain(y_Local , 0, (DIMENSIONS_X-1));

                //Update Cell Value
                Flood_Fill_Value(x_Local, y_Local, loopSize);
                
                //Debug Messages
                //API_setColor(x_Local, y_Local, 'y'); 
                //sprintf(buffer, "MESSAGE - South - (x_Local, y_Local) = (%d,%d)", x_Local, y_Local);
                //log_out(buffer);
            }

            //East
            //Prime the loopers
            x_Local = constrain((x+i) , 0, (DIMENSIONS_X-1));
            y_Local = constrain((y-i) , 0, (DIMENSIONS_Y-1));            
            //Loop
            for(int j = 0 ; j < loopSize ; j++)
            {
                //Move to next cell
                x_Local = x_Local - 1;
                x_Local = constrain(x_Local , 0, (DIMENSIONS_X-1));
                y_Local = constrain(y_Local , 0, (DIMENSIONS_X-1));

                //Update Cell Value
                Flood_Fill_Value(x_Local, y_Local, loopSize);
                
                //Debug Messages
                //API_setColor(x_Local, y_Local, 'g'); 
                //sprintf(buffer, "MESSAGE - East  - (x_Local, y_Local) = (%d,%d)", x_Local, y_Local);
                //log_out(buffer);
            }

            //Move to next loop
            loopSize = loopSize + 2;            
        }

        //Mark the target square color
        API_setColor(x, y, 'a');   
    }
}



//Find the direction the least value is in
RatDirection Rat_FindDirection()
{
    RatDirection wReturn;

    int Least_Neighbour;
    uint8_t x;
    uint8_t y;
    uint8_t x_plus;
    uint8_t y_plus;
    uint8_t x_minus;
    uint8_t y_minus;

    //Has it already been updated?
    //if(Maze_Cells[x][y].UpdateFlag == false)
    {

        //Manage Maze borders
        x       = myRat.x;
        y       = myRat.y;
        x_plus  = constrain((x+1) , 0, (DIMENSIONS_X-1));
        y_plus  = constrain((y+1) , 0, (DIMENSIONS_Y-1)); 
        x_minus = constrain((x-1) , 0, (DIMENSIONS_X-1));
        y_minus = constrain((y-1) , 0, (DIMENSIONS_Y-1));   

        //Initialize Least_Neighbour
        Least_Neighbour = Maze_Cells[x][y].Weight;

        //Maze North
        if(Wall_V[x][y_plus] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Maze_Cells[x][y].Weight, Maze_Cells[x][y_plus].Weight);
            
            sprintf(buffer, "MESSAGE - Rat_FindDirection - NORTH - (Least_Neighbour, Maze_Cells[x][y].Weight) = (%d,%d)", Least_Neighbour, Maze_Cells[x][y].Weight);
            //log_out(buffer);     

            if(Least_Neighbour < Maze_Cells[x][y].Weight)
            {
                wReturn = NORTH; 
                sprintf(buffer, "MESSAGE - Rat_FindDirection - NORTH - Maze_Cells[x][y_plus].Weight = %d", Maze_Cells[x][y_plus].Weight);                   
                //log_out(buffer);
            }
        }

        //Maze South
        if(Wall_V[x][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Maze_Cells[x][y].Weight, Maze_Cells[x][y_minus].Weight);

            sprintf(buffer, "MESSAGE - Rat_FindDirection - SOUTH - (Least_Neighbour, Maze_Cells[x][y].Weight) = (%d,%d)", Least_Neighbour, Maze_Cells[x][y].Weight);
            //log_out(buffer);    

            if(Least_Neighbour < Maze_Cells[x][y].Weight)
            {
                wReturn = SOUTH;          
                sprintf(buffer, "MESSAGE - Rat_FindDirection - SOUTH - Maze_Cells[x][y_minus].Weight = %d", Maze_Cells[x][y_minus].Weight);     
                //log_out(buffer);     
            }         
        }

        //Maze East
        if(Wall_H[x_plus][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Maze_Cells[x][y].Weight, Maze_Cells[x_plus][y].Weight);

            sprintf(buffer, "MESSAGE - Rat_FindDirection - EAST  - (Least_Neighbour, Maze_Cells[x][y].Weight) = (%d,%d)", Least_Neighbour, Maze_Cells[x][y].Weight);
            //log_out(buffer);    

            if(Least_Neighbour < Maze_Cells[x][y].Weight)
            {
                wReturn = EAST;               
                sprintf(buffer, "MESSAGE - Rat_FindDirection - EAST - Maze_Cells[x_plus][y].Weight = %d", Maze_Cells[x_plus][y].Weight);     
                //log_out(buffer);
            }           
        }

        //Maze West
        if(Wall_H[x][y] != MAZE_WALL_PRESENT)
        {
            //Least_Neighbour already has the local value
            Least_Neighbour = MIN(Maze_Cells[x][y].Weight, Maze_Cells[x_minus][y].Weight);

            sprintf(buffer, "MESSAGE - Rat_FindDirection - WEST  - (Least_Neighbour, Maze_Cells[x][y].Weight) = (%d,%d)", Least_Neighbour, Maze_Cells[x][y].Weight);
            //log_out(buffer);    

            if(Least_Neighbour < Maze_Cells[x][y].Weight)
            {
                wReturn = WEST;               
                sprintf(buffer, "MESSAGE - Rat_FindDirection - WEST - Maze_Cells[x_minus][y].Weight = %d", Maze_Cells[x_minus][y].Weight);     
                //log_out(buffer);
            }           
        }
    }    

    return wReturn;
}


//mms Init function
void Maze_Init()
{
    //Set the Rat Starting Square here
    myRat.x = 0;
    myRat.y = 0;
    myRat.heading = NORTH;

    //init arrays
    log_out("MESSAGE - Init - Wall_H");
    initializeMaze_H();
    log_out("MESSAGE - Init - Wall_V");
    initializeMaze_V();
    //Reset Maze
    Maze_Reset();
    //Set the first Floodfill to target the center
    by_TARG_X = TARG_X;
    by_TARG_Y = TARG_Y;
    Maze_FloodFill(by_TARG_X, by_TARG_Y);
}



//mms Main function
int main(int argc, char* argv[]) 
{

    if(Maze_State == MAZE_CALIBRATE)
    {
        //Init the Maze
        Maze_Init();

        //Target the center
        by_TARG_X = TARG_X;
        by_TARG_Y = TARG_Y;

        Maze_State = MAZE_EXPLORE;
    }

    if(Maze_State == MAZE_EXPLORE)
    {
        //Solve the maze
        Maze_Solve();

        if((myRat.x == by_TARG_X) && (myRat.y == by_TARG_Y))
        {
            //Parse the maze and mark all unknown walls as present
            normalize_Maze();    

            //Go Home
            by_TARG_X = HOME_X;
            by_TARG_Y = HOME_Y;

            //Flood fill maze
            //update maze values
            Maze_Reset();
            //for(int i = 0 ; i < MAX(DIMENSIONS_X, DIMENSIONS_Y); i++)
            while (Maze_Cells[myRat.x][myRat.y].Weight == 255)
            {
                Maze_FloodFill(by_TARG_X, by_TARG_Y);
            }

            //EXIT STATE_RUN
            Maze_State = MAZE_RETURN_HOME;    
        }  
    }

    if(Maze_State == MAZE_RETURN_HOME)
    {

        //Solve the maze
        Maze_Solve();

        if((myRat.x == by_TARG_X) && (myRat.y == by_TARG_Y))
        {
            //Target the center
            by_TARG_X = TARG_X;
            by_TARG_Y = TARG_Y;
         
            //Flood fill maze
            //update maze values
            Maze_Reset();
            //for(int i = 0 ; i < MAX(DIMENSIONS_X, DIMENSIONS_Y); i++)
            while (Maze_Cells[myRat.x][myRat.y].Weight == 255)
            {
                Maze_FloodFill(by_TARG_X, by_TARG_Y);
            }         

            //EXIT STATE_RUN
            Maze_State = MAZE_FEED_THE_RAT_SOME_CHEESE;    
        }  
    }

    if(Maze_State == MAZE_FEED_THE_RAT_SOME_CHEESE)
    {
        //Solve the maze
        Maze_Solve();

        if((myRat.x == by_TARG_X) && (myRat.y == by_TARG_Y))
        {
            //EXIT STATE_RUN
            return 0; 
        }  
    }    

}


//mms Main function
void Maze_Solve()
{
    log_out("Running...");

    RatDirection RetDesiredDirection;

    while (!((myRat.x == by_TARG_X) && (myRat.y == by_TARG_Y))) 
    {
        if(Maze_State == MAZE_EXPLORE)
        {
            //Store the current state
            Maze_StoreWalls();
            Maze_DisplayWalls();

            //update maze values
            Maze_Reset();
            //for(int i = 0 ; i < MAX(DIMENSIONS_X, DIMENSIONS_Y); i++)
            while (Maze_Cells[myRat.x][myRat.y].Weight == 255)
            {
                Maze_FloodFill(by_TARG_X, by_TARG_Y);
            }
        }

        //Find the best path based on flood fill
        RetDesiredDirection = Rat_FindDirection();
        
        //Correct direction if we not on the right heading
        if(myRat.heading!=RetDesiredDirection)
        {
            if(RetDesiredDirection == NORTH)
            {
                if(myRat.heading == NORTH)
                {
                    ;//Not possible to reach here
                }
                else
                if(myRat.heading == EAST)
                {
                    API_turnLeft();
                    Rat_TurnLeft();                    
                }
                else
                if(myRat.heading == SOUTH)
                {
                    API_turnRight();
                    Rat_TurnRight();
                    API_turnRight();
                    Rat_TurnRight();                    
                }
                else
                if(myRat.heading == WEST)
                {
                    API_turnRight();
                    Rat_TurnRight();   
                }
                sprintf(buffer, "MESSAGE - Rat Cell - (x, y) = (%d,%d) - NORTH", myRat.x, myRat.y);
            }
            else
            if(RetDesiredDirection == EAST)
            {
                if(myRat.heading == NORTH)
                {
                    API_turnRight();
                    Rat_TurnRight();
                }
                else
                if(myRat.heading == EAST)
                {
                    ;//Not possible to reach here
                }
                else
                if(myRat.heading == SOUTH)
                {
                    API_turnLeft();
                    Rat_TurnLeft();                                        
                }
                else
                if(myRat.heading == WEST)
                {
                    API_turnRight();
                    Rat_TurnRight();
                    API_turnRight();
                    Rat_TurnRight();            
                }                
                sprintf(buffer, "MESSAGE - Rat Cell - (x, y) = (%d,%d) - EAST", myRat.x, myRat.y);
            }
            else
            if(RetDesiredDirection == SOUTH)
            {   
                if(myRat.heading == NORTH)
                {
                    API_turnRight();
                    Rat_TurnRight();
                    API_turnRight();
                    Rat_TurnRight();                                                    
                }
                else
                if(myRat.heading == EAST)
                {
                    API_turnRight();
                    Rat_TurnRight();
                }
                else
                if(myRat.heading == SOUTH)
                {
                    ;//Not possible to reach here
                }
                else
                if(myRat.heading == WEST)
                {
                    API_turnLeft();
                    Rat_TurnLeft();  
                } 
                sprintf(buffer, "MESSAGE - Rat Cell - (x, y) = (%d,%d) - SOUTH", myRat.x, myRat.y);
            }
            else
            if(RetDesiredDirection == WEST)
            {
                if(myRat.heading == NORTH)
                {
                    API_turnLeft();
                    Rat_TurnLeft(); 
                }
                else
                if(myRat.heading == EAST)
                {
                    API_turnRight();
                    Rat_TurnRight();
                    API_turnRight();
                    Rat_TurnRight();                     
                }
                else
                if(myRat.heading == SOUTH)
                {
                    API_turnRight();
                    Rat_TurnRight();
                }
                else
                if(myRat.heading == WEST)
                {
                    ;//Not possible to reach here           
                }                  
                sprintf(buffer, "MESSAGE - Rat Cell - (x, y) = (%d,%d) - WEST", myRat.x, myRat.y);
            }

            log_out(buffer);
        }

        //Proceed if we are in the right heading
        if(myRat.heading == RetDesiredDirection)
        {
            //Move the robot forward the specified number
            API_moveForward();
            Rat_MoveForward();
            sprintf(buffer, "MESSAGE - Rat Cell - (x, y) = (%d,%d) - FORWARD", myRat.x, myRat.y);    
            log_out(buffer);
        }        

    }

}