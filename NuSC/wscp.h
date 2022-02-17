#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <limits.h>
#include <float.h>
#include <memory.h>
#include <sys/times.h>
#include <unistd.h>
#include <math.h>
#include <set>
#include <algorithm>
#include <utility>

using namespace std;

#define mypop(stack) stack[--stack##_fill_pointer]
#define mypush(item, stack) stack[stack##_fill_pointer++] = item

static struct tms start_time;
static double get_runtime()
{
    struct tms stop;
    times(&stop);
    return (double)(stop.tms_utime - start_time.tms_utime + stop.tms_stime - start_time.tms_stime) / sysconf(_SC_CLK_TCK);
}
static void start_timing()
{
    times(&start_time);
}

class WSCP
{
  public:
    //set(variable, soft clause) information
    long long *score;
    long long *pscore;
    long long *time_stamp;
    int *cost;
    int *org_cost;
    bool *cc;
    int *fix;
    set<int> tabu_list;
    int *zero_stack;
    int zero_stack_fill_pointer;
    int *index_in_zero_stack;

    //variable(hard clause)
    long long *weight;
    int *cover_count;
    int *cover_set;
    int *cover_set2;
    int *uncover_stack;
    int *index_in_uncover_stack;
    int uncover_stack_fill_pointer;

    //solution information
    int *cur_solu;
    long long cur_cost;
    int reduce_cost;
    int *best_solu;
    long long best_cost;
    double best_time;
    int *set_solu;
    int *index_in_set_solu;
    int set_count;

    //date structure
    int cutoff_time;
    int seed;
    int t;
    long long step;
    long long max_step;
    long long tries;
    long long max_tries;

    int var_num;
    int set_num;

    int *var_delete;
    int *set_delete;
    int **var_set;
    int *var_set_num;
    int *org_var_set_num;
    int **set_var;
    int *set_var_num;
    int *org_set_var_num;

    int *goodset_stack;
    int goodset_stack_fill_pointer;

    WSCP();
    WSCP(int time_limit);
    void all_memory();
    void free_memory();

    void build_instance(char *file_name);
    void reduce_instance();
    void init();
    void local_search();
    void add_set(int s);
    void remove_set(int s);
    void flip(int flip_set);
    void cover(int selected_var);
    void uncover(int selected_var);
    int compare(int s1, int c1, int s2, int c2);
    int compare(int s1, int c1, int s2, int c2, int p1, int p2);
    int compareV2(int s1, int c1, int s2, int c2, int p1, int p2, int weight1, int weight2);
    double weight1 = 4;
    double weight2 = 1;
    double novelty_p = 0.5;
    double explore_p = 0.5;
    int tabu_length = 5;
    int select_set(int is_tabu);
    int select_set_Novelty();
    int select_set_from_zero_stack();
    void set_param(int new_weight, int new_tabu_length, double new_novelty_p);
    void update_weight();
    void check_solu();
    void check_cur_solu();
};

WSCP::WSCP()
{
    step = 0;
    t = 170;
    cutoff_time = 1000;
    max_step = INT_MAX;
    max_tries = INT_MAX;
}

WSCP::WSCP(int time_limit)
{
    step = 0;
    t = 170;
    cutoff_time = time_limit;
    max_step = INT_MAX;
    max_tries = INT_MAX;
}

void WSCP::set_param(int new_weight, int new_tabu_length, double new_novelty_p)
{
    weight1 = new_weight;
    tabu_length = new_tabu_length;
    novelty_p = new_novelty_p;
}

int WSCP::compare(int s1, int c1, int s2, int c2)
{
    if (c1 == c2)
    {
        if (s1 > s2)
            return 1;
        else if (s1 == s2)
            return 0;
        else
            return -1;
    }
    long long t1 = s1, t2 = s2;
    t1 = t1 * c2;
    t2 = t2 * c1;
    if (t1 > t2)
        return 1;
    else if (t1 == t2)
        return 0;
    else
        return -1;
}

int WSCP::compare(int s1, int c1, int s2, int c2, int p1, int p2)
{
    if (c1 == c2)
    {
        if ((s1) > (s2) || (s1 == s2 && p1 > p2))
            return 1;
        else if ((s1 == s2) && (p1 == p2))
            return 0;
        else
            return -1;
    }
    long long t1 = s1, t2 = s2;
    t1 = t1 * c2;
    t2 = t2 * c1;
    if (t1 > t2)
        return 1;
    else if (t1 == t2)
        return 0;
    else
        return -1;
}

