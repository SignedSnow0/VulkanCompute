#include "vec.hpp"

float randomNormalDistribution() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

   float theta = PI * 2 * distribution(generator);
   float rho = sqrt(-2.0f * log(distribution(generator)));
   return rho * cos(theta);
}