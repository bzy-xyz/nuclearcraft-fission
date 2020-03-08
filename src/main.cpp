#include "main.h"

#include <string>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
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

typedef std::tuple<BlockType, CoolerType, ModeratorType, NeutronSourceType, ReflectorType> BlockDefinition;

const std::vector<BlockDefinition> shortBlockDefs_stage1 = {
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::ra_be,     ReflectorType::air),
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::po_be,     ReflectorType::air),
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::cf_252,    ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::graphite,    NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::beryllium,   NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::heavyWater,  NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::conductor,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::conductor,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::conductor,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::reflector,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::beryllium_carbon),
  std::make_tuple(BlockType::reflector,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::lead_steel),
};

const std::vector<BlockDefinition> shortBlockDefs_stage2 = {
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::ra_be,     ReflectorType::air),
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::po_be,     ReflectorType::air),
  std::make_tuple(BlockType::reactorCell,  CoolerType::air,          ModeratorType::air,         NeutronSourceType::cf_252,    ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::graphite,    NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::beryllium,   NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::moderator,    CoolerType::air,          ModeratorType::heavyWater,  NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::conductor,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::reflector,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::beryllium_carbon),
  std::make_tuple(BlockType::reflector,    CoolerType::air,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::lead_steel),
  std::make_tuple(BlockType::cooler,       CoolerType::water,        ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::iron,         ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::redstone,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::quartz,       ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::obsidian,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::glowstone,    ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::lapis,        ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::gold,         ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::prismarine,   ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::purpur,       ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::diamond,      ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::emerald,      ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::copper,       ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::tin,          ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::lead,         ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::boron,        ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::lithium,      ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::magnesium,    ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::manganese,    ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::aluminum,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::silver,       ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::helium,       ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::enderium,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::cryotheum,    ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::carobbite,    ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::fluorite,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::villiaumite,  ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::arsenic,      ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::endstone,     ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::slime,        ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
  std::make_tuple(BlockType::cooler,       CoolerType::netherbrick,  ModeratorType::air,         NeutronSourceType::unprimed,  ReflectorType::air),
};

const std::vector<BlockDefinition> * shortBlockDefs = &shortBlockDefs_stage1;

// float objective_fn_efficiency(Reactor & r, FuelType optimizeFuel)
// {
//   return 1 + (r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100.
//           + r.sandwichedModerators() * 20 + r.numCoolers() * 2 + r.numModerators() + r.totalCells())
//           / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
// }

float objective_fn_output_stage1(Reactor & r, FuelType optimizeFuel)
{
  // return 1 + (r.effectivePowerGenerated(optimizeFuel) * 100 * pow(r.dutyCycle(optimizeFuel), 1.1)
  //         + r.sandwichedModerators() * 10  + r.numCoolers() * 2 + r.numModerators() + r.averageEfficiencyForFuel(optimizeFuel) * 100)
  //         * (pow(0.95, abs(r.heatBalance() / 60 - r.numEmptyBlocks())))
  //         / (1 + r.numTrappedCells() * 10 /* + r.numValidClusters() */ /* + r.inactiveBlocks() * r.inactiveBlocks() */ /* + std::max(r.numCoolerTypes() - 6, 0) */ /*+ (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0)*/);
  return 1 + (r.totalCells() * 1 + r.averageEfficiencyForFuel(optimizeFuel) * 2000 + r.sandwichedModerators() * 100 + r.numModerators() * 10)
         * (pow(0.8, abs(r.heatBalance() / 40 - r.numEmptyBlocks())))
         / (0.1 + r.numTrappedCells() * r.numTrappedCells() * 50 + std::max((long)0, r.totalCells() - 3));
}

float objective_fn_output_stage2(Reactor & r, FuelType optimizeFuel)
{
  return 1 + (r.effectivePowerGenerated(optimizeFuel) * 100 * pow(r.dutyCycle(optimizeFuel), 3) + r.averageEfficiencyForFuel(optimizeFuel) * 10 + r.totalCooling());
          //* (pow(0.99, std::max(r.numEmptyBlocks() - (largeindex_t)(r.volume() * (1.0 - EMPTY_REACTOR_PENALTY_THRESH)), (largeindex_t)0)));
}


// float objective_fn_cells(Reactor & r, FuelType optimizeFuel)
// {
//   return (1 + r.totalCells() * r.dutyCycle(optimizeFuel))
//           / (1);
// }

// float keep_fn_efficiency(Reactor & r, FuelType optimizeFuel)
// {
//   return (r.effectivePowerGenerated(optimizeFuel) / std::max(r.totalCells(), (int_fast32_t)1) + r.effectivePowerGenerated(optimizeFuel) / 100.)
//   / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
// }