int WSCP::compareV2(int s1, int c1, int s2, int c2, int p1, int p2, int weight1, int weight2)
{
    int combined_s1 = weight1*s1 + weight2*p1;
    int combined_s2 = weight1*s2 + weight2*p2;

    // fix combined score
    if (c1 == c2)
    {
        if (combined_s1 > combined_s2)
            return 1;
        else if (combined_s1 == combined_s2)
            return 0;
        else
            return -1;
    }
    long long t1 = s1, t2 = s2;
    t1 = t1 * c2;
    t2 = t2 * c1;
    if (t1 > t2)
        return 1;
    else if (t1 == t2)
        return 0;
    else
        return -1;
}

void WSCP::all_memory()
{
    int max_set_num = set_num + 10;
    int max_var_num = var_num + 10;

    score = new long long[max_set_num];
    pscore = new long long[max_set_num];
    time_stamp = new long long[max_set_num];
    cost = new int[max_set_num];
    org_cost = new int[max_set_num];
    cc = new bool[max_set_num];
    fix = new int[max_set_num];
    zero_stack = new int[max_set_num];
    index_in_zero_stack = new int[max_set_num];

    //variable(hard clause)
    weight = new long long[max_var_num];
    cover_count = new int[max_var_num];
    cover_set = new int[max_var_num];
    cover_set2 = new int[max_var_num];
    uncover_stack = new int[max_var_num];
    index_in_uncover_stack = new int[max_var_num];

    //solution information
    cur_solu = new int[max_set_num];
    best_solu = new int[max_set_num];
    set_solu = new int[max_set_num];
    index_in_set_solu = new int[max_set_num];

    //date structure
    var_delete = new int[max_var_num];
    var_set = new int *[max_var_num];
    var_set_num = new int[max_var_num];
    org_var_set_num = new int[max_var_num];
    set_var = new int *[max_set_num];
    set_var_num = new int[max_set_num];
    org_set_var_num = new int[max_set_num];

    goodset_stack = new int[max_set_num];
}

void WSCP::free_memory()
{
    delete score;
    delete pscore;
    delete time_stamp;
    delete cost;
    delete org_cost;
    delete cc;
    delete fix;
    delete zero_stack;
    delete index_in_zero_stack;

    delete weight;
    delete cover_count;
    delete cover_set;
    delete cover_set2;
    delete uncover_stack;
    delete index_in_uncover_stack;

    delete cur_solu;
    delete best_solu;
    delete set_solu;
    delete index_in_set_solu;

    for (int i = 0; i < var_num; ++i)
        delete var_set[i];

    delete var_delete;
    delete var_set;
    delete var_set_num;
    delete org_var_set_num;
    delete org_set_var_num;

    for (int i = 0; i < set_num; ++i)
        delete set_var[i];
    delete set_var;
    delete set_var_num;

    delete goodset_stack;
}

void WSCP::build_instance(char *file_name)
{
    ifstream infile(file_name);

    infile >> var_num >> set_num;
    all_memory();

    for (int i = 0; i < set_num; ++i)
    {
        infile >> cost[i];
        org_cost[i] = cost[i];
    }
    memset(set_var_num, 0, set_num * sizeof(int));
    memset(fix, 0, set_num * sizeof(bool));
    for (int i = 0; i < var_num; ++i)
    {
        infile >> var_set_num[i];
        var_set[i] = new int[var_set_num[i] + 1];
        for (int j = 0; j < var_set_num[i]; ++j)
        {
            infile >> var_set[i][j];
            --var_set[i][j];
            ++set_var_num[var_set[i][j]];
        }
    }

    for (int i = 0; i < set_num; ++i)
    {
        set_var[i] = new int[set_var_num[i] + 1];
        set_var_num[i] = 0; //reset for buile set_var
    }
    //build set_var
    for (int i = 0; i < var_num; ++i)
    {
        for (int j = 0; j < var_set_num[i]; ++j)
        {
            int cur_set = var_set[i][j];
            set_var[cur_set][set_var_num[cur_set]++] = i;
        }
    }
}

