///////////////////////////////////////////////////////////////////////////////
// maxtime.hh
//
// Compute the set of rides that maximizes the time spent at rides, within a given budget,
// with the greedy method or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once


#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


// One armor item available for purchase.
class ArmorItem
{
	//
	public:

		//
		ArmorItem
		(
			const std::string& description,
			double cost_gold,
			double defense_points
		)
			:
			_description(description),
			_cost_gold(cost_gold),
			_defense_points(defense_points)
		{
			assert(!description.empty());
			assert(cost_gold > 0);
		}

		//
		const std::string& description() const { return _description; }
		double cost() const { return _cost_gold; }
		double defense() const { return _defense_points; }

	//
	private:

		// Human-readable description of the armor, e.g. "new enchanted helmet". Must be non-empty.
		std::string _description;

		// Cost, in units of gold; Must be positive
		double _cost_gold;

		// Defense points; most be non-negative.
		double _defense_points;
};


// Alias for a vector of shared pointers to ArmorItem objects.
typedef std::vector<std::shared_ptr<ArmorItem>> ArmorVector;


// Load all the valid armor items from the CSV database
// Armor items that are missing fields, or have invalid values, are skipped.
// Returns nullptr on I/O error.
std::unique_ptr<ArmorVector> load_armor_database(const std::string& path)
{
	std::unique_ptr<ArmorVector> failure(nullptr);

	std::ifstream f(path);
	if (!f)
	{
		std::cout << "Failed to load armor database; Cannot open file: " << path << std::endl;
		return failure;
	}

	std::unique_ptr<ArmorVector> result(new ArmorVector);

	size_t line_number = 0;
	for (std::string line; std::getline(f, line); )
	{
		line_number++;

		// First line is a header row
		if ( line_number == 1 )
		{
			continue;
		}

		std::vector<std::string> fields;
		std::stringstream ss(line);

		for (std::string field; std::getline(ss, field, '^'); )
		{
			fields.push_back(field);
		}

		if (fields.size() != 3)
		{
			std::cout
				<< "Failed to load armor database: Invalid field count at line " << line_number << "; Want 3 but got " << fields.size() << std::endl
				<< "Line: " << line << std::endl
				;
			return failure;
		}

		std::string
			descr_field = fields[0],
			cost_gold_field = fields[1],
			defense_points_field = fields[2]
			;

		auto parse_dbl = [](const std::string& field, double& output)
		{
			std::stringstream ss(field);
			if ( ! ss )
			{
				return false;
			}

			ss >> output;

			return true;
		};

		std::string description(descr_field);
		double cost_gold, defense_points;
		if (
			parse_dbl(cost_gold_field, cost_gold)
			&& parse_dbl(defense_points_field, defense_points)
		)
		{
			result->push_back(
				std::shared_ptr<ArmorItem>(
					new ArmorItem(
						description,
						cost_gold,
						defense_points
					)
				)
			);
		}
	}

	f.close();

	return result;
}


// Convenience function to compute the total cost and defense in an ArmorVector.
// Provide the ArmorVector as the first argument
// The next two arguments will return the cost and defense back to the caller.
void sum_armor_vector
(
	const ArmorVector& armors,
	double& total_cost,
	double& total_defense
)
{
	total_cost = total_defense = 0;
	for (auto& armor : armors)
	{
		total_cost += armor->cost();
		total_defense += armor->defense();
	}
}


// Convenience function to print out each ArmorItem in an ArmorVector,
// followed by the total kilocalories and protein in it.
void print_armor_vector(const ArmorVector& armors)
{
	std::cout << "*** Armor Vector ***" << std::endl;

	if ( armors.size() == 0 )
	{
		std::cout << "[empty armor list]" << std::endl;
	}
	else
	{
		for (auto& armor : armors)
		{
			std::cout
				<< "Ye olde " << armor->description()
				<< " ==> "
				<< "Cost of " << armor->cost() << " gold"
				<< "; Defense points = " << armor->defense()
				<< std::endl
				;
		}

		double total_cost, total_defense;
		sum_armor_vector(armors, total_cost, total_defense);
		std::cout
			<< "> Grand total cost: " << total_cost << " gold" << std::endl
			<< "> Grand total defense: " << total_defense
			<< std::endl
			;
	}
}


