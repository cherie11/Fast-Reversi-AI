#include <iostream>
#include <stdint.h>

#include <algorithm>
#include <vector>
#include <map>
#include <tuple>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <array>
#define LSB_HASH  0x07EDD5E59A4E28C2
#define FULL_MASK 0xFFFFFFFFFFFFFFFF
#define P_CORNER 0x8100000000000081
#define P_SUB_CORNER 0x42C300000000C342 
/*
 * pos: position of the chess on the board
 */
struct pos{
public:
    int x = 0;
    int y = 0;
    pos(int xin,int yin){
        x = xin;
        y = yin;
    }
    
    ~pos(){}
};
/*
 * board: include two uint64_t variable. show the black keys and white keys
 */
struct board{
public:
    uint64_t B = 0;
    uint64_t W = 0;
    board(uint64_t Bin, uint64_t Win){
        B = Bin;
        W = Win;
    }
    board(){
        
    }
    ~board(){}
};

/*
 * rootstruct: node of Mente Carlo Treet 
 * color: {-1, 1} which color of the key will be set in this node
 * count: visit times
 * eval: winning rate
 */
typedef struct rootstruct{
public:
    struct rootstruct * parentState;
    std::map<uint64_t ,struct rootstruct * > childState;
    int color;
    int count;
    double eval;
    board curBoard;
    rootstruct(){
        this-> childState = std::map<uint64_t, struct rootstruct *>();
        this-> color = 0;   
        this-> count = 0;
        this-> eval = 0.0;
        this-> curBoard = board();
        this-> parentState = nullptr;
    }
} *root;

class MCTS {
private:
    const double lamda = 1;
    const double gamma = 0.01;

    // for evaluating the current situation
    int WEIGHTS[7] = {-3, -7, 11, -4, 8, 1, 2};
    uint64_t P_RINGS[7] = {0x4281001818008142,
               0x42000000004200,
               0x2400810000810024,
               0x24420000422400,
               0x1800008181000018,
               0x18004242001800,
               0x3C24243C0000};
    // overall search
    pos MCTSearch(root state_node);
    
    // check whether the game is end: neither of two players could move.
    bool checkEnd(board myboard);
    
    // get final grade by counting the number of keys
    double getFinalVal(uint64_t P, uint64_t O, int color);
    
    // backup the value towards root
    void backup(root state, double reward);
    
    // expand the node with the movement mv
    root expand(uint64_t mv, root state_node);
    
    // choose bestChild from the child list
    std::tuple<uint64_t , root> bestChild(root state, double C);
    
    // do move
    // return mask: binary representation for tile flipped
    uint64_t flip(uint64_t W, uint64_t B, uint64_t mv);

    // evaluate the position
    double calvalue(uint64_t , uint64_t , uint64_t);
    
    //search for 1 direction
    uint64_t move_gen_sub(uint64_t P, uint64_t mask, int dir);
    
    // find the legal moves
    uint64_t move_gen(uint64_t P, uint64_t O);
    
    // count the number of keys on the board
    uint64_t count_bit(uint64_t b);
    
    // choose the node for simulation
    root treePolicy(root state, int depth);
    
    // simulate the game
    double defaultPolicy(root state);

    // generate leagal move list
    // return list of 0~64
    std::vector<uint64_t > gen_movelist(uint64_t W, uint64_t B);
    