void WSCP::reduce_instance()
{
    int s1, s2, s3;
    reduce_cost = 0;
    memset(fix, 0, set_num * sizeof(int));
    for (int i = 0; i < var_num; ++i)
    {
        if (var_set_num[i] == 1)
        {
            fix[var_set[i][0]] = 1;
        }
        else if (var_set_num[i] == 2)
        {
            s1 = var_set[i][0];
            s2 = var_set[i][1];
            if (fix[s1] != 0 || fix[s2] != 0)
                continue;
            if (set_var_num[s1] < set_var_num[s2])
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s1]; ++j)
                {
                    int v = set_var[s1][j];
                    int flag = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s2)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 0)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (cost[s1] >= cost[s2])
                    {
                        fix[s1] = -1;
                        fix[s2] = 1;
                    }
                    else if (cost[s1] < cost[s2])
                    {
                        fix[s1] = s2 + 10;
                        cost[s2] -= cost[s1];
                        reduce_cost += cost[s1];
                    }
                }
            }
            else
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s2]; ++j)
                {
                    int v = set_var[s2][j];
                    int flag = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s1)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 0)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (set_var_num[s1] == set_var_num[s2])
                    {
                        if (cost[s1] <= cost[s2])
                        {
                            fix[s1] = 1;
                            fix[s2] = -1;
                        }
                    }
                    else if (cost[s2] >= cost[s1])
                    {
                        fix[s2] = -1;
                        fix[s1] = 1;
                    }
                    else if (cost[s2] < cost[s1])
                    {
                        fix[s2] = s1 + 10;
                        cost[s1] -= cost[s2];
                        reduce_cost += cost[s2];
                    }
                }
            }
        }
        else if (var_set_num[i] == 3)
        {
            s1 = var_set[i][0];
            s2 = var_set[i][1];
            s3 = var_set[i][2];
            if (fix[s1] != 0 || fix[s2] != 0 || fix[s3] != 0)
                continue;
            if (set_var_num[s1] == set_var_num[s2] && set_var_num[s1] < set_var_num[s3])
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s1]; ++j)
                {
                    int v = set_var[s1][j];
                    int flag2 = 0;
                    int flag3 = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s2)
                        {
                            flag2 = 1;
                        }
                        else if (var_set[v][k] == s3)
                        {
                            flag3 = 1;
                        }
                        if (flag2 == 1 && flag3 == 1)
                            break;
                    }
                    if (flag2 != 1 || flag3 != 1)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (fix[s3] == 1)
                    {
                        fix[s1] = -1;
                        fix[s2] = -1;
                    }
                    else if (cost[s1] >= cost[s3] && cost[s2] >= cost[s3])
                    {
                        fix[s1] = -1;
                        fix[s2] = -1;
                        fix[s3] = 1;
                    }
                    else
                    {
                        if (cost[s1] > cost[s2])
                        {
                            fix[s1] = -1;
                            fix[s2] = 10 + s3;
                            cost[s3] -= cost[s2];
                            reduce_cost += cost[s2];
                        }
                        else
                        {
                            fix[s2] = -1;
                            fix[s1] = 10 + s3;
                            cost[s3] -= cost[s1];
                            reduce_cost += cost[s1];
                        }
                    }
                }
            }
            else if (set_var_num[s1] == set_var_num[s3] && set_var_num[s1] < set_var_num[s2])
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s1]; ++j)
                {
                    int v = set_var[s1][j];
                    int flag2 = 0;
                    int flag3 = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s2)
                        {
                            flag2 = 1;
                        }
                        else if (var_set[v][k] == s3)
                        {
                            flag3 = 1;
                        }
                        if (flag2 == 1 && flag3 == 1)
                            break;
                    }
                    if (flag2 != 1 || flag3 != 1)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (fix[s2] == 1)
                    {
                        fix[s1] = -1;
                        fix[s3] = -1;
                    }
                    else if (cost[s1] >= cost[s2] && cost[s3] >= cost[s2])
                    {
                        fix[s1] = -1;
                        fix[s3] = -1;
                        fix[s2] = 1;
                    }
                    else
                    {
                        if (cost[s1] > cost[s3])
                        {
                            fix[s1] = -1;
                            fix[s3] = 10 + s2;
                            cost[s2] -= cost[s3];
                            reduce_cost += cost[s3];
                        }
                        else
                        {
                            fix[s3] = -1;
                            fix[s1] = 10 + s2;
                            cost[s2] -= cost[s1];
                            reduce_cost += cost[s1];
                        }
                    }
                }
            }
            else if (set_var_num[s3] == set_var_num[s2] && set_var_num[s3] < set_var_num[s1])
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s3]; ++j)
                {
                    int v = set_var[s3][j];
                    int flag2 = 0;
                    int flag3 = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s2)
                        {
                            flag2 = 1;
                        }
                        else if (var_set[v][k] == s1)
                        {
                            flag3 = 1;
                        }
                        if (flag2 == 1 && flag3 == 1)
                            break;
                    }
                    if (flag2 != 1 || flag3 != 1)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (fix[s1] == 1)
                    {
                        fix[s2] = -1;
                        fix[s3] = -1;
                    }
                    if (cost[s3] >= cost[s1] && cost[s2] >= cost[s1])
                    {
                        fix[s3] = -1;
                        fix[s2] = -1;
                        fix[s1] = 1;
                    }
                    else
                    {
                        if (cost[s2] > cost[s3])
                        {
                            fix[s2] = -1;
                            fix[s3] = 10 + s1;
                            cost[s1] -= cost[s3];
                            reduce_cost += cost[s3];
                        }
                        else
                        {
                            fix[s3] = -1;
                            fix[s2] = 10 + s1;
                            cost[s1] -= cost[s2];
                            reduce_cost += cost[s2];
                        }
                    }
                }
            }
            else if (set_var_num[s3] == set_var_num[s2] && set_var_num[s3] == set_var_num[s1])
            {
                int flag1 = 0;
                for (int j = 0; j < set_var_num[s3]; ++j)
                {
                    int v = set_var[s3][j];
                    int flag2 = 0;
                    int flag3 = 0;
                    for (int k = 0; k < var_set_num[v]; ++k)
                    {
                        if (var_set[v][k] == s2)
                        {
                            flag2 = 1;
                        }
                        else if (var_set[v][k] == s1)
                        {
                            flag3 = 1;
                        }
                        if (flag2 == 1 && flag3 == 1)
                            break;
                    }
                    if (flag2 != 1 || flag3 != 1)
                    {
                        flag1 = 1;
                        break;
                    }
                }
                if (flag1 == 0)
                {
                    if (cost[s1] <= cost[s2] && cost[s1] <= cost[s3])
                    {
                        fix[s1] = 1;
                        fix[s2] = -1;
                        fix[s3] = -1;
                    }
                    else if (cost[s2] <= cost[s1] && cost[s2] <= cost[s3])
                    {
                        fix[s2] = 1;
                        fix[s1] = -1;
                        fix[s3] = -1;
                    }
                    else if (cost[s3] <= cost[s1] && cost[s3] <= cost[s2])
                    {
                        fix[s3] = 1;
                        fix[s2] = -1;
                        fix[s1] = -1;
                    }
                }
            }
        }
    }
    int v, tem_s;
    int top_s;
    memset(var_delete, 0, var_num * sizeof(int));
    for (int s = 0; s < set_num; ++s)
    {
        if (fix[s] == 1)
        {
            for (int i = 0; i < set_var_num[s]; ++i)
            {
                v = set_var[s][i];
                if (var_delete[v] == 1)
                    continue;
                for (int j = 0; j < var_set_num[v]; ++j)
                {
                    tem_s = var_set[v][j];
                    if (fix[tem_s] == 0)
                    {
                        for (int k = 0; k < set_var_num[tem_s]; ++k)
                        {
                            if (set_var[tem_s][k] == v)
                            {
                                top_s = set_var[tem_s][--set_var_num[tem_s]];
                                set_var[tem_s][set_var_num[tem_s]] = set_var[tem_s][k];
                                set_var[tem_s][k] = top_s;
                                if (set_var_num[tem_s] == 0)
                                {
                                    fix[tem_s] = -1;
                                }
                                break;
                            }
                        }
                    }
                }
                var_delete[v] = 1;
            }
        }
        else if (fix[s] > 9)
        {
            for (int i = 0; i < set_var_num[s]; ++i)
            {
                v = set_var[s][i];
                if (var_delete[v] == 1)
                    continue;
                for (int j = 0; j < var_set_num[v]; ++j)
                {
                    tem_s = var_set[v][j];
                    if (fix[tem_s] == 0)
                    {
                        for (int k = 0; k < set_var_num[tem_s]; ++k)
                        {
                            if (set_var[tem_s][k] == v)
                            {
                                top_s = set_var[tem_s][--set_var_num[tem_s]];
                                set_var[tem_s][set_var_num[tem_s]] = set_var[tem_s][k];
                                set_var[tem_s][k] = top_s;
                                if (set_var_num[tem_s] == 0)
                                {
                                    fix[tem_s] = -1;
                                }
                                break;
                            }
                        }
                    }
                }
                var_delete[v] = 1;
            }
        }
    }
}

