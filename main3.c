#include <stdio.h>
#define SIZE 8

#define WHITE 0
#define BLACK 1
#define QUIT 65

#define FAIL_MOVE 0
#define NORMAL_MOVE 1 
#define SPECIAL_MOVE 2 //rokkade, en passant. needed to indicate that special case piece-movement must occur

int current_team;
int turn;

typedef struct{
    int y, x;
} Coord;

typedef struct{
    char* id;
    Coord coords;
    int round_first_moved;
} Piece;


//have first be king
Piece white_p[SIZE+SIZE];
Piece black_p[SIZE+SIZE];

/*
    In special cases, return 2 like rokkade and en passant for pawn.
    Use this return value to move/kill the rook/pawn
*/

//necessary for moves that are defined before these functions yet dependent upon them
int tile_is_threatened(Coord tile, int team);
int can_move_according_to_type(Piece* p, Coord to);

/*
============== GLOBAL TEAM-FLAG RELATED ==============
*/

//change to other team
void change_current_team(){
    current_team = !current_team;
}

//chars that correspond to Pieces id-arrays first char, representing team black or white
char get_current_team_flag(){
    return  current_team ? 'B' : 'W';
}

int get_enemy(int team){
    if(team == WHITE){
        return BLACK;
    }
    return WHITE;
}

/*
============== TESTING ==============
*/

//temporary map-system for testing purposes
char* map [SIZE][SIZE];

//place string on test map
void place_str(int y, int x, char* str){
    map[y][x] = str;
}

//clear string from test map
void clear_tile(int y, int x){
    place_str(y,x,NULL);
}

//put piece on map
void place_piece(Piece p){
    place_str(p.coords.y, p.coords.x,p.id);
}

//map pieces string to maps coords and clear previous coords
void move_p(Coord new_coords, Piece* p){
    clear_tile(p->coords.y,p->coords.x);
    p->coords = new_coords;
    place_piece(*p);
}

//put team on map
void assemble(Piece* team){
    int i = 0;
    Piece i_p;
    while((i_p=team[i]).id!=NULL){
        place_piece(i_p);
        i++;
    }

}

//set up both teams piece-strings on the map
void assemble_teams(){
    assemble(white_p);
    assemble(black_p);
}

//find out how to pass map as an arg from main
void print_map(){
    int alternate = 0;

    for(int y = 0; y < SIZE; y++){
        for(int x = 0; x < SIZE; x++){
            if(map[y][x] != NULL){
                printf("%s", map[y][x]);
            }else{
                if(alternate)
                    printf(" - ");
                else
                    printf(" # ");
            }
            alternate = !alternate;
        }
        printf("\n");
    }
    printf("\n");
}

/*
============== INPUT RELATED ==============
*/

//maybe just a == b
int are_identical(Coord a, Coord b){
    return  a.y == b.y && a.x == b.x;
}

//checking that input is on the board or QUIT which signifies that user want to discontinue an on-going move
int valid_val(int val){
    return (val > 0 && val <= SIZE) || val == QUIT;
}

//gets either a coordinate or q for resetting the move-selection process
int get_input(){
    char val = 0; //if this is undefined it *could* result in valid_val before player gets to choose
    
    //we do (int) val - '0' so that if user types char '5' the int 5 is given to valid_val 
    while(!valid_val((val=((int) val - '0'))))
        val = getchar();
    return  val;
}

//gets the piece array for white or black depending upon arg
Piece* get_team(int black_team){
    return !black_team ? white_p : black_p;
}

Piece* get_piece(Coord c, int team){
    Piece* pieces = get_team(team);
    int i = 0;
    while(pieces[i].id!=NULL){
        if(are_identical(pieces[i].coords,c))
            return &pieces[i]; 
        i++;
    }
    return NULL;
}

Piece* get_any_piece(Coord c){
    Piece* p;
    if((p=get_piece(c,WHITE))!= NULL)
        return p;
    else if((p=get_piece(c,BLACK))!= NULL)
        return p;
    return NULL;
}

//why ought i use a pointer rather than just returning a new coord?
//because the whole coord must be return ed and then values must be checked for QUIT
int set_coord(Coord* coord){
    if((coord->y = get_input())==QUIT || (coord->x = get_input())==QUIT)
        return  0;
    return  1;
}