float keep_fn_output(Reactor & r, FuelType optimizeFuel)
{
  return 1 + (r.effectivePowerGenerated(optimizeFuel) * 100 * pow(r.dutyCycle(optimizeFuel), 4) + r.totalCells() * 20 + + r.totalCooling() / 25 + r.averageEfficiencyForFuel(optimizeFuel) * 100)
  //* (pow(0.99, std::max(r.numEmptyBlocks() - (largeindex_t)(r.volume() * (1.0 - EMPTY_REACTOR_PENALTY_THRESH)), (largeindex_t)0)))
  / (1 /* + r.numValidClusters() */ /* + r.inactiveBlocks() * r.inactiveBlocks() */ /* + std::max(r.numCoolerTypes() - 6, 0) */ /*+ (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0)*/);
}

// float keep_fn_cells(Reactor & r, FuelType optimizeFuel)
// {
//   return (1 + r.totalCells() * r.dutyCycle(optimizeFuel))
//   / (1 + r.inactiveBlocks() * r.inactiveBlocks() + std::max(r.numCoolerTypes() - 6, 0) + (r.isSelfSustaining() ? r.numPrimedCells() * 2 : 0));
// }

#define OBJECTIVE_FN objective_fn_output_stage1
#define KEEP_FN keep_fn_output

std::set<Reactor> tabuSet;
std::deque<Reactor> tabuList;