void WSCP::cover(int selected_var)
{
    int index = index_in_uncover_stack[selected_var];
    int top_set = mypop(uncover_stack);
    index_in_uncover_stack[selected_var] = -1;
    uncover_stack[index] = top_set;
    index_in_uncover_stack[top_set] = index;
}

void WSCP::uncover(int selected_var)
{
    index_in_uncover_stack[selected_var] = uncover_stack_fill_pointer;
    mypush(selected_var, uncover_stack);
}

void WSCP::add_set(int s)
{
    index_in_set_solu[s] = set_count;
    set_solu[set_count++] = s;
}

void WSCP::remove_set(int s)
{
    int index = index_in_set_solu[s];
    int top_set = set_solu[--set_count];
    index_in_set_solu[top_set] = index;
    set_solu[index] = top_set;
    index_in_set_solu[s] = -1;
}

void WSCP::flip(int flip_set)
{
    int cur_set, cur_var;
    long long flip_set_score = score[flip_set];
    long long flip_set_pscore = pscore[flip_set];
    cur_solu[flip_set] = 1 - cur_solu[flip_set];

    if (index_in_zero_stack[flip_set] != -1 && cur_solu[flip_set] == 0)
    {
        int index = index_in_zero_stack[flip_set];
        int top_set = mypop(zero_stack);
        zero_stack[index] = top_set;
        index_in_zero_stack[top_set] = index;
        index_in_zero_stack[flip_set] = -1;
    }

    if (cur_solu[flip_set] == 1)
    {
        add_set(flip_set);
        cur_cost += cost[flip_set];
        for (int i = 0; i < set_var_num[flip_set]; ++i)
        {
            cur_var = set_var[flip_set][i];
            for (int j = 0; j < var_set_num[cur_var]; ++j)
                cc[var_set[cur_var][j]] = 1;
            ++cover_count[cur_var];
            if (cover_count[cur_var] == 1) //0->1
            {
                for (int j = 0; j < var_set_num[cur_var]; ++j)
                {
                    cur_set = var_set[cur_var][j];
                    score[cur_set] -= weight[cur_var];
                    pscore[cur_set] += weight[cur_var];
                }
                cover_set[cur_var] = flip_set;
                cover(cur_var);
            }
            else if (cover_count[cur_var] == 2) //1->2
            {
                cur_set = cover_set[cur_var];
                score[cur_set] += weight[cur_var];
                if (score[cur_set] == 0 && index_in_zero_stack[cur_set] == -1)
                {
                    index_in_zero_stack[cur_set] = zero_stack_fill_pointer;
                    mypush(cur_set, zero_stack);
                }
                cover_set2[cur_var] = flip_set;
                for (int j = 0; j < var_set_num[cur_var]; ++j)
                {
                    pscore[var_set[cur_var][j]] -= weight[cur_var];
                }
            }
            else if (cover_count[cur_var] == 3)
            {
                pscore[cover_set[cur_var]] += weight[cur_var];
                pscore[cover_set2[cur_var]] += weight[cur_var];
            }
        }
    }
    else
    {
        remove_set(flip_set);
        cur_cost -= cost[flip_set];
        for (int i = 0; i < set_var_num[flip_set]; ++i)
        {
            cur_var = set_var[flip_set][i];
            for (int j = 0; j < var_set_num[cur_var]; ++j)
                cc[var_set[cur_var][j]] = 1;
            --cover_count[cur_var];

            if (cover_count[cur_var] == 2)
            {
                int flag = 0;
                for (int j = 0; j < var_set_num[cur_var]; ++j)
                {
                    cur_set = var_set[cur_var][j];
                    if (cur_solu[cur_set] == 1)
                    {
                        pscore[cur_set] -= weight[cur_var];
                        if (flag == 0)
                        {
                            cover_set[cur_var] = cur_set;
                            flag = 1;
                        }
                        else
                        {
                            cover_set2[cur_var] = cur_set;
                            break;
                        }
                    }
                }
            }
            else if (cover_count[cur_var] == 1) //2->1
            {

                for (int j = 0; j < var_set_num[cur_var]; ++j)
                {
                    pscore[var_set[cur_var][j]] += weight[cur_var];
                }

                if (cover_set[cur_var] == flip_set)
                {
                    cover_set[cur_var] = cover_set2[cur_var];
                }
                score[cover_set[cur_var]] -= weight[cur_var];
                if (index_in_zero_stack[cover_set[cur_var]] != -1 && score[cover_set[cur_var]] != 0)
                {
                    int top_set = mypop(zero_stack);
                    int index = index_in_zero_stack[cover_set[cur_var]];
                    zero_stack[index] = top_set;
                    index_in_zero_stack[top_set] = index;
                    index_in_zero_stack[cover_set[cur_var]] = -1;
                }
            }
            else if (cover_count[cur_var] == 0) //1->0
            {
                for (int j = 0; j < var_set_num[cur_var]; ++j)
                {
                    cur_set = var_set[cur_var][j];
                    score[cur_set] += weight[cur_var];
                    pscore[cur_set] -= weight[cur_var];
                }
                uncover(cur_var);
            }
        }
        cc[flip_set] = 0;
    }
    pscore[flip_set] = -flip_set_pscore;
    score[flip_set] = -flip_set_score;
}

