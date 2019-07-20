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
#define OPTIMIZE_FUEL FuelType::LEU235_OX

std::default_random_engine generator;

const std::vector<BlockType> shortBlockTypes = {
  // BlockType::air, //0
  // BlockType::air, //1
  BlockType::reactorCell, //2 (primed)
  BlockType::moderator, //3
  BlockType::moderator, //4
  BlockType::moderator, //5
  // BlockType::moderator, //3
  // BlockType::moderator, //4
  // BlockType::moderator, //5
  BlockType::conductor,
  BlockType::reflector,
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
  BlockType::cooler
};

const std::vector<CoolerType> shortCoolerTypes_all = {
  // CoolerType::air,
  // CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  // CoolerType::air,
  // CoolerType::air,
  // CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::water,
  CoolerType::iron,
  CoolerType::redstone,
  CoolerType::quartz,
  CoolerType::obsidian,
  CoolerType::glowstone,
  CoolerType::lapis,
  CoolerType::gold,
  CoolerType::prismarine,
  CoolerType::purpur,
  CoolerType::diamond,
  CoolerType::emerald,
  CoolerType::copper,
  CoolerType::tin,
  CoolerType::lead,
  CoolerType::boron,
  CoolerType::lithium,
  CoolerType::magnesium,
  CoolerType::manganese,
  CoolerType::aluminum,
  CoolerType::silver,
  CoolerType::helium,
  CoolerType::enderium,
  CoolerType::cryotheum,
  CoolerType::carobbite,
  CoolerType::fluorite,
  CoolerType::villiaumite,
  CoolerType::arsenic,
  CoolerType::tcalloy,
  CoolerType::endstone,
  CoolerType::slime,
  CoolerType::netherbrick
};

const std::vector<CoolerType> shortCoolerTypes_early = {
  // CoolerType::air,
  // CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  // CoolerType::air,
  // CoolerType::air,
  // CoolerType::air,
  CoolerType::air,
  CoolerType::air,
  // CoolerType::water,
  // CoolerType::iron,
  // CoolerType::redstone,
  // CoolerType::quartz,
  // CoolerType::obsidian,
  // CoolerType::glowstone,
  // CoolerType::lapis,
  // CoolerType::gold,
  // CoolerType::copper,
  // CoolerType::tin,
  // CoolerType::lead,
  // CoolerType::boron,
  // CoolerType::lithium,
  // CoolerType::magnesium,
  // CoolerType::manganese,
  // CoolerType::aluminum,
  // CoolerType::silver,
  // CoolerType::netherbrick
};

const std::vector<ModeratorType> shortModeratorTypes_all = {
  // ModeratorType::air,
  // ModeratorType::air,
  ModeratorType::air,
  ModeratorType::graphite,
  ModeratorType::beryllium,
  ModeratorType::heavyWater,
  // ModeratorType::graphite,
  // ModeratorType::beryllium,
  // ModeratorType::heavyWater,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air
};

const std::vector<ModeratorType> shortModeratorTypes_early = {
  // ModeratorType::air,
  // ModeratorType::air,
  ModeratorType::air,
  ModeratorType::graphite,
  ModeratorType::beryllium,
  ModeratorType::heavyWater,
  // ModeratorType::graphite,
  // ModeratorType::beryllium,
  // ModeratorType::heavyWater,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
  ModeratorType::air,
};

const std::vector<CoolerType> * shortCoolerTypes = &shortCoolerTypes_early;
const std::vector<ModeratorType> * shortModeratorTypes = &shortModeratorTypes_early;

float objective_fn_efficiency(Reactor & r, FuelType optimizeFuel)
{
  return 1 + (r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100.
          + r.sandwichedModerators() * 20 + r.numCoolers() * 2 + r.numModerators() + r.totalCells())
          / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
}

float objective_fn_output(Reactor & r, FuelType optimizeFuel)
{
  return 1 + (r.effectivePowerGenerated(optimizeFuel) * 100 * pow(r.dutyCycle(optimizeFuel), 1.1)
          + r.sandwichedModerators() * 10  + r.numCoolers() * 2 + r.numModerators() + r.totalCells() * 400 + r.numConductors())
          / (1 + r.numTrappedCells() /* + r.numValidClusters() */ /* + r.inactiveBlocks() * r.inactiveBlocks() */ /* + std::max(r.numCoolerTypes() - 6, 0) */ /*+ (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0)*/);
}


float objective_fn_cells(Reactor & r, FuelType optimizeFuel)
{
  return (1 + r.totalCells() * r.dutyCycle(optimizeFuel))
          / (1);
}

float keep_fn_efficiency(Reactor & r, FuelType optimizeFuel)
{
  return (r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100.)
  / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
}