void do_current_team(){
    int done = 0;
    Coord selected_piece, target_tile;

    while(!done){        
        selected_piece = target_tile = (Coord){0,0};

        printf("Keep rolling rolling rolling roll-\n");

        if(!set_coord(&selected_piece) || get_piece(selected_piece,current_team) == NULL)
            continue;

        printf("Selected: %d and %d\n",selected_piece.y,selected_piece.x);

        if(!set_coord(&target_tile) || are_identical(target_tile,selected_piece))
            continue;

        printf("Target: %d and %d\n",target_tile.y,target_tile.x);

        done = 1;
    }

    //move(selected_piece,target_tile);
    //change_current_team();
}


/*
============== MOVEMENT RELATED ==============
*/

//used when determining distance between two points. we want to avoid negative values since smallest pos on map is 0,0
int subtract_smallest(int a, int  b){
    return a > b ? (a - b) : (b - a);
}

//is the point n tiles away?
int n_tiles_away(int a, int b, int n){
    return  subtract_smallest(a,b) == n;
}

//for determining whether a position is diagonal to another
int is_diagonaly_aligned(Coord a, Coord b){
    return subtract_smallest(a.y, b.y) == subtract_smallest(a.x,b.x);
}

//is the point n diagonal tiles away?
int n_diagonal_tiles_away(Coord a, Coord b, int n){
    return n_tiles_away(a.y,b.y,n) && n_tiles_away(a.x,b.x,n);
}

int n_straight_tiles_away(Coord a, Coord b, int n){
    return (n_tiles_away(a.y, b.y, n) && a.x == b.x) || (n_tiles_away(a.x, b.x, n) && a.y == b.y);
}

//for determining straight lines between two coords
int is_in_straight_line(Coord a, Coord b){
    return  a.y == b.y || a.x == b.x;
}

//for straight lines
//determining whether the y or x axis is where the line is drawn between two coords.
int get_changing_axis(Coord a, Coord b){
    return a.y != b.y ? a.x : a.y;
}

//makes sense for diagonal and straight line coords
int is_larger(Coord a, Coord b){
    if(a.y > b.y || a.x > b.x)
        return 1;
    return 0;
}

//Checks for obstruction between two points on the board
//only works for straight lines or diagonal lines(but only those are necessary)
int is_obstructed(Coord a, Coord b){
        
        Coord larger, smallest;

        if(is_larger(a,b)){
            larger = a;
            smallest = b;
        }else{
            larger = b;
            smallest = a;
        }

        int y = smallest.y;
        int x = smallest.x;
        
        while(y < larger.y || x < larger.x){
            
            if((y != smallest.y || x != smallest.x) && get_any_piece((Coord){y,x})!=NULL)
                return 1;

            if(y < larger.y)
                y++;

            if(x < larger.x)
                x++;
        }
        return 0;
}


//checks if one point is before another, relative to the team. 
int get_pawn_forward_path(int a, int b, int team){
    if(team)
        return  b > a;
    return  a > b;
}

//get the coord "behind" c(for en passant)
//what is behind varies from team to team, a positive increase on the y-axis for black, a negative decrease for white
Coord get_coord_behind(Coord c){
    return  (Coord){c.y + (current_team == WHITE ? 1 : -1),c.x}; 
}

//NB! valid turn for en passant must vary according to black or white!
//if there is an enemy pawn behind coord b who moved 2 steps this turn(should be last turn for black)
int is_en_passant(Coord b){
    Piece* p;
    Coord behind = get_coord_behind(b);
    //NB! Not tested with the new turn-settings
    return ((p=get_piece(behind,get_enemy(current_team))) != NULL && p->id[1] == 'P' && p->round_first_moved == turn);
}

int pawn_move(Piece* p, Coord b){

    //if position at b is in front of the pawn p indicating that the player wishes to move the pawn in a forward direction
    if(get_pawn_forward_path(p->coords.y,b.y,current_team)){

        //if the pawn wants to move diagonally one tile either to kill enemy there or to commit en passant on an enemy behind target tile b
        if(n_diagonal_tiles_away(p->coords,b,1) &&  (get_piece(b,!current_team) != NULL || is_en_passant(b))){
            return  SPECIAL_MOVE;

        //if its pawns first round and player wishes to move it 2 tiles by the y-axis
        }else if(p->round_first_moved == 0 && subtract_smallest(p->coords.y,b.y) == 2){
            p->round_first_moved = turn;
            return  NORMAL_MOVE;
        
        //if its regular one tile-forward move
        }else if(n_tiles_away(p->coords.y,b.y,1) && p->coords.x == b.x)
            return NORMAL_MOVE;
    }
    return  FAIL_MOVE;
}