void WSCP::init()
{
    cur_cost = reduce_cost;
    best_cost = INT_MAX;
    for (int i = 0; i < set_num; ++i)
    {
        best_solu[i] = -1;
        if (fix[i] == 1)
        {
            cur_solu[i] = 1;
            cur_cost += cost[i];
            continue;
        }
        if (fix[i] != 0)
        {
            cur_solu[i] = 0;
            continue;
        }
        score[i] = set_var_num[i];
        pscore[i] = 0;
        cc[i] = 1;
        time_stamp[i] = 0;
        cur_solu[i] = 0;
        index_in_set_solu[i] = -1;
        index_in_zero_stack[i] = -1;
    }
    for (int i = 0; i < var_num; ++i)
    {
        if (var_delete[i] == 1)
            continue;
        weight[i] = 1;
        cover_count[i] = 0;
    }

    uncover_stack_fill_pointer = 0;
    set_count = 0;
    for (int i = 0; i < var_num; ++i)
    {
        if (var_delete[i] == 1)
            continue;
        index_in_uncover_stack[i] = uncover_stack_fill_pointer;
        mypush(i, uncover_stack);
    }
    check_cur_solu();

    //init feasible solution
    zero_stack_fill_pointer = 0;
    //cout << "Start initialize feasible solution! " << endl;
    while (uncover_stack_fill_pointer > 0)
    {
        //cout << "number of uncover var: " << uncover_stack_fill_pointer << endl;
        int selected_uncover_var = uncover_stack[random() % uncover_stack_fill_pointer];
        //cout << "selected uncover var: " << selected_uncover_var << endl;
        int sr = INT_MIN, ct = 1, best_set = 0;
        int ps = INT_MIN;
        //cout << "var set number: " << var_set_num[selected_uncover_var] << endl;
        for (int i = 0; i < var_set_num[selected_uncover_var]; ++i)
        {
            int cur_set = var_set[selected_uncover_var][i];

            if (sr == INT_MIN || compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) < 0)
            {
                sr = score[cur_set];
                ps = pscore[cur_set];
                ct = cost[cur_set];
                best_set = cur_set;
            }
            else if (compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) == 0)
            {
                if (pscore[best_set] < pscore[cur_set])
                    best_set = cur_set;
                else if (pscore[best_set] == pscore[cur_set] && time_stamp[best_set] > time_stamp[cur_set])
                    best_set = cur_set;
            }
        }
        flip(best_set);
        //cout << "filp set: " << best_set << endl;
    }
}

