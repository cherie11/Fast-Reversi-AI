#include <iostream>
#include "MCTS.h"
int main() {
    std::cout << "hello world!" << std::endl;
    MCTS tree = MCTS();
    int cur_color = -1;
    std::array<int, 64> array = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1,-1, 0, 0, 0,
        0, 0, 0,-1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };
    pos postion = tree.get_move(array, cur_color,10);
    printf("****(%d,%d)\n",postion.x,postion.y);
    
    std::array<int, 64> array2= {
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,
         0, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    };
    postion = tree.get_move(array2, cur_color);
    printf("***(%d,%d)\n",postion.x,postion.y);
    std::array<int, 64> array3 = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,-1, 0, 0,
        0, 0, 0, 0,-1, 0, 0, 0,
        0, 0,-1,-1, 1, 0, 0, 0,
        0, 0, 1,-1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };
    postion = tree.get_move(array3, cur_color);
    printf("***(%d,%d)\n",postion.x,postion.y);
    return 0;
}