float keep_fn_output(Reactor & r, FuelType optimizeFuel)
{
  return 1 + (r.effectivePowerGenerated(optimizeFuel) * 100 * pow(r.dutyCycle(optimizeFuel), 4) + r.totalCells() * 20 + r.numCoolers())
  / (1 /* + r.numValidClusters() */ /* + r.inactiveBlocks() * r.inactiveBlocks() */ /* + std::max(r.numCoolerTypes() - 6, 0) */ /*+ (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0)*/);
}

float keep_fn_cells(Reactor & r, FuelType optimizeFuel)
{
  return (1 + r.totalCells() * r.dutyCycle(optimizeFuel))
  / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
}

#define OBJECTIVE_FN objective_fn_efficiency
#define KEEP_FN keep_fn_efficiency

std::set<Reactor> tabuSet;
std::deque<Reactor> tabuList;

void step_rnd(Reactor & r, int idx, FuelType f, decltype(OBJECTIVE_FN) objective_fn)
{
  std::vector<Reactor> steps;
  std::vector<double> step_weights;

  int switchThresh = std::max(r.x() * 10 + r.y() * 10 + r.z() * 10 - 100, 50);

  // principled extension
  if(idx > switchThresh)
  {
    std::vector<std::tuple<coord_t, BlockType, CoolerType, ModeratorType, float> > principledActions;

    PrincipledSearchMode m = PrincipledSearchMode::computeCooling;

    if(r.isBalanced())
    {
      m = PrincipledSearchMode::optimizeModerators;
    }

    std::set<coord_t> principledLocations = r.suggestPrincipledLocations();
    for(const coord_t & ploc : principledLocations)
    {
      auto suggestedBlocks = r.suggestedBlocksAt(UNPACK(ploc), m);
      for (const auto & tpl : suggestedBlocks)
      {
        principledActions.push_back(std::tuple_cat(std::make_tuple(ploc), tpl));
      }
    }

    if(principledActions.size())
    {
      #pragma omp parallel for
      for(int m = 0; m < 200; m++)
      {
        Reactor r1 = r;

        int nn = std::uniform_int_distribution<int>(1, 4)(generator);
        float s = 0;
        for(int n = 0; n < nn; n++)
        {
          int i = std::uniform_int_distribution<int>(0, principledActions.size() - 1)(generator);
          auto theAction = principledActions[i];

          coord_t where = std::get<0>(theAction);
          BlockType bt = std::get<1>(theAction);
          CoolerType ct = std::get<2>(theAction);
          ModeratorType mt = std::get<3>(theAction);
          float _s = std::get<4>(theAction);

          BlockType bt_at = r1.blockTypeAt(UNPACK(where));
          if(bt_at != BlockType::reactorCell && (bt_at != BlockType::moderator || mt != ModeratorType::air) && bt_at != BlockType::reflector)
          {
            r1.setCell(UNPACK(where), bt, ct, mt, bt == BlockType::reactorCell);
            s += _s;
          }
        }
        double score = std::max(pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000), 0.01);
        #pragma omp critical
        {
          if(!tabuSet.count(r1) || m == 0) {
            steps.push_back(r1);
            step_weights.push_back(score * s);
          }
        }
      }
    }
  }

  // unprincipled random search
  if(idx <= switchThresh || steps.size() == 0)
  {
    #pragma omp parallel for
    for(int m = 0; m < 200; m++)
    {
      int x, y, z, i;

      Reactor r1 = r;

      int nn = std::uniform_int_distribution<int>(1, 8)(generator);;
      for(int n = 0; n < nn; n++) {
        x = std::uniform_int_distribution<int>(0, r.x() - 1)(generator);
        y = std::uniform_int_distribution<int>(0, r.y() - 1)(generator);
        z = std::uniform_int_distribution<int>(0, r.z() - 1)(generator);
        i = std::uniform_int_distribution<int>(0, shortCoolerTypes->size() - 1)(generator);
        r1.setCell(x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
        if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 10000) {
          r1.setCell(r.x() - 1 - x, y, z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
          if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 5000) {
            r1.setCell(x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
            r1.setCell(r.x() - 1 - x, y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
            if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 2000) {
              r1.setCell(x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
              r1.setCell(r.x() - 1 - x, r.y() - 1 - y, z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
              r1.setCell(x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
              r1.setCell(r.x() - 1 - x, r.y() - 1 - y, r.z() - 1 - z, shortBlockTypes[i], (*shortCoolerTypes)[i], (*shortModeratorTypes)[i], true);
            }
          }
        }
      }

      double score = std::max(pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000), 0.01);
      #pragma omp critical
      {
        if(!tabuSet.count(r1) || m == 0) {
          steps.push_back(r1);
          step_weights.push_back(score);
        }
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

  shortCoolerTypes = &shortCoolerTypes_early;

  if (argc >= 6) {
    switch (atoi(argv[5])) {
      case 0:
        shortCoolerTypes = &shortCoolerTypes_all;
        break;
      case 1:
        shortCoolerTypes = &shortCoolerTypes_early;
        break;
    }
  }

  if (shortCoolerTypes == &shortCoolerTypes_all) {
    fprintf(stderr, "all ");
  }
  else if (shortCoolerTypes == &shortCoolerTypes_early) {
    fprintf(stderr, "passive_early ");
  }
  else {
    fprintf(stderr, "??? ");
  }

  auto objective_fn = OBJECTIVE_FN;
  auto keep_fn = KEEP_FN;

  if (argc >= 7) {
    switch (atoi(argv[6])) {
      case 0:
        objective_fn = objective_fn_efficiency;
        keep_fn = keep_fn_efficiency;
        break;
      case 1:
        objective_fn = objective_fn_output;
        keep_fn = keep_fn_output;
        break;
      case 2:
        objective_fn = objective_fn_cells;
        keep_fn = keep_fn_cells;
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
  for(int _x = 0; _x < x; _x++)
  {
    for(int _y = 0; _y < y; _y++)
    {
      for(int _z = 0; _z < z; _z++)
      {
        // int i = std::uniform_int_distribution<int>(0, shortCoolerTypes->size() - 1)(generator);
        r.setCell(_x, _y, _z, BlockType::conductor, CoolerType::air, ModeratorType::air, false);
      }
    }
  }
  // Reactor r = fromInitReactor();

  // r.setCell(DIM / 2, DIM / 2, DIM / 2, BlockType::reactorCell, CoolerType::air);

  Reactor best_r = r;

  int switchThresh = std::max(r.x() * 10 + r.y() * 10 + r.z() * 10 - 100, 50);

  for(int i = 0; i < 20000; i++)
  {
    if(!(i % 50)) {
      if(i % 2000 == 50) {
        fprintf(stderr, "\n");
      }
      fprintf(stderr, "\r");
      for(int j = 0; j < 75; j++)
      {
        fprintf(stderr, " ");
      }
      fprintf(stderr, "\r");
      fprintf(stderr, "step %u %f %u %f %f", i, objective_fn(r, optimizeFuel), best_r.totalCells(), best_r.effectivePowerGenerated(optimizeFuel), best_r.effectivePowerGenerated(optimizeFuel) / std::max(best_r.totalCells(), (int_fast32_t)1));
    }
    step(r, i, optimizeFuel, objective_fn);
    if(keep_fn(r, optimizeFuel) > keep_fn(best_r  , optimizeFuel))
    {
      best_r = r;
    }
    if(!(i % 1000) || (!(i % 500) && objective_fn(r, optimizeFuel) < 10.)) {
      r = best_r;
      r.pruneInactives(true);
      r.floodFillWithConductors();
      objective_fn(r, optimizeFuel);
    }
    if(i == switchThresh)
    {
      r.pruneInactives(true);
      r.pruneInactives(true);
    }

    // consider criteria for early stopping
    // if(r.effectivePowerGenerated(optimizeFuel) > 0 && r.isBalanced()
    //  && r.effectivePowerGenerated(optimizeFuel) > best_r.effectivePowerGenerated(optimizeFuel) * 0.95)
    // {
    //   break;
    // }

    if(got_sigint) break;
  }


  printf("\n-------------------------\n");

  printf("N %d\n", best_r.totalCells());
  printf("Cl %d\n", best_r.numValidClusters());
  printf("O %f\n", objective_fn(best_r, optimizeFuel));
  printf("i %d\n", best_r.inactiveBlocks());
  if(best_r.isSelfSustaining())
  {
    printf("Self-Sustaining!\n");
  }

  FuelType f = optimizeFuel;
  printf("%s %f %f %f\n\n\n", fuelNameForFuelType(f).c_str(), best_r.powerGenerated(f), best_r.effectivePowerGenerated(f), best_r.effectivePowerGenerated(f) / std::max(best_r.totalCells(), (int_fast32_t)1));
  printf("%s\n", best_r.describe().c_str());
  printf("%s\n", best_r.clusterStats().c_str());

  best_r.pruneInactives();
  objective_fn(best_r, optimizeFuel);

  printf("\n-------------------------\n");

  printf("N %d\n", best_r.totalCells());
  printf("Cl %d\n", best_r.numValidClusters());
  printf("O %f\n", objective_fn(best_r, optimizeFuel));
  printf("i %d\n", best_r.inactiveBlocks());
  if(best_r.isSelfSustaining())
  {
    printf("Self-Sustaining!\n");
  }

  f = optimizeFuel;
  printf("%s %f %f %f\n\n\n", fuelNameForFuelType(f).c_str(), best_r.powerGenerated(f), best_r.effectivePowerGenerated(f), best_r.effectivePowerGenerated(f) / std::max(best_r.totalCells(), (int_fast32_t)1));
  printf("%s\n", best_r.describe().c_str());
  printf("%s\n", best_r.clusterStats().c_str());
  return 0;
}