int WSCP::select_set(int is_tabu)
{
    int sr = INT_MIN, ct = 1, i;
    int best_pscore = INT_MIN;
    int best_set = -1;
    if (t < set_count)
    {
        for (int j = 0; j < t; ++j)
        {
            i = set_solu[rand() % set_count];
            if (!cur_solu[i])
                continue;
            if (fix[i])
                continue;

            if (sr == INT_MIN || compareV2(sr, ct, score[i], cost[i], best_pscore, pscore[i], weight1, weight2) < 0)
            {
                sr = score[i];
                best_pscore = pscore[i];
                ct = cost[i];
                best_set = i;
            }
            else if (compareV2(sr, ct, score[i], cost[i], best_pscore, pscore[i], weight1, weight2) == 0)
            {
                if (pscore[best_set] < pscore[i])
                    best_set = i;
                else if (pscore[best_set] == pscore[i] && time_stamp[best_set] > time_stamp[i])
                    best_set = i;
            }
        }
    }
    else
    {
        for (int j = 0; j < set_count; ++j)
        {
            i = set_solu[j];
            if (!cur_solu[i])
                continue;
            if (fix[i])
                continue;

            if (sr == INT_MIN || compareV2(sr, ct, score[i], cost[i], best_pscore, pscore[i], weight1, weight2) < 0)
            {
                sr = score[i];
                best_pscore = pscore[i];
                ct = cost[i];
                best_set = i;
            }
            else if (compareV2(sr, ct, score[i], cost[i], best_pscore, pscore[i], weight1, weight2) == 0)
            {
                if (pscore[best_set] < pscore[i])
                    best_set = i;
                else if (pscore[best_set] == pscore[i] && time_stamp[best_set] > time_stamp[i])
                    best_set = i;
            }
        }
    }

    return best_set;
}