// Filter the vector source, i.e. create and return a new ArmorVector
// containing the subset of the armor items in source that match given
// criteria.
// This is intended to:
//	1) filter out armor with zero or negative defense that are irrelevant to our optimization
//	2) limit the size of inputs to the exhaustive search algorithm since it will probably be slow.
//
// Each armor item that is included must have at minimum min_defense and at most max_defense.
//	(i.e., each included armor item's defense must be between min_defense and max_defense (inclusive).
//
// In addition, the the vector includes only the first total_size armor items that match these criteria.
std::unique_ptr<ArmorVector> filter_armor_vector
(
	const ArmorVector& source,
	double min_defense,
	double max_defense,
	int total_size
)
{
	// declaring a new ArmorVector to return
	ArmorVector output;

	// variable to limit the size of the output
    int current_size = 0;

	for (auto& armor : source)
    {
        double d = armor->defense();
        // condition check to belong in the output
        if (d > 0 && d >= min_defense && d <= max_defense && current_size < total_size)
        {
            current_size++;
            output.push_back(std::shared_ptr<ArmorItem>(new ArmorItem(armor->description(), armor->cost(), armor->defense())));
        }
    }

	return std::make_unique<ArmorVector>(output);
}

// return a binary of a number 'num' of a fixed length 'len'
std::string get_binary(int num, int len)
{
    // getting reverse binary
    std::string bin_str = "";
    while(num != 0)
    {
        int rem = num % 2;
        bin_str += '0' + rem;
        num /= 2;
    }
    // fixing length
    int sz = bin_str.size();
    for(int i = sz; i < len; i++) bin_str += '0';
    // fixing reverse
    for(int i = 0; i < len / 2; i++)
    {
        std::swap(bin_str[i], bin_str[len - 1 - i]);
    }
    return bin_str;
}


// Compute the optimal set of armor items with a greedy algorithm.
// Specifically, among the armor items that fit within a total_cost gold budget,
// choose the armors whose defense is greatest.
// Repeat until no more armor items can be chosen, either because we've run out of armor items,
// or run out of gold.
std::unique_ptr<ArmorVector> greedy_max_defense
(
	const ArmorVector& armors,
	double total_cost
)
{
	// declaring a new ArmorVector to return
	ArmorVector output;
	ArmorVector source = armors;
    double current_cost = 0.0;
    while(!source.empty())
    {
        // variable to set the minimum value
        int loop_pos = 0;
        double max_ratio = 0;
        int optimal_choice_pos = 0;
        //getting maximum value
        for (auto& armor : source)
        {
            if(loop_pos == 0)
            {
                max_ratio = armor->defense() / armor->cost();
                optimal_choice_pos = loop_pos;
                loop_pos++;
                continue;
            }
            double cur_ratio = armor->defense() / armor->cost();
            if(cur_ratio > max_ratio)
            {
                max_ratio = cur_ratio;
                optimal_choice_pos = loop_pos;
            }
            loop_pos++;
        }
        // check fitting condition
        if(current_cost + source[optimal_choice_pos]->cost() <= total_cost)
        {
            current_cost += source[optimal_choice_pos]->cost();
            output.push_back(source[optimal_choice_pos]);
        }
        source.erase(source.begin() + optimal_choice_pos);
    }

	return std::make_unique<ArmorVector>(output);
}


// Compute the optimal set of armor items with an exhaustive search algorithm.
// Specifically, among all subsets of armor items,
// return the subset whose gold cost fits within the total_cost budget,
// and whose total defense is greatest.
// To avoid overflow, the size of the armor items vector must be less than 64.
std::unique_ptr<ArmorVector> exhaustive_max_defense
(
	const ArmorVector& armors,
	double total_cost
)
{
	const int n = armors.size();
	assert(n < 64);

    // variables
    double best_defense = -1.0, current_cost = -1.0, current_defense = -1.0;
    std::string best_set = "";
	// size of power set
	int pn = 1;
	for(int i = 0; i < n; i++) pn *= 2;
	// calculating results for all the subsets
	for(int i = 0; i < pn; i++)
    {
        // calculating result for a subset
        std::string bitmask_string = get_binary(i, n);
        current_defense = 0.0;
        current_cost = 0.0;
        for(int j = 0; j < n; j++) if(bitmask_string[j] == '1')
        {
            current_cost += armors[j]->cost();
            current_defense += armors[j]->defense();
        }
        // filtering the optimal option
        if(current_cost <= total_cost && current_defense > best_defense)
        {
            best_defense = current_defense;
            best_set = bitmask_string;
        }
    }
    // creating output vector
    ArmorVector output;
    for(int i = 0; i < n; i++) if(best_set[i] == '1')
    {
        output.push_back(armors[i]);
    }
	return std::make_unique<ArmorVector>(output);
}









