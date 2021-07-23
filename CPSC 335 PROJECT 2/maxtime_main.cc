#include "maxtime.hh"
#include "timer.hh"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

int main()
{
    auto all_armors = load_armor_database("ride.csv");
	assert( all_armors );

    int MAX = 20;
    double time_greedy[MAX + 1];
    double time_exhaustive[MAX + 1];
    time_exhaustive[0] = 0.0;
    time_greedy[0] = 0.0;


    for(int i = 1; i <= MAX; i++)
    {
        std::unique_ptr<ArmorVector> soln_greedy, soln_exhaustive;

        double average_time_taken_exhaustive = 0.0;
        for(int j = 0; j < 10; j++)
        {
            Timer* t = new Timer();
            auto filtered_armors = filter_armor_vector(*all_armors, 1.0, 2500.0, i);
            soln_exhaustive = exhaustive_max_defense(*filtered_armors, 2500.0);
            double time_taken_exhaustive = t->elapsed() * 1000;
            average_time_taken_exhaustive += time_taken_exhaustive;
        }
        time_exhaustive[i] = average_time_taken_exhaustive / 10;

        double average_time_taken_greedy = 0.0;
        for(int j = 0; j < 10; j++)
        {
            Timer* t = new Timer();
            auto filtered_armors = filter_armor_vector(*all_armors, 1.0, 2500.0, 200 * i);
            soln_greedy = greedy_max_defense(*filtered_armors, 2500.0);
            double time_taken_greedy = t->elapsed() * 1000;
            average_time_taken_greedy += time_taken_greedy;
        }
        time_greedy[i] = average_time_taken_greedy / 10;
    }
    std::cout<<"\nAverage time taken for exhaustive in milliseconds\n"<<std::endl;
    for(int i = 0; i < MAX + 1; i++) std::cout<<" " <<time_exhaustive[i]<<std::endl;

    std::cout<<"\nAverage time taken for greedy in milliseconds\n"<<std::endl;
    for(int i = 0; i < MAX + 1; i++) std::cout<<" " <<time_greedy[i]<<std::endl;
	return 0;
}


