#pragma once
#include "FunctionsAGE3.h"

namespace AGE3
{
	template<unsigned L>
	struct NDimensionPoint
	{
		static unsigned seed;

		bitset<L> value;

		void mutate(float prob)
		{
			for (unsigned i = 0; i < L; ++i) {
				if (rand() % 1000 < prob * 1000)
					value[i].flip();
			}
		}

		//Crossover
		void operator& (NDimensionPoint& other)
		{
			const unsigned ch_pos = rand() % (L);

			for (unsigned j = ch_pos % L; j < L; ++j)
			{
				bool b = value[j];
				value[j] = other.value[j];
				other.value[j] = b;
			}
		}
	};
	template<unsigned L>
	unsigned NDimensionPoint<L>::seed = []() {const auto seed = time(nullptr);  srand(seed); return seed; }();

	template<typename Class>
	float sat_solver() {

		//CONSTANTS
		constexpr unsigned pop_size = 100;
		constexpr unsigned gen_max = 1000;
		float p_m = 0.01f;
		constexpr float p_c = 0.3f;

		//bool C = Class::clauses[0];

		float best_fit = 1.5f;
		float average_fit = 1.1f;
		float worst_fit = 1.1f;

		auto pressure_function = [&](decltype(Class::bitstring)& bitset) -> float
		{
			Class::bitstring = bitset;
			int count = 0;
			for (auto& f : Class::clauses)
			{
				if (f())
					count++;
			}
			return 19084-count;
		};
		auto engine = std::mt19937{ std::random_device{}() };

		//VARS
		//float global_best = numeric_limits<float>::infinity();

		//RUN
		//for (unsigned run = 0; run < 15; run++) {
			array<NDimensionPoint<Class::variables>, pop_size> pop;
			unsigned current_generation = 0;

			float best = numeric_limits<float>::infinity();
			float last_generation_best = numeric_limits<float>::infinity();

			auto stop_condition = [&]() ->bool
			{
				if (current_generation % 30 == 0)
				{
					best_fit += 1;
					if (worst_fit > 1.01)
						worst_fit -= 0.01;
				}

				static bool put_back_p_m = false;
				if (current_generation % 50 == 0)
				{
					if (best < last_generation_best + 1 && best > last_generation_best - 1)
					{
						put_back_p_m = true;
						p_m = 0.2f;
					}
				}

				if (put_back_p_m)
				{
					p_m = 0.01f;
					put_back_p_m = false;
				}

				return current_generation++ > gen_max;
			};

			//Init phase
			for (auto& c : pop)
			{
				c.mutate(0.5f);
			}

			do
			{
				array<unsigned, pop_size / 2> other_c_index;
				iota(other_c_index.begin(), other_c_index.end(), pop_size / 2);
				shuffle(other_c_index.begin(), other_c_index.end(), engine);

				//Mutation phase
				for (auto& c : pop)
				{
					c.mutate(p_m);
				}

				//Crossing phase
				shuffle(pop.begin(), pop.end(), engine);
				for (unsigned c = 0; c < pop_size / 2; ++c)
				{
					if (double(engine() % 1000) / 1000.0f < p_c)
						pop[c] & pop[other_c_index[c]];
				}

				//Selection phase
				float values[pop_size];
				float min = numeric_limits<float>::infinity();
				float max = -numeric_limits<float>::infinity();
				for (unsigned i = 0; i < pop_size; i++)
				{
					//Computing the values is a bit different, we have to turn from binary to gray code

					/*array<float, N> arr;
					for (unsigned j = 0; j < L; ++j)
					{
						unsigned x = pop[i].coordinates[j].to_ulong();
						x = x ^ (x >> 1);
						arr[j] = interval.left + x * (interval.right - interval.left) / (pow_c(2, L) - 1);
					}*/

					//Deocamdata fara codul gray

					values[i] = pressure_function(pop[i].value);
					if (values[i] < min)
						min = values[i];
					else if (values[i] > max)
						max = values[i];
				}

				//WHEEL_OF_FORTUNE
				float fitness_values[pop_size];
				for (unsigned i = 0; i < pop_size; i++)
				{
					if (values[i] == max)
						fitness_values[i] = worst_fit * max - values[i];
					else if (values[i] == min)
						fitness_values[i] = best_fit * max - values[i];
					else
						fitness_values[i] = average_fit * max - values[i];
				}

				float wheel_values[pop_size];
				wheel_values[0] = fitness_values[0];
				for (unsigned i = 1; i < pop_size; i++)
				{
					wheel_values[i] = wheel_values[i - 1] + fitness_values[i];
				}

				decltype(pop) new_pop;
				for (unsigned i = 0; i < pop_size; ++i)
				{
					const auto stop_poz = float(engine() % 1000) / 1000.0f * wheel_values[pop_size - 1];

					unsigned idx = -1;
					for (unsigned j = 0; j < pop_size; j++)
					{
						if (stop_poz <= wheel_values[j])
						{
							idx = j;
							break;
						}
					}

					new_pop[i] = pop[idx];
				}
				pop = new_pop;
				best = min;
				printf("%f\n", best);
			} while (!stop_condition());

			//if (best < global_best)
			//	global_best = best;
		//}

		//return global_best;
			return best;
	}
}