    std::tuple<uint64_t , uint64_t > pop_lsb(uint64_t bitmap);
    struct pos to_move(int bitmove){
        return pos(bitmove % 8, bitmove / 8);
    }
    double eval(uint64_t W, uint64_t B){
        int w0 = ((W & BIT[0]) != 0)? 1:0;
        int w1 = ((W & BIT[7]) != 0)? 1:0;
        int w2 = ((W & BIT[56]) != 0)? 1:0;
        int w3 = ((W & BIT[63]) != 0)? 1:0;
        int b0 = ((B & BIT[0]) != 0)? 1:0;
        int b1 = ((B & BIT[7]) != 0)? 1:0;
        int b2 = ((B & BIT[56]) != 0)? 1:0;
        int b3 = ((B & BIT[63]) != 0)? 1:0;
       
        int wunstable = 0;
	    int bunstable = 0;
        if ((w0 != 1) && (b0 != 1)){
            wunstable += ((W & BIT[1]) != 0) + ((W & BIT[8]) != 0) + ((W & BIT[9]) != 0);
            bunstable += ((B & BIT[1]) != 0) + ((B & BIT[8]) != 0) + ((B & BIT[9]) != 0);
        }
	    if ((w1 != 1) && (b1 != 1)){
            wunstable += ((W & BIT[6]) != 0) + ((W & BIT[14]) != 0) + ((W & BIT[15]) != 0);
            bunstable += ((B & BIT[6]) != 0) + ((B & BIT[14]) != 0) + ((B & BIT[15]) != 0);
        }
	    if ((w2 != 1) && (b2 != 1)) {
            wunstable += ((W & BIT[48]) != 0) + ((W & BIT[49]) != 0) + ((W & BIT[57]) != 0);
            bunstable += ((B & BIT[48]) != 0) + ((B & BIT[49]) != 0) + ((B & BIT[57]) != 0);
        }
	    if ((w3 != 1) && (b3 != 1)) {
            wunstable += ((W & BIT[62]) != 0) + ((W & BIT[54]) != 0) + ((W & BIT[55]) != 0);
            bunstable += ((B & BIT[62]) != 0) + ((B & BIT[54]) != 0) + ((B & BIT[55]) != 0);
	    }
        double scoreunstable = - 10.0 * (wunstable - bunstable);
        // piece difference
        double wpiece = (w0 + w1 + w2 + w3) * 100.0;
	    for (int i = 0; i < 7; i++){
	    int tmp = count_bit(W&P_RINGS[i]);
            wpiece += WEIGHTS[i] * tmp;
	    }
        double bpiece = (b0 + b1 + b2 + b3) * 100.0;
        for (int i = 0; i < 7; i++){
	    int tmp = count_bit(B&P_RINGS[i]);
            bpiece += WEIGHTS[i] * tmp;
	    }
        double scorepiece = wpiece - bpiece;
        //mobility
        double wmob = count_bit(move_gen(W, B));

        double scoremob = 5 * wmob;
        return scorepiece + scoreunstable + scoremob;
    } 
public:
    void init_map();
    
    // bit representation of each position
    std::vector<uint64_t> BIT = std::vector<uint64_t >();
    
    root selfRoot = nullptr;
    double C = 1.96;
    double time = 0;
    int depth = 0;
    std::vector<int> graph = {
        5,1,3,3,3,3,1,5,
        1,1,2,2,2,2,1,1,
        3,2,4,4,4,4,2,3,
        3,2,4,0,0,4,2,3,
        3,2,4,0,0,4,2,3,
        3,2,4,4,4,4,2,3,
        1,1,2,2,2,2,1,1,
        5,1,3,3,3,3,1,5,
        
                   };
    std::vector<int> SQ_DIR = {
        2, 2, 7, 7, 7, 7, 3, 3,
        2, 2, 7, 7, 7, 7, 3, 3,
        4, 4, 8, 8, 8, 8, 6, 6,
        4, 4, 8, 8, 8, 8, 6, 6,
        4, 4, 8, 8, 8, 8, 6, 6,
        4, 4, 8, 8, 8, 8, 6, 6,
        0, 0, 5, 5, 5, 5, 1, 1,
        0, 0, 5, 5, 5, 5, 1, 1
    };
    std::vector< std::vector<int> > DIR = {
        {1, -7, -8},
        {-1, -9, -8},
        {1,8,9},
        {7,8,-1},
        {8,9,1,-7,-8},
        {-1,1,-7,-8,-9},
        {7,8,-1,-9,-8},
        {7,8,9,-1,1},
        {-1, 1, -7,7,-8,8,-9,9}
    };
    std::map<int, std::vector<uint64_t >> RADIAL_MAP = std::map<int, std::vector<uint64_t >>();
    std::vector<uint64_t> LSB_TABLE = std::vector<uint64_t>(64);
    
    MCTS(){
        init_map();
        selfRoot = nullptr;
    }
    // return the position based on the current situation
    pos get_move(std::array<int, 64> myboard, int color, int depth = 10, int time_remaining = 0, int time_opponent = 0);
    
    
    
    ~MCTS(){
        
    }
};