int WSCP::select_set_Novelty()
{
    int selected_uncover_var = uncover_stack[random() % uncover_stack_fill_pointer];
    int sr = INT_MIN, ct = 1, best_set = 0;
    int ps = INT_MIN;
    for (int i = 0; i < var_set_num[selected_uncover_var]; ++i)
    {
        int cur_set = var_set[selected_uncover_var][i];
        if (sr == INT_MIN || compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) < 0)
        {
            sr = score[cur_set];
            ps = pscore[cur_set];
            ct = cost[cur_set];
            best_set = cur_set;
        }
        else if (compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) == 0)
        {
            if (pscore[best_set] < pscore[cur_set])
                best_set = cur_set;
            else if (pscore[best_set] == pscore[cur_set] && time_stamp[best_set] > time_stamp[cur_set])
                best_set = cur_set;
        }
    }

    int sec_best_set = 0;
    sr = INT_MIN;
    ct = 1;
    ps = INT_MIN;

    for (int i = 0; i < var_set_num[selected_uncover_var]; ++i)
    {
        int cur_set = var_set[selected_uncover_var][i];
        if (cur_set == best_set)
            continue;
        if (sr == INT_MIN || compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) < 0)
        {
            sr = score[cur_set];
            ps = pscore[cur_set];
            ct = cost[cur_set];
            sec_best_set = cur_set;
        }
        else if (compareV2(sr, ct, score[cur_set], cost[cur_set], ps, pscore[cur_set], weight1, weight2) == 0)
        {
            if (pscore[sec_best_set] < pscore[cur_set])
                sec_best_set = cur_set;
            else if (pscore[sec_best_set] == pscore[cur_set] && time_stamp[sec_best_set] > time_stamp[cur_set])
                sec_best_set = cur_set;
        }
    }
    
    if (step - time_stamp[best_set] < tabu_length)
    {
        if ((random() % 100)/(double)101 < novelty_p)
            best_set = sec_best_set;
    }

    return best_set;
}

int WSCP::select_set_from_zero_stack()
{
    int best_set = -1, best_cost = 0;
    int tem_set;
    if (t < zero_stack_fill_pointer)
    {
        for (int i = 0; i < t; ++i)
        {
            tem_set = zero_stack[rand() % zero_stack_fill_pointer];
            if (cost[tem_set] > best_cost)
            {
                best_cost = cost[tem_set];
                best_set = tem_set;
            }
        }
    }
    else
    {
        for (int i = 0; i < zero_stack_fill_pointer; ++i)
        {
            tem_set = zero_stack[i];
            if (cost[tem_set] > best_cost)
            {
                best_cost = cost[tem_set];
                best_set = tem_set;
            }
        }
    }
    return best_set;
}

void WSCP::update_weight()
{
    int cur_set, cur_var;
    for (int i = 0; i < uncover_stack_fill_pointer; ++i)
    {
        cur_var = uncover_stack[i];
        weight[cur_var]++;
        for (int j = 0; j < var_set_num[cur_var]; ++j)
        {
            cur_set = var_set[cur_var][j];
            score[cur_set]++;
        }
    }
}

void WSCP::local_search()
{
    for (tries = 0; tries < max_tries; ++tries)
    {
        for (step = 0; step < max_step; ++step)
        {
            if (uncover_stack_fill_pointer == 0)
            {
                if (zero_stack_fill_pointer > 0)
                {
                    int flip_set = select_set_from_zero_stack();
                    if (flip_set == -1)
                        return;
                    flip(flip_set);
                    time_stamp[flip_set] = step;
                    continue;
                }
                else if (cur_cost < best_cost)
                {
                    best_cost = cur_cost;
                    best_time = get_runtime();
                    cout << "o " << best_cost << " " << best_time << endl;
                    for (int i = 0; i < set_num; ++i)
                        best_solu[i] = cur_solu[i];
                }
            }
            if (uncover_stack_fill_pointer > 0)
            {
                int best_set = select_set_Novelty();

                if (cur_cost + cost[best_set] >= best_cost)
                {
                    int flip_set;
                    flip_set = select_set(0);
                    if (flip_set == -1)
                        return;
                    if (flip_set < 0)
                    {
                        flip(best_set);
                        time_stamp[best_set] = step;
                        continue;
                    }
                    else if (compare(-score[flip_set], cost[flip_set], score[best_set], cost[best_set]) < 0)
                    {
                        if (flip_set == -1 || best_set == -1)
                            return;

                        flip(flip_set);
                        flip(best_set);
                        time_stamp[flip_set] = step;
                        time_stamp[best_set] = step;
                        if (step % 1000 == 0)
                        {
                            if (get_runtime() > cutoff_time)
                                return;
                        }
                        continue;
                    }
                    else
                    {
                        flip_set = select_set(0);

                        if (flip_set == -1)
                            return;
                        flip(flip_set);
                        time_stamp[flip_set] = step;

                        flip_set = select_set(0);

                        if (flip_set == -1)
                            return;
                        if (flip_set >= 0)
                        {
                            flip(flip_set);
                            time_stamp[flip_set] = step;
                        }

                        if (step % 1000 == 0)
                        {
                            if (get_runtime() > cutoff_time)
                                return;
                        }
                        continue;
                    }
                }
                else
                {
                    if (best_set == -1)
                        return;
                    flip(best_set);
                    update_weight();
                    time_stamp[best_set] = step;
                    if (step % 1000 == 0)
                    {
                        if (get_runtime() > cutoff_time)
                            return;
                    }
                    continue;
                }
            }
            int flip_set = select_set(0);
            
            if (flip_set == -1)
                return;
            flip(flip_set);

            time_stamp[flip_set] = step;
            if (step % 1000 == 0)
            {
                if (get_runtime() > cutoff_time)
                    return;
            }
        }
    }
}