void step_rnd(Reactor & r, int idx, FuelType f, decltype(OBJECTIVE_FN) objective_fn)
{
  std::vector<Reactor> steps;
  std::vector<double> step_weights;

  int switchThresh = std::max(r.x() * 20 + r.y() * 20 + r.z() * 20, 50);

  // principled extension
  if(idx > switchThresh)
  {
    std::vector<std::tuple<coord_t, BlockType, CoolerType, ModeratorType, NeutronSourceType, ReflectorType, float> > principledActions;

    PrincipledSearchMode m = PrincipledSearchMode::computeCooling;

    // if(r.effectivePowerGenerated(f))
    // {
    //   m = PrincipledSearchMode::hybrid;
    // }

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

    // if(principledActions.size())
    // {
    //   #pragma omp parallel for
    //   for(int m = 0; m < principledActions.size(); m++)
    //   {
    //     Reactor r1 = r;
    //     float s = 0;
    //     auto theAction = principledActions[m];
    //
    //     coord_t where = std::get<0>(theAction);
    //     BlockType bt = std::get<1>(theAction);
    //     CoolerType ct = std::get<2>(theAction);
    //     ModeratorType mt = std::get<3>(theAction);
    //     float _s = std::get<4>(theAction);
    //
    //     BlockType bt_at = r1.blockTypeAt(UNPACK(where));
    //     if(bt_at != BlockType::reactorCell && (bt_at != BlockType::moderator || mt != ModeratorType::air) && bt_at != BlockType::reflector)
    //     {
    //       r1.setCell(UNPACK(where), bt, ct, mt, bt == BlockType::reactorCell);
    //       s += _s;
    //     }
    //     double score = std::max(pow(objective_fn(r1, f), 1. + (float)(idx % 10000) / 5000), 0.01);
    //     #pragma omp critical
    //     {
    //       if(!tabuSet.count(r1) || m == 0) {
    //         steps.push_back(r1);
    //         step_weights.push_back(score * s);
    //       }
    //     }
    //   }
    // }

    if(principledActions.size())
    {
      #pragma omp parallel for
      for(int m = 0; m < 200; m++)
      {
        Reactor r1 = r;

        int nn = std::uniform_int_distribution<int>(1, 3)(generator);
        float s = 0;
        for(int n = 0; n < nn; n++)
        {
          int i = std::uniform_int_distribution<int>(0, principledActions.size() - 1)(generator);
          auto theAction = principledActions[i];

          coord_t where = std::get<0>(theAction);
          BlockType bt = std::get<1>(theAction);
          CoolerType ct = std::get<2>(theAction);
          ModeratorType mt = std::get<3>(theAction);
          NeutronSourceType nt = std::get<4>(theAction);
          ReflectorType rt = std::get<5>(theAction);
          float _s = std::get<6>(theAction);

          BlockType bt_at = r1.blockTypeAt(UNPACK(where));
          if(bt_at != BlockType::reactorCell && (bt_at != BlockType::moderator || mt != ModeratorType::air) && bt_at != BlockType::reflector)
          {
            r1.setCell(UNPACK(where), bt, ct, mt, nt, rt);
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
  //if(idx <= switchThresh || steps.size() == 0)
  if(1)
  {
    int m_limit = (idx <= switchThresh ? 400 : 100);
    #pragma omp parallel for
    for(int m = 0; m < m_limit; m++)
    {
      int x, y, z, i;

      Reactor r1 = r;

      int nn = std::uniform_int_distribution<int>(1, 4)(generator);;
      for(int n = 0; n < nn; n++) {
        x = std::uniform_int_distribution<int>(0, r.x() - 1)(generator);
        y = std::uniform_int_distribution<int>(0, r.y() - 1)(generator);
        z = std::uniform_int_distribution<int>(0, r.z() - 1)(generator);
        i = std::uniform_int_distribution<int>(0, shortBlockDefs->size() - 1)(generator);
        r1.setCell(x, y, z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        // if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 1000) {
        //   r1.setCell(r.x() - 1 - x, y, z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //   if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 500) {
        //     r1.setCell(x, y, r.z() - 1 - z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //     r1.setCell(r.x() - 1 - x, y, r.z() - 1 - z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //     if(r.x() > 2 && r.y() > 2 && r.z() > 2 && idx < 200) {
        //       r1.setCell(x, r.y() - 1 - y, z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //       r1.setCell(r.x() - 1 - x, r.y() - 1 - y, z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //       r1.setCell(x, r.y() - 1 - y, r.z() - 1 - z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //       r1.setCell(r.x() - 1 - x, r.y() - 1 - y, r.z() - 1 - z, std::get<BlockType>((*shortBlockDefs)[i]), std::get<CoolerType>((*shortBlockDefs)[i]), std::get<ModeratorType>((*shortBlockDefs)[i]), std::get<NeutronSourceType>((*shortBlockDefs)[i]), std::get<ReflectorType>((*shortBlockDefs)[i]));
        //     }
        //   }
        // }
      }

      double score = std::max(pow(objective_fn(r1, f) / (1 + r.centerDist(x, y, z) / std::max(std::max(r.x(), r.y()), r.z())), 1. + (float)(idx % 10000) / 5000), 0.01);
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

  auto objective_fn = objective_fn_output_stage1;
  auto keep_fn = KEEP_FN;

  signal(SIGINT, catch_sigint);

  Reactor r(x, y, z);
  for(int _x = 0; _x < x; _x++)
  {
    for(int _y = 0; _y < y; _y++)
    {
      for(int _z = 0; _z < z; _z++)
      {
        // int i = std::uniform_int_distribution<int>(0, shortCoolerTypes->size() - 1)(generator);
        r.setCell(_x, _y, _z, BlockType::conductor, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air);
      }
    }
  }
  // Reactor r = fromInitReactor();

  // r.setCell(DIM / 2, DIM / 2, DIM / 2, BlockType::reactorCell, CoolerType::air);

  Reactor best_r = r;

  int switchThresh = std::max(r.x() * 20 + r.y() * 20 + r.z() * 20, 50);

  for(int i = 0; i < 20000; i++)
  {
    if(!(i % 5)) {
      if(i % 2000 == 5) {
        fprintf(stderr, "\n");
      }
      fprintf(stderr, "\r");
      for(int j = 0; j < 75; j++)
      {
        fprintf(stderr, " ");
      }
      fprintf(stderr, "\r");
      fprintf(stderr, "step %u %f %u %f %f %f %f %d %f", i, objective_fn(r, optimizeFuel), best_r.totalCells(), best_r.effectivePowerGenerated(optimizeFuel), best_r.effectivePowerGenerated(optimizeFuel) / std::max(best_r.totalCells(), (int_fast32_t)1), best_r.dutyCycle(optimizeFuel), best_r.heatBalance(), best_r.numEmptyBlocks(), best_r.emptyReactorInefficiencyFactor());
    }
    step(r, i, optimizeFuel, objective_fn);
    if(keep_fn(r, optimizeFuel) > keep_fn(best_r  , optimizeFuel))
    {
      best_r = r;
    }
    if(!(i % 50) || (i < switchThresh && !(i % 200))) {
      r = best_r;
      r.pruneInactives(true);
      r.floodFillWithConductors();
      objective_fn(r, optimizeFuel);
    }
    if(i == switchThresh)
    {
      r = best_r;
      r.pruneInactives(true);
      r.pruneInactives(true);
      objective_fn = objective_fn_output_stage2;
      objective_fn(r, optimizeFuel);
    }

    if(!(i % 4000) && best_r.effectivePowerGenerated(optimizeFuel) <= 0)
    {
      r = best_r;
      r.clearInfeasibleClusters();
      objective_fn(r, optimizeFuel);
      r.pruneInactives(true);
      r.pruneInactives(true);
      r.floodFillWithConductors();
      objective_fn(r, optimizeFuel);
      best_r = r;
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

  char outFileName[128];
  snprintf(outFileName, 128, "out_%d_%d_%d_%d_%.2f.json", time(NULL), best_r.x(), best_r.y(), best_r.z(), best_r.effectivePowerGenerated(f));

  FILE * outJson = fopen(outFileName, "w");
  if(outJson)
  {
    std::string rjson = best_r.jsonExport(optimizeFuel);
    fwrite(rjson.data(), sizeof(decltype(rjson)::value_type), rjson.size(), outJson);
    fclose(outJson);
  }

  FILE * outCSV = fopen("log.csv", "a");
  if(outCSV)
  {
    fprintf(outCSV, "\"%s\",%f,%f,%f\n", outFileName, best_r.effectivePowerGenerated(optimizeFuel), best_r.totalHeating(), best_r.totalCooling());
    fclose(outCSV);
  }
  return 0;
}