//the first paranthesis clause is if horse moves upwards and to the side, the second is if it moves 3 sideways and 1 vertically
int horse_move(Piece* horse, Coord to){
    return  (n_tiles_away(horse->coords.y,to.y,3) && n_tiles_away(horse->coords.x,to.x,1)) || (n_tiles_away(horse->coords.y,to.y,1) && n_tiles_away(horse->coords.x,to.x,3));
}

int king_move(Piece* king, Coord to){

    if(n_diagonal_tiles_away(king->coords,to,1) || n_straight_tiles_away(king->coords, to,1)){
        king->round_first_moved == 0 ? turn : 0;
        return NORMAL_MOVE;
    }

    Piece* p;//potential_tower;
        
    //if the player do not wish to move the king back or forth and if the king has not moved this game
    //and if there is an unmoved tower at the requested position, this must mean that the player wishes to perform rokkade
    if(king->coords.y == to.y && king->round_first_moved == 0 && (p=get_piece(to,current_team))->id[1] == 'T' && p->round_first_moved == 0){
        int x = king->coords.x;

        while(x < to.x){
            if(tile_is_threatened((Coord){to.y,x}, get_enemy(current_team)))
                return FAIL_MOVE;
            x++;            
        }
        return SPECIAL_MOVE;
    }
    return FAIL_MOVE;
}

int bishop_move(Piece* bishop, Coord to){
    return is_diagonaly_aligned(bishop->coords, to);
}
int tower_move(Piece* tower, Coord to){
    //NB! replace with  n_straight_tiles_away, but do it "SIZE" for unlimited movement
    //return is_in_straight_line(tower->coords, to);
    return tower->coords.y == to.y || tower->coords.x == to.x;
}

char get_type(Piece* p){
    return p->id[1];
}

int can_move_according_to_type(Piece* p, Coord to){
    
    switch (get_type(p)){
        case 'T':
            return tower_move(p,to) && !is_obstructed(p->coords,to);
        
        case 'B':
            return bishop_move(p,to) && !is_obstructed(p->coords,to);
        
        case 'H':
            return horse_move(p,to);

        case 'K':
            //we want to return only king_move in case it returns a SPECIAL_MOVE value of 2 indicating rokkade
            if(!is_obstructed(p->coords,to))
                return king_move(p,to);
            break;

        case 'P':
            //we only want to return pawn_move in case it returns a SPECIAL_MOVE value of 2 indicating en passant
            if(!is_obstructed(p->coords,to))
                return pawn_move(p,to);
            break;
        default:
            return FAIL_MOVE;
    }
}

//return 1 if a soldier can threaten the tile
int tile_is_threatened(Coord tile, int team){
    Piece* p_set = get_team(team);
    Piece* p;
    int i = 0;

    while((p=&p_set[i])->id!= NULL){
        if(can_move_according_to_type(p,tile)){
            return 1;
        }
        i++;
    }
    return 0;
}

/*
    ############
    #~Next step:
    ############
        - implement movement for all soldier types
        - then do the rokkade (if all tiles to the tower is unobstructed *and* unthreatened, and if neither king nor tower has moved)
        - then conceptualize the check system (if any enemy soldier can threaten the king)
        - then conceptualize the check-mate system (if no possible move can be performed without resulting in a check)
*/

/*
============== MAIN ==============
*/

int main(int argc, char* args[]){
    current_team = WHITE;
    turn = 1;
    black_p[0]=(Piece){"BT1",(Coord){3,5},1};
    
    Coord target = {4,4};

    //white_p[0]=(Piece){"WT1",(Coord){2,4},0};
    white_p[0]=(Piece){"WT1",(Coord){7,7},0};
    white_p[1]=(Piece){"WK1",(Coord){7,4},0};
    //white_p[2]=(Piece){"WH1",(Coord){6,5},0};
    
    
    assemble_teams();
    print_map();

    printf("%d\n", king_move(&white_p[1],(Coord){7,7}));

    return  0;
} 