void WSCP::check_solu()
{
    long long tem_cost = 0;
    for (int i = 0; i < set_num; ++i)
    {
        if (fix[i] == 1 && best_solu[i] != 1)
            cout << "wrong 1" << endl;
        if (fix[i] < 0 && best_solu[i] != 0)
            cout << "wrong 2" << endl;
        if (fix[i] > 9)
        {
            int s = fix[i] - 10;
            if (best_solu[i] != 0)
                cout << "wrong 3" << endl;
            if (best_solu[s] == 0)
                best_solu[i] = 1;
        }
    }
    for (int i = 0; i < var_num; ++i)
    {
        int flag = 0;
        for (int j = 0; j < var_set_num[i]; ++j)
        {
            if (best_solu[var_set[i][j]] == 1)
            {
                flag = 1;
                break;
            }
        }
        if (flag == 0)
        {
            cout << "best solu is wrong in 0 " << endl;
            return;
        }
    }
    for (int i = 0; i < set_num; ++i)
    {
        if (best_solu[i] == 1)
            tem_cost += org_cost[i];
    }

    tem_cost = 0;
    for (int i = 0; i < var_num; ++i)
    {
        int flag = 0;
        for (int j = 0; j < var_set_num[i]; ++j)
        {
            if (best_solu[var_set[i][j]] == 1 || fix[var_set[i][j]] == 1 || fix[var_set[i][j]] == -2)
            {
                flag = 1;
                break;
            }
        }
        if (flag == 0)
        {
            cout << "best solu is wrong in 1" << endl;
            return;
        }
    }
    for (int i = 0; i < set_num; ++i)
    {
        if (fix[i] == 1 || best_solu[i] == 1)
            tem_cost += org_cost[i];

        if (fix[i] == -2)
        {

            for (int j = 0; j < org_set_var_num[i]; ++j)
            {
                int flag = 0;
                int v = set_var[i][j];
                for (int k = 0; k < var_set_num[v]; ++k)
                {
                    if (fix[var_set[v][k]] == 1 || best_solu[var_set[v][k]] == 1)
                    {
                        flag = 1;
                        break;
                    }
                }
                if (flag == 0)
                {
                    tem_cost += org_cost[i];
                    break;
                }
            }
        }
    }

    if (tem_cost != best_cost)
        cout << "best solu is wrong in 5 " << endl;

    tem_cost = 0;
    for (int i = 0; i < set_num; ++i)
    {
        if (fix[i] == 1 || best_solu[i] == 1)
            tem_cost += org_cost[i];
    }
    for (int i = 0; i < var_num; ++i)
    {
        int flag = 0;
        for (int j = 0; j < var_set_num[i]; ++j)
        {
            if (fix[var_set[i][j]] == 1 || best_solu[var_set[i][j]] == 1)
            {
                flag = 1;
                break;
            }
        }
        if (flag == 0)
        {
            if (var_set_num[i] == 2)
            {
                tem_cost += org_cost[i];
            }
        }
    }
}

void WSCP::check_cur_solu()
{
    long long tem_cost = 0;
    for (int i = 0; i < set_num; ++i)
    {
        if (fix[i] == 1 && cur_solu[i] != 1)
            cout << "wrong 11" << endl;
        if (fix[i] < 0 && cur_solu[i] != 0)
            cout << "wrong 22" << endl;
        if (fix[i] > 9)
        {
            if (cur_solu[i] != 0)
                cout << "wrong 33" << endl;
        }
    }

    for (int i = 0; i < set_num; ++i)
    {
        if (cur_solu[i] == 1)
            tem_cost += org_cost[i];
        else if (fix[i] > 9 && cur_solu[fix[i] - 10] == 0)
            tem_cost += org_cost[i];
    }
    if (tem_cost != cur_cost)
        cout << "some thing wrong in cur cost and tem_cost " << endl;
}
