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
#include <thread>

#include <signal.h>

#include <omp.h>

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
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,
  // BlockType::cooler,

};

const std::vector<CoolerType> shortCoolerTypes_all = {
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
  // CoolerType::enderium,
  // CoolerType::cryotheum,
  // CoolerType::iron,
  // CoolerType::emerald,
  // CoolerType::copper,
  // CoolerType::tin,
  // CoolerType::magnesium,
  // CoolerType::activeCryotheum,
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
  // CoolerType::enderium,
  // CoolerType::cryotheum,
  // CoolerType::iron,
  // CoolerType::emerald,
  // CoolerType::copper,
  // CoolerType::tin,
  // CoolerType::magnesium,
  // CoolerType::activeCryotheum,
};

const std::vector<CoolerType> shortCoolerTypes_passive = {
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
  // CoolerType::enderium,
  // CoolerType::cryotheum,
  // CoolerType::iron,
  // CoolerType::emerald,
  // CoolerType::copper,
  // CoolerType::tin,
  // CoolerType::magnesium,
  // CoolerType::activeCryotheum,
};

const std::vector<CoolerType> * shortCoolerTypes = &shortCoolerTypes_passive;

float objective_fn_efficiency(Reactor & r, FuelType optimizeFuel)
{
  return (1e-10 + r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100000.)
          //- (r.heatGenerated(OPTIMIZE_FUEL) > 0 ? r.effectivePowerGenerated(OPTIMIZE_FUEL) : 0))
          / (0.1 + r.inactiveBlocks() * r.inactiveBlocks() + (r.heatGenerated(optimizeFuel) > 0 ? r.heatGenerated(optimizeFuel) / 10000 : 0));
          // - r.heatGenerated(FuelType::air) / 10;
}

float objective_fn_output(Reactor & r, FuelType optimizeFuel)
{
  return (1e-10 + r.effectivePowerGenerated(optimizeFuel))
          / (0.1 + r.inactiveBlocks() * r.inactiveBlocks() + (r.heatGenerated(optimizeFuel) > 0 ? r.heatGenerated(optimizeFuel) / 10000 : 0))
          - r.heatGenerated(FuelType::air) / 10;
}

float objective_fn_cells(Reactor & r, FuelType optimizeFuel)
{
  float mult = r.heatGenerated(optimizeFuel) <= 0 ? 1 : (r.heatGenerated(FuelType::air) / (r.heatGenerated(FuelType::air) - r.heatGenerated(optimizeFuel)));
  return (1e-10 + r.totalCells() * mult)
          / (1 + r.inactiveBlocks() * r.inactiveBlocks()); // + (r.heatGenerated(optimizeFuel) <= 0 ? 0 : (r.heatGenerated(optimizeFuel)) / 50000));
}

#define OBJECTIVE_FN objective_fn_efficiency

std::set<Reactor> tabuSet;
std::deque<Reactor> tabuList;

