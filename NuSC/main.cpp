#include <fstream>
#include "wscp.h"

int main(int argc, char *argv[])
{
    ifstream infile(argv[1]);
    double var_num, set_num;
    infile >> var_num >> set_num;

    int time_limit = atoi(argv[2]);
    WSCP wscp_solver(time_limit);
    int seed = atoi(argv[3]);
    srand(seed);

    int new_weight, tabu_len;
    double novelty_p;

    if (argc == 6){
        new_weight = atoi(argv[4]);
        tabu_len = atoi(argv[5]);
        novelty_p = atof(argv[6]);
    }
    else{
        if (set_num > var_num){
            new_weight = 5;
            tabu_len = 4;
            novelty_p = 0.1;
        }else{
            new_weight = 80;
            tabu_len = 5;
            novelty_p = 0.5;
        }
    }

    wscp_solver.build_instance(argv[1]);
    wscp_solver.reduce_instance();
    wscp_solver.set_param(new_weight, tabu_len, novelty_p);
    
    start_timing();
    wscp_solver.init();
    wscp_solver.local_search();
    wscp_solver.check_solu();

    cout << "v ";
    for (int i = 0; i < wscp_solver.set_num; ++i)
        cout << wscp_solver.best_solu[i] << " ";
    cout << endl;
    wscp_solver.free_memory();
}