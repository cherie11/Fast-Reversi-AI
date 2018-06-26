#include "MCTS.h"
pos MCTS::MCTSearch(root state_node){

    time_t start = std::time(NULL);
    int cnt=0;
    std::cout<<"#loading..."<<std::endl;

    while(1){
        cnt++;
        root leafState = treePolicy(state_node, this->depth);
        double reward = defaultPolicy(leafState);
        backup(leafState, reward);
        time_t now = std::time(NULL);
    /*Computation Budget Constraint*/
	if(now - start >= 10){
            break;
        }
    }
    
    std::cout << "$" << cnt << std::endl;
    return to_move(std::get<0>(bestChild(state_node, 0)));
}

root MCTS::treePolicy(root state, int depth){
    
    while(1){
        uint64_t W = state->curBoard.W;
        uint64_t B = state->curBoard.B;
        std::vector<uint64_t> legal_moves;
        //generate leagl move
        if(state->color == 1){
            legal_moves = gen_movelist(W,B);
        }else{
            legal_moves = gen_movelist(B,W);
        }
        if(legal_moves.empty()){
            return state;
        }
        
        auto childKeys = state->childState;
        //expand new node
        for(auto mv : legal_moves){
            if (childKeys.find(mv) == childKeys.end()){
                return expand(mv, state);
            }
        }
        auto res = bestChild(state, this->C);
       
        state = std::get<1>(res);
    }
}
double MCTS::defaultPolicy( root state){
    /*Simulation the game*/
    int cur_color = state->color;
    double v = 0.5;
    uint64_t W = state->curBoard.W;
    uint64_t B = state->curBoard.B;
    uint64_t player = B;
    uint64_t opponent = W;
    if(cur_color == 1){
        player = W;
        opponent = B;
        
    }
    while(1){
        std::vector<uint64_t > legal_moves = gen_movelist(player, opponent);
        if(legal_moves.empty() && (gen_movelist(opponent, player)).size() == 0){
            v = getFinalVal(player, opponent, cur_color);
	       break;
        }
        else{
            if(!legal_moves.empty()){
                double posVal = -999;
                uint64_t bestMv = legal_moves[0];
                /*Simulation with random and predefined rule*/
                if(rand() % 5 == 1){
                    bestMv = legal_moves[rand()%(legal_moves.size())];
                }
                else{
                    /*Choose move by preweight*/
                    for(auto mv: legal_moves){
                    	if(graph[mv]> posVal){
                            posVal = graph[mv];
                            bestMv = mv;
                    	}

                        else if(graph[mv]==posVal){
                            /*Choose move by mobility and potential mobility*/
                            if(calvalue(player,opponent, mv)>calvalue(player,opponent, bestMv)) 
                            bestMv = mv;
                        }
                     }
                }
                uint64_t flipmask = flip(player, opponent, bestMv);
                player ^= flipmask | BIT[bestMv];
                opponent ^= flipmask;
            }
            uint64_t t = player;
            player = opponent;
            opponent = t;
            cur_color = -cur_color;
        }
    }
    return v;
}
double MCTS::calvalue(uint64_t player,uint64_t opponent,uint64_t mv){
    /*Calculate by heriustic value*/
    uint64_t flipmask2 = flip(player,opponent, mv);
    uint64_t opp1 = player ^ (flipmask2 | BIT[mv]);
    uint64_t player1 = opponent ^ flipmask2;

    int posmob=count_bit(move_gen(player1,opp1))-count_bit(move_gen(opp1,player1));
    int mobscr=count_bit(player1)-count_bit(opp1);
	double value =  0.5*posmob+0.5*mobscr ;
    return value;

}
bool MCTS::checkEnd(board myboard){
    /*Check end state of game*/
    uint64_t W = myboard.W;
    uint64_t B = myboard.B;
    uint64_t selfTurn = W;
    uint64_t opponent = B;
    if(selfRoot->color == -1){
        selfTurn = B;
        opponent = W;
    }
    uint64_t selfMoves = move_gen(selfTurn, opponent);
    uint64_t opponentMoves = move_gen(opponent, selfTurn);
    return (selfMoves == 0 && opponentMoves == 0);
}
double MCTS::getFinalVal(uint64_t P, uint64_t O, int color){
    /*Judge the result of game*/
    double v=0.5;
    if (count_bit(P)>count_bit(O)){
        if(color == selfRoot->color){ // player color is same as the state color
            v = 1.0;
        }else{
            v = 0;
        }

    }else if(count_bit(P)<count_bit(O)){
        if(color == selfRoot->color){
            v = 0;
        }else{
            v = 1.0;
        }
    }
    
    return v;
}
void MCTS::backup(root state, double reward){
    /*Backup the reward*/
    while(state != nullptr){
        state->eval = (state->eval * state->count + reward)/double(state->count + 1);
        state->count += 1;
        state = state->parentState;
    }
}
root MCTS::expand(uint64_t mv, root state_node){
    /*Expand and Initialize new node with new move*/
    root state = new rootstruct();
    state->parentState = state_node;
    state->count = 0;
    state->eval = 0;
    uint64_t W = state_node->curBoard.W;
    uint64_t B = state_node->curBoard.B;
    uint64_t P = B;
    uint64_t O = W;
    //state_node->color = 1;
    if(state_node->color == 1){
        P = W;
        O = B;
    }
    //flip the disc
    uint64_t f_mask = flip(P, O, mv);
    P ^= (f_mask | BIT[mv]);
    O ^= f_mask;
    //create new node for board state
    if(state_node->color == 1){
        W = P;
        B = O;
    }else{
        W = O;
        B = P;
    }
    state->curBoard.B = B;
    state->curBoard.W = W;
    if(move_gen(O,P) == 0){
        state->color = state_node->color;
    }else{
        state->color = -state_node->color;
    }
    
    state_node->childState[mv] = state;
    return state;
}
std::tuple<uint64_t , root> MCTS::bestChild(root state, double C){
    /*Choose best child by score*/
    uint64_t targetMv = 0;
    root target_state = nullptr;
    int color = state->color;
    double targetEval = 0;
    targetEval = -999999.0;
    auto childList = state->childState;
    for(auto &it: childList){
        root child = it.second;
        double eval = 1 - child->eval + sqrt(C * log(state->count)/float(child->count));
        if(color == selfRoot->color){
            eval = child->eval + sqrt(C * log(state->count)/float(child->count));
        }
        if(eval > targetEval){
                targetEval = eval;
                targetMv = it.first;
                target_state = child;
            }
    }
    return std::tuple<uint64_t , root>(targetMv, target_state);
};
uint64_t MCTS::flip(uint64_t W, uint64_t B, uint64_t mv){
    uint64_t mask = 0;
    std::vector<int> dirList= DIR[SQ_DIR[mv]];
    for (auto dir : dirList){
        int64_t mvtmp = mv;
        mvtmp += dir;
        /*
        mvtmp : going on current direction
        condition 1 : Opponent has disc mvtmp
        condition 2 : mvtmp is accessiable
        */
        while(mvtmp >= 0 && mvtmp < 64 && ((BIT[mvtmp] & B) != 0) && ((BIT[mvtmp] & RADIAL_MAP[dir][mv]) != 0)){
            mvtmp += dir;
        }
        //Stop by the player disc
        if (mvtmp >= 0 && mvtmp < 64 && ((BIT[mvtmp] & W) != 0) && ((BIT[mvtmp] & RADIAL_MAP[dir][mv]) != 0)){
            mvtmp -= dir;//back to init position
            while(mvtmp != mv) {
                mask |= BIT[mvtmp];
                mvtmp -= dir;
            }
        }
        
    }
    return mask;
}
void MCTS::init_map(){
    // *********************
    // fill_bit_table
    for(int i = 0; i < 64; i ++){
        BIT.push_back((uint64_t)0x1 << i);
    }
    // *********************
    // fill_radial_map
    std::map<int, std::vector<int>> rad_map = {{-1,{-1,0} },{1,{1,0} },{-8,{0,-1} },{8,{0,1} },{-7,{1,-1} },{7,{-1,1} },{-9,{-1,-1} },{9,{1,1} } };
    for (auto &it : rad_map) {
        std::vector<uint64_t > lis(64);
        for(int sqr = 0; sqr < 64; sqr ++){
            uint64_t mask = 0;
            int sq = sqr;
            pos p = to_move(sq);  // 0~64 --> x,y
            sq += it.first;  // check this direction from current point
            int x = p.x + it.second[0];
            int y = p.y + it.second[1];
            while(x >= 0 && x < 8 && y >= 0 && y < 8 && sq >= 0 && sq <= 64){
                mask |= BIT[sq];   //able to move to such position
                sq += it.first;  // keep going on such direction(0~64)
                x += it.second[0];
                y += it.second[1];
            }
            lis[sqr] = mask;
        }
        RADIAL_MAP[it.first] = lis;
    }
    // *********************
    // fill_lsh_table
    uint64_t bitmap = 1;
    for(uint64_t i = 0; i < 64; i++){
        LSB_TABLE[(((bitmap & (~bitmap + 1)) * LSB_HASH) & FULL_MASK) >> 58] = i;
        bitmap <<=1;
    }
}
uint64_t MCTS::move_gen_sub(uint64_t P, uint64_t mask, int dir){
    //Search for 1 direction
    int dir2 = dir * 2;
    uint64_t flip1  = mask & (P << dir);
    uint64_t flip2  = mask & (P >> dir);
    flip1 |= mask & (flip1 << dir);
    flip2 |= mask & (flip2 >> dir);
    uint64_t mask1  = mask & (mask << dir);
    uint64_t mask2  = mask & (mask >> dir);
    flip1 |= mask1 & (flip1 << dir2);
    flip2 |= mask2 & (flip2 >> dir2);
    flip1 |= mask1 & (flip1 << dir2);
    flip2 |= mask2 & (flip2 >> dir2);
    return (flip1 << dir) | (flip2 >> dir);
}
uint64_t MCTS::move_gen(uint64_t P, uint64_t O){
    /*Check possible move by 8 direction*/
    uint64_t mask = O & 0x7E7E7E7E7E7E7E7E;
    return ((move_gen_sub(P, mask, 1) \
             | move_gen_sub(P, O, 8)  \
             | move_gen_sub(P, mask, 7) \
             | move_gen_sub(P, mask, 9)) & ~(P|O)) & FULL_MASK;
}
uint64_t MCTS::count_bit(uint64_t b){
    /*Calculate the disc amount*/
    b -=  (b >> 1) & 0x5555555555555555;
    b  = (((b >> 2) & 0x3333333333333333) + (b & 0x3333333333333333));
    b  = ((b >> 4) + b)  & 0x0F0F0F0F0F0F0F0F;
    return ((b * 0x0101010101010101) & FULL_MASK) >> 56;
}
pos MCTS::get_move(std::array<int, 64> myboard, int color, int depth , int time_remaining, int time_opponent){
    srand(std::time(NULL));
    this->selfRoot = new rootstruct();
    uint64_t W = 0;
    uint64_t B = 0;
    /*Convert board array to bitboard*/
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            if(myboard[c*8+r] == -1){
                B |= BIT[8 * r + c];
            }else if(myboard[c*8+r] == 1){
                W |= BIT[8*r + c];
            }
        }
    }
    /*Initialize root node*/
    selfRoot->parentState = nullptr;
    selfRoot->color = color;
    selfRoot->count = 0;
    selfRoot->eval = 0.0;
    selfRoot->curBoard = board(B, W);
    this->depth = depth;
    
    return this->MCTSearch(selfRoot);
}
std::vector<uint64_t > MCTS::gen_movelist(uint64_t W, uint64_t B){
    /*Concert the possible move(bit board) to move list*/
    std::vector<uint64_t> mlist = std::vector<uint64_t>();
    uint64_t leagal_moves = move_gen(W, B);
    uint64_t leagal_binary = leagal_moves;
    while(leagal_binary != 0){
        std::tuple<uint64_t , uint64_t > res = pop_lsb(leagal_binary);
        leagal_binary = std::get<1>(res);
        mlist.push_back(std::get<0>(res));
    }
    return mlist;
}
std::tuple<uint64_t , uint64_t > MCTS::pop_lsb(uint64_t bitmap){
    /*Calculate the least bit for 1*/
    uint64_t l = LSB_TABLE[(((bitmap & (~bitmap + 1)) * LSB_HASH) & FULL_MASK) >> 58];
    bitmap &= bitmap - 1;
    return std::tuple<uint64_t, uint64_t >(l, bitmap & FULL_MASK);
};
