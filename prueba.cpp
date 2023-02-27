#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
 
int main()
{
    std::random_device rd{};
    std::mt19937 gen{rd()};
 
    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> d{5, 2};
 
    std::map<int, int> hist{};
    for (int n = 0; n != 10000; ++n)
        ++hist[std::round(d(gen))];
 
    for (auto [x, y] : hist)
        std::cout << std::setw(2) << x << ' ' << std::string(y / 200, '*') << '\n';
}