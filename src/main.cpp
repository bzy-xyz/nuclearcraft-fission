#include "main.h"

#include <string>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <set>
#include <deque>
#include <memory>
#include <random>
#include <functional>

#include <signal.h>

#include "Reactor.h"

#define DIM_X 5
#define DIM_Y 5
#define DIM_Z 5
#define OPTIMIZE_FUEL FuelType::LEU235O

std::default_random_engine generator;

const std::vector<BlockType> shortBlockTypes = {
  BlockType::air, //0
  BlockType::reactorCell, //1
  BlockType::moderator, //2
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,
  BlockType::cooler,

};

const std::vector<CoolerType> shortCoolerTypes_all = {
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::water,
  CoolerType::redstone,
  CoolerType::quartz,
  CoolerType::gold,
  CoolerType::glowstone,
  CoolerType::lapis,
  CoolerType::diamond,
  CoolerType::liquidHelium,
  CoolerType::enderium,
  CoolerType::cryotheum,
  CoolerType::iron,
  CoolerType::emerald,
  CoolerType::copper,
  CoolerType::tin,
  CoolerType::magnesium,
  CoolerType::activeCryotheum,
};

const std::vector<CoolerType> shortCoolerTypes_active = {
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  // CoolerType::water,
  // CoolerType::redstone,
  // CoolerType::quartz,
  // CoolerType::gold,
  // CoolerType::glowstone,
  // CoolerType::lapis,
  // CoolerType::diamond,
  // CoolerType::liquidHelium,
  CoolerType::enderium,
  // CoolerType::cryotheum,
  // CoolerType::iron,
  // CoolerType::emerald,
  // CoolerType::copper,
  // CoolerType::tin,
  // CoolerType::magnesium,
  CoolerType::activeCryotheum,
};

const std::vector<CoolerType> shortCoolerTypes_passive = {
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::water,
  CoolerType::redstone,
  CoolerType::quartz,
  CoolerType::gold,
  CoolerType::glowstone,
  CoolerType::lapis,
  CoolerType::diamond,
  CoolerType::liquidHelium,
  CoolerType::enderium,
  CoolerType::cryotheum,
  CoolerType::iron,
  CoolerType::emerald,
  CoolerType::copper,
  CoolerType::tin,
  CoolerType::magnesium,
  // CoolerType::activeCryotheum,
};

const std::vector<CoolerType> * shortCoolerTypes = &shortCoolerTypes_passive;

float objective_fn_efficiency(Reactor & r, FuelType optimizeFuel)
{
  return (1 + r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100000.)
          //- (r.heatGenerated(OPTIMIZE_FUEL) > 0 ? r.effectivePowerGenerated(OPTIMIZE_FUEL) : 0))
          / (0.1 + r.inactiveBlocks() * r.inactiveBlocks());
}

float objective_fn_output(Reactor & r, FuelType optimizeFuel)
{
  return (1 + r.effectivePowerGenerated(optimizeFuel))
          / (0.1 + r.inactiveBlocks() * r.inactiveBlocks());
}

float objective_fn_cells(Reactor & r, FuelType optimizeFuel)
{
  float mult = r.heatGenerated(optimizeFuel) <= 0 ? 1 : r.heatGenerated(FuelType::air) / (r.heatGenerated(FuelType::air) - r.heatGenerated(optimizeFuel));
  return (1 + r.totalCells() * mult)
          / (0.1 + r.inactiveBlocks() * r.inactiveBlocks());
}

#define OBJECTIVE_FN objective_fn_efficiency

std::set<Reactor> tabuSet;
std::deque<Reactor> tabuList;