void step_rnd(Reactor & r, int idx, FuelType f, decltype(OBJECTIVE_FN) objective_fn)
{
  std::vector<Reactor> steps;
  std::vector<double> step_weights;

  // principled extension
  std::vector<std::tuple<coord_t, BlockType, CoolerType, float> > principledActions;

  std::set<coord_t> principledLocations = r.suggestPrincipledLocations();
  for(const coord_t & ploc : principledLocations)
  {
    BlockType bt = r.blockTypeAt(UNPACK(ploc));
    // if (bt != BlockType::reactorCell && bt != BlockType::moderator) {
      auto suggestedBlocks = r.suggestedBlocksAt(UNPACK(ploc), f);
      for (const auto & tpl : suggestedBlocks)
      {
        principledActions.push_back(std::tuple_cat(std::make_tuple(ploc), tpl));
      }
    // }
  }

  if(principledActions.size())
  {
    // #pragma omp parallel for
    for(int m = 0; m < 100; m++)
    {
      Reactor r1 = r;

      int nn = std::uniform_int_distribution<int>(1, 2)(generator);
      float s = 0;
      for(int n = 0; n < nn; n++)
      {
        int i = std::uniform_int_distribution<int>(0, principledActions.size() - 1)(generator);
        auto theAction = principledActions[i];

        coord_t where = std::get<0>(theAction);
        BlockType bt = std::get<1>(theAction);
        CoolerType ct = std::get<2>(theAction);
        float _s = std::get<3>(theAction);

        int x = where[0];
        int y = where[1];
        int z = where[2];

        r1.setCell(UNPACK(where), bt, ct);
        if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 2000) {
          r1.setCell(r.x() - 1 - x, y, z, bt, ct);
          r1.setCell(x, y, r.z() - 1 - z, bt, ct);
          r1.setCell(r.x() - 1 - x, y, r.z() - 1 - z, bt, ct);
          r1.setCell(x, r.y() - 1 - y, z, bt, ct);
          r1.setCell(r.x() - 1 - x, r.y() - 1 - y, z, bt, ct);
          r1.setCell(x, r.y() - 1 - y, r.z() - 1 - z, bt, ct);
          r1.setCell(r.x() - 1 - x, r.y() - 1 - y, r.z() - 1 - z, bt, ct);
        }
        s += _s;
      }
      double score = std::max(pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000), 0.01);
      // #pragma omp critical
      {
        // if(!tabuSet.count(r1) || m == 0) {
          steps.push_back(r1);
          step_weights.push_back(score * s);
        // }
      }
    }
  }

  // #pragma omp parallel for
  for(int m = 0; m < 50; m++)
  {
    int x, y, z, i;

    Reactor r1 = r;

    int nn = std::uniform_int_distribution<int>(1, 4)(generator);;
    float s = 0;
    for(int n = 0; n < nn; n++) {
      x = std::uniform_int_distribution<int>(0, r.x() - 1)(generator);
      y = std::uniform_int_distribution<int>(0, r.y() - 1)(generator);
      z = std::uniform_int_distribution<int>(0, r.z() - 1)(generator);
      i = std::uniform_int_distribution<int>(0, shortCoolerTypes->size() - 1)(generator);
      // if (shortBlockTypes[i] != BlockType::reactorCell && r.blockTypeAt(x, y, z) != BlockType::reactorCell && r.blockTypeAt(x, y, z) != BlockType::moderator )
      if(1)
      {
        r1.setCell(x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 2000) {
          r1.setCell(r.x() - 1 - x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(r.x() - 1 - x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(r.x() - 1 - x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
          r1.setCell(r.x() - 1 - x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i]);
        }
      }
    }

    double score = pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000);
    // #pragma omp critical
    {
    //   if(!tabuSet.count(r1) || m == 0) {
        steps.push_back(r1);
        step_weights.push_back(score);
    //   }
    }
  }

  int ret_idx = std::discrete_distribution<int>(step_weights.begin(), step_weights.end())(generator);
  r = steps[ret_idx];

  // tabuSet.insert(r);
  // tabuList.push_back(r);

  // if(tabuList.size() > 10000)
  // {
  //   Reactor z = tabuList.front();
  //   tabuList.pop_front();
  //   tabuSet.erase(z);
  // }

}

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
    fprintf(stderr, "whitelisted ");
  }
  else if (shortCoolerTypes == &shortCoolerTypes_passive) {
    fprintf(stderr, "whitelisted ");
  }
  else if (shortCoolerTypes == &shortCoolerTypes_active) {
    fprintf(stderr, "whitelisted ");
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

  if (argc >= 8) {
    std::string fn (argv[7]);
    Reactor * ret = Reactor::fromJsonFile(fn);
    if (ret) {
      r = *ret;
      fprintf(stderr, "loaded %d %d %d reactor from file\n", r.x(), r.y(), r.z());
      delete ret;
    }
  }

  // r.setCell(DIM / 2, DIM / 2, DIM / 2, BlockType::reactorCell, CoolerType::air);

  Reactor best_r = r;

  unsigned int num_threads = omp_get_num_procs() / 2;

  fprintf(stderr, "running %d parallel searches\n", num_threads);
  omp_set_num_threads(num_threads);

  std::vector<Reactor> reactors;
  for(int i = 0; i < num_threads; i++)
  {
    reactors.push_back(r);
  }

  for(int i = 0; i < 20000; i++)
  {
    if(!(i % 50)) fprintf(stderr, "step %u %f %u %f %f\n", i, objective_fn(reactors[0], optimizeFuel), best_r.totalCells(), best_r.effectivePowerGenerated(optimizeFuel), best_r.effectivePowerGenerated(optimizeFuel) / std::max(best_r.totalCells(), (int_fast32_t)1));
    #pragma omp parallel for
    for(int j = 0; j < num_threads; j++) {
      step(reactors[j], i, optimizeFuel, objective_fn);
    }
    for(int j = 0; j < num_threads; j++) {
      if(objective_fn(reactors[j], optimizeFuel) > objective_fn(best_r, optimizeFuel))
      {
        best_r = reactors[j];
      }
      //if(!(i % 250) || (!(i % 250) && objective_fn(reactors[j], optimizeFuel) < 1.)) reactors[j] = best_r;
      if (rand() % 250 == 0) reactors[j] = best_r;
    }
    

    if(got_sigint) break;
  }


  printf("-------------------------\n");

  printf("N %d\n", best_r.totalCells());

  printf("P %f\n", best_r.powerGenerated(FuelType::generic) / best_r.totalCells());
  printf("H %f\n", best_r.heatGenerated(FuelType::generic) / best_r.totalCells());
  printf("C %f\n", best_r.heatGenerated(FuelType::air));

  FuelType f = optimizeFuel;
  printf("%s %f %f %f %f\n\n\n", fuelNameForFuelType(f).c_str(), best_r.powerGenerated(f), best_r.heatGenerated(f), best_r.effectivePowerGenerated(f), best_r.effectivePowerGenerated(f) / std::max(best_r.totalCells(), (int_fast32_t)1));
  std::string desc = best_r.describe();
  printf("%s\n", desc.c_str());

  best_r.toJsonFile("out.json");
  return 0;
}