void step_rnd(Reactor & r, int idx, FuelType f, decltype(OBJECTIVE_FN) objective_fn)
{
  std::vector<Reactor> steps;
  std::vector<double> step_weights;

  #pragma omp parallel for
  for(int m = 0; m < 400; m++)
  {
    int x, y, z, i;

    Reactor r1 = r;

    int nn = std::uniform_int_distribution<int>(1, 5)(generator);;
    for(int n = 0; n < nn; n++) {
      x = std::uniform_int_distribution<int>(0, r.x() - 1)(generator);
      y = std::uniform_int_distribution<int>(0, r.y() - 1)(generator);
      z = std::uniform_int_distribution<int>(0, r.z() - 1)(generator);
      i = std::uniform_int_distribution<int>(0, shortCoolerTypes->size() - 1)(generator);
      r1.setCell(x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
      if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 500) {
        r1.setCell(r.x() - 1 - x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(r.x() - 1 - x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(r.x() - 1 - x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        r1.setCell(r.x() - 1 - x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
      }
    }

    double score = pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000);
    #pragma omp critical
    {
      if(!tabuSet.count(r1) || m == 0) {
        steps.push_back(r1);
        step_weights.push_back(score);
      }
    }
  }

  int ret_idx = std::discrete_distribution<int>(step_weights.begin(), step_weights.end())(generator);
  r = steps[ret_idx];

  tabuSet.insert(r);
  tabuList.push_back(r);

  if(tabuList.size() > 10000)
  {
    Reactor z = tabuList.front();
    tabuList.pop_front();
    tabuSet.erase(z);
  }

}
//
// void step_grd(Reactor & r, int idx, FuelType f, decltype(OBJECTIVE_FN) objective_fn)
// {
//   std::vector<Reactor> steps;
//   std::vector<double> step_weights;
//
//   #pragma omp parallel for
//   for(int x = 0; x < DIM_X; x++){
//     for(int y = 0; y < DIM_Y; y++){for(int z = 0; z < DIM_Z; z++){
//       for(int i = 0; i <= 17; i++) {
//         Reactor r1 = r;
//         r1.setCell(x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         /*r1.setCell(DIM - 1 - x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(x, y, DIM - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(DIM - 1 - x, y, DIM - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(x, DIM - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(DIM - 1 - x, DIM - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(x, DIM - 1 - y, DIM - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
//         r1.setCell(DIM - 1 - x, DIM - 1 - y, DIM - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);*/
//
//         double score = pow(objective_fn(r1, f), 1. + (float)idx / 5000);
//         #pragma omp critical
//         {
//           if(!tabuSet.count(r1)) {
//             steps.push_back(r1);
//             step_weights.push_back(score);
//           }
//         }
//       }
//     }}
//   }
//
//   int ret_idx = std::discrete_distribution<int>(step_weights.begin(), step_weights.end())(generator);
//   r = steps[ret_idx];
//
//   tabuSet.insert(r);
//   tabuList.push_back(r);
//
//   if(tabuList.size() > 10000)
//   {
//     Reactor z = tabuList.front();
//     tabuList.pop_front();
//     tabuSet.erase(z);
//   }
//
// }

#define step step_rnd

volatile bool got_sigint = false;
void catch_sigint(int sig) {
  got_sigint = true;
}

int main(int argc, char ** argv)
{

  index_t x = DIM_X, y = DIM_Y, z = DIM_Z;

  if (argc >= 4) {
    x = atoi(argv[1]);
    y = atoi(argv[2]);
    z = atoi(argv[3]);
  }

  FuelType optimizeFuel = OPTIMIZE_FUEL;

  if (argc >= 5) {
    int z = atoi(argv[4]);
    if (z >= 2 && z < static_cast<int>(FuelType::FUEL_TYPE_MAX))
    {
      optimizeFuel = static_cast<FuelType>(z);
    }
  }

  fprintf(stderr, "%d %d %d %s ", x, y, z, fuelNameForFuelType(optimizeFuel).c_str());

  shortCoolerTypes = &shortCoolerTypes_passive;

  if (argc >= 6) {
    switch (atoi(argv[5])) {
      case 0:
        shortCoolerTypes = &shortCoolerTypes_all;
        break;
      case 1:
        shortCoolerTypes = &shortCoolerTypes_passive;
        break;
      case 2:
        shortCoolerTypes = &shortCoolerTypes_active;
        break;
    }
  }

  if (shortCoolerTypes == &shortCoolerTypes_all) {
    fprintf(stderr, "all ");
  }
  else if (shortCoolerTypes == &shortCoolerTypes_passive) {
    fprintf(stderr, "passive ");
  }
  else if (shortCoolerTypes == &shortCoolerTypes_active) {
    fprintf(stderr, "active ");
  }
  else {
    fprintf(stderr, "??? ");
  }

  auto objective_fn = OBJECTIVE_FN;

  if (argc >= 7) {
    switch (atoi(argv[6])) {
      case 0:
        objective_fn = objective_fn_efficiency;
        break;
      case 1:
        objective_fn = objective_fn_output;
        break;
      case 2:
        objective_fn = objective_fn_cells;
        break;
    }
  }

  if (objective_fn == objective_fn_efficiency) {
    fprintf(stderr, "efficiency\n");
  }
  else if (objective_fn == objective_fn_output) {
    fprintf(stderr, "output\n");
  }
  else if (objective_fn == objective_fn_cells) {
    fprintf(stderr, "cells\n");
  }
  else {
    fprintf(stderr, "???\n");
  }

  signal(SIGINT, catch_sigint);

  Reactor r(x, y, z);

  // r.setCell(DIM / 2, DIM / 2, DIM / 2, BlockType::reactorCell, CoolerType::air);

  Reactor best_r = r;

  for(int i = 0; i < 20000; i++)
  {
    if(!(i % 50)) fprintf(stderr, "step %u %f %u %f %f\n", i, objective_fn(r, optimizeFuel), best_r.totalCells(), best_r.effectivePowerGenerated(optimizeFuel), best_r.effectivePowerGenerated(optimizeFuel) / std::max(best_r.totalCells(), (int_fast32_t)1));
    step(r, i, optimizeFuel, objective_fn);
    if(objective_fn(r, optimizeFuel) > objective_fn(best_r, optimizeFuel))
    {
      best_r = r;
    }
    if(!(i % 1000) || (!(i % 50) && objective_fn(r, optimizeFuel) < 10.)) r = best_r;

    if(got_sigint) break;
  }


  printf("-------------------------\n");

  printf("N %d\n", best_r.totalCells());

  printf("P %f\n", best_r.powerGenerated(FuelType::generic) / best_r.totalCells());
  printf("H %f\n", best_r.heatGenerated(FuelType::generic) / best_r.totalCells());
  printf("C %f\n", best_r.heatGenerated(FuelType::air));

  FuelType f = optimizeFuel;
  printf("%s %f %f %f %f\n\n\n", fuelNameForFuelType(f).c_str(), best_r.powerGenerated(f), best_r.heatGenerated(f), best_r.effectivePowerGenerated(f), best_r.effectivePowerGenerated(f) / std::max(best_r.totalCells(), (int_fast32_t)1));
  printf("%s\n", best_r.describe().c_str());
  return 0;
}
