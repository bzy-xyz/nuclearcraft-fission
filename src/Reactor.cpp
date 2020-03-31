#include "Reactor.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>
#include <array>

#include <json/json.h>

#define RULESET_VANILLA

static std::map<CoolerType, float> coolerStrengths_E2E = {
  {CoolerType::air, 0},
  {CoolerType::water, 20},
  {CoolerType::redstone, 80},
  {CoolerType::quartz, 80},
  {CoolerType::gold, 120},
  {CoolerType::glowstone, 120},
  {CoolerType::lapis, 100},
  {CoolerType::diamond, 120},
  {CoolerType::liquidHelium, 120},
  {CoolerType::enderium, 140},
  {CoolerType::cryotheum, 140},
  {CoolerType::iron, 60},
  {CoolerType::emerald, 140},
  {CoolerType::copper, 60},
  {CoolerType::tin, 80},
  {CoolerType::magnesium, 100},
  {CoolerType::activeWater, 50},
  {CoolerType::activeCryotheum, 3250},
};

static float fuel_power_E2E[] = {
  0, 6,
  60.0,
  84.0,
  144.0,
  201.6,
  576.0,
  806.4,
  120.0,
  168.0,
  480.0,
  672.0,
  90.0,
  126.0,
  360.0,
  504.0,
  105.0,
  147.0,
  420.0,
  588.0,
  165.0,
  231.0,
  660.0,
  924.0,
  155.4,
  243.6,
  192.0,
  268.8,
  768.0,
  1075.2,
  210.0,
  294.0,
  840.0,
  1176.0,
  162.0,
  226.8,
  648.0,
  907.2,
  138.0,
  193.2,
  552.0,
  772.8,
  135.0,
  189.0,
  540.0,
  756.0,
  216.0,
  302.4,
  864.0,
  1209.6,
  225.0,
  315.0,
  900.0,
  1260.0,
};

static float fuel_heat_E2E[] = {
  0, 1.2,
  18.0,
  22.5,
  60.0,
  75.0,
  360.0,
  450.0,
  50.0,
  62.5,
  300.0,
  375.0,
  36.0,
  45.0,
  216.0,
  270.0,
  40.0,
  50.0,
  240.0,
  300.0,
  70.0,
  87.5,
  420.0,
  525.0,
  57.5,
  97.5,
  94.0,
  117.5,
  564.0,
  705.0,
  112.0,
  140.0,
  672.0,
  840.0,
  68.0,
  85.0,
  408.0,
  510.0,
  54.0,
  67.5,
  324.0,
  405.0,
  52.0,
  65.0,
  312.0,
  390.0,
  116.0,
  145.0,
  696.0,
  870.0,
  120.0,
  150.0,
  720.0,
  900.0
};

static std::map<CoolerType, float> coolerStrengths_vanilla = {
  {CoolerType::air, 0},
  {CoolerType::water, 60},
  {CoolerType::redstone, 90},
  {CoolerType::quartz, 90},
  {CoolerType::gold, 120},
  {CoolerType::glowstone, 130},
  {CoolerType::lapis, 120},
  {CoolerType::diamond, 150},
  {CoolerType::liquidHelium, 140},
  {CoolerType::enderium, 120},
  {CoolerType::cryotheum, 160},
  {CoolerType::iron, 80},
  {CoolerType::emerald, 160},
  {CoolerType::copper, 80},
  {CoolerType::tin, 120},
  {CoolerType::magnesium, 110},
  {CoolerType::activeWater, 150},
  {CoolerType::activeCryotheum, 6400},
};

static float fuel_power_vanilla[] = {
  0, 1,
  60.0,
  84.0,
  144.0,
  201.6,
  576.0,
  806.4,
  120.0,
  168.0,
  480.0,
  672.0,
  90.0,
  126.0,
  360.0,
  504.0,
  105.0,
  147.0,
  420.0,
  588.0,
  165.0,
  231.0,
  660.0,
  924.0,
  155.4,
  243.6,
  192.0,
  268.8,
  768.0,
  1075.2,
  210.0,
  294.0,
  840.0,
  1176.0,
  162.0,
  226.8,
  648.0,
  907.2,
  138.0,
  193.2,
  552.0,
  772.8,
  135.0,
  189.0,
  540.0,
  756.0,
  216.0,
  302.4,
  864.0,
  1209.6,
  225.0,
  315.0,
  900.0,
  1260.0,
};

static float fuel_heat_vanilla[] = {
  0, 1,
  18.0,
  22.5,
  60.0,
  75.0,
  360.0,
  450.0,
  50.0,
  62.5,
  300.0,
  375.0,
  36.0,
  45.0,
  216.0,
  270.0,
  40.0,
  50.0,
  240.0,
  300.0,
  70.0,
  87.5,
  420.0,
  525.0,
  57.5,
  97.5,
  94.0,
  117.5,
  564.0,
  705.0,
  112.0,
  140.0,
  672.0,
  840.0,
  68.0,
  85.0,
  408.0,
  510.0,
  54.0,
  67.5,
  324.0,
  405.0,
  52.0,
  65.0,
  312.0,
  390.0,
  116.0,
  145.0,
  696.0,
  870.0,
  120.0,
  150.0,
  720.0,
  900.0
};

#ifdef RULESET_E2E
static std::map<CoolerType, float> coolerStrengths = coolerStrengths_E2E;
static float * fuel_power = fuel_power_E2E;
static float * fuel_heat = fuel_heat_E2E;
#endif

#ifdef RULESET_VANILLA
static std::map<CoolerType, float> coolerStrengths = coolerStrengths_vanilla;
static float * fuel_power = fuel_power_vanilla;
static float * fuel_heat = fuel_heat_vanilla;
#endif

static char blockTypeShort[] = {
  '_',
  'R',
  'M',
  'C',
  '#',
  '?'
};
static char coolerTypeShort[] = {
  '_',
  'W', // Water
  'R', // Redstone
  'Q', // Quartz
  'G', // Gold
  'O', // glOwstone
  'L', // Lapis
  'D', // Diamond
  'H', // liquidHelium
  'E', // Enderium
  'C', // Cryotheum
  'I', // Iron
  'M', // eMerald
  'P', // coPper
  'S', // Sn (tin)
  'A', // mAgnesium
  'w', // active Water
  'c', // active Cryotheum
  '?'
};

static std::string fuel_names[] = {
    "air",
    "generic",
    "TBU", "TBUO", // 2
    "LEU233", "LEU233O", // 4
    "HEU233", "HEU233O", // 6
    "LEU235", "LEU235O", // 8
    "HEU235", "HEU235O", // 10
    "LEN236", "LEN236O", // 12
    "HEN236", "HEN236O", // 14
    "LEP239", "LEP239O", // 16
    "HEP239", "HEP239O", // 18
    "LEP241", "LEP241O", // 20
    "HEP241", "HEP241O", // 22
    "MOX239", "MOX241", // 24
    "LEA242", "LEA242O", // 26
    "HEA242", "HEA242O", // 28
    "LECm243", "LECm243O", // 30
    "HECm243", "HECm243O", // 32
    "LECm245", "LECm245O", // 34
    "HECm245", "HECm245O", // 36
    "LECm247", "LECm247O", // 38
    "HECm247", "HECm247O", // 40
    "LEB248", "LEB248O", // 42
    "HEB248", "HEB248O", // 44
    "LECf249", "LECf249O", // 46
    "HECf249", "HECf249O", // 48
    "LECf251", "LECf251O", // 50
    "HECf251", "HECf251O", // 52
    "FUEL_TYPE_MAX"
};

std::map<std::string, std::tuple<BlockType, CoolerType> > stringToBlockType = {
  {"FuelCell", std::make_tuple(BlockType::reactorCell, CoolerType::air)},
  {"Graphite", std::make_tuple(BlockType::moderator, CoolerType::air)},
  {"Beryllium", std::make_tuple(BlockType::moderator, CoolerType::air)},
  {"Water", std::make_tuple(BlockType::cooler, CoolerType::water)},
  {"Redstone", std::make_tuple(BlockType::cooler, CoolerType::redstone)},
  {"Quartz", std::make_tuple(BlockType::cooler, CoolerType::quartz)},
  {"Gold", std::make_tuple(BlockType::cooler, CoolerType::gold)},
  {"Glowstone", std::make_tuple(BlockType::cooler, CoolerType::glowstone)},
  {"Lapis", std::make_tuple(BlockType::cooler, CoolerType::lapis)},
  {"Diamond", std::make_tuple(BlockType::cooler, CoolerType::diamond)},
  {"Helium", std::make_tuple(BlockType::cooler, CoolerType::liquidHelium)},
  {"Enderium", std::make_tuple(BlockType::cooler, CoolerType::enderium)},
  {"Cryotheum", std::make_tuple(BlockType::cooler, CoolerType::cryotheum)},
  {"Iron", std::make_tuple(BlockType::cooler, CoolerType::iron)},
  {"Emerald", std::make_tuple(BlockType::cooler, CoolerType::emerald)},
  {"Copper", std::make_tuple(BlockType::cooler, CoolerType::copper)},
  {"Tin", std::make_tuple(BlockType::cooler, CoolerType::tin)},
  {"Magnesium", std::make_tuple(BlockType::cooler, CoolerType::magnesium)},
  {"Active Cryotheum", std::make_tuple(BlockType::cooler, CoolerType::activeCryotheum)},
};

Reactor * Reactor::fromJsonFile(std::string fn)
{
  Json::Value root;
  std::ifstream fs(fn, std::ifstream::binary);

  if (!fs.is_open())
  {
    fprintf(stderr, "couldn't open json file :(");
    return nullptr;
  }

  fs >> root;

  Reactor * out = new Reactor(root["InteriorDimensions"]["X"].asInt(), root["InteriorDimensions"]["Y"].asInt(), root["InteriorDimensions"]["Z"].asInt());

  for (std::string & k : root["CompressedReactor"].getMemberNames()) {
    Json::Value v = root["CompressedReactor"][k];
    for (Json::Value & coords : v) {
      std::tuple<BlockType, CoolerType> & blockInfo = stringToBlockType[k];
      out->setCell(
        coords["X"].asInt() - 1, coords["Y"].asInt() - 1, coords["Z"].asInt() - 1,
        std::get<BlockType>(blockInfo), std::get<CoolerType>(blockInfo)
      );
    }
  }

  return out;
}

const std::string & fuelNameForFuelType(FuelType f) {
  return fuel_names[static_cast<int>(f)];
}

Reactor::Reactor(index_t x, index_t y, index_t z) {
  _blocks = std::vector<BlockType>(x * y * z, BlockType::air);
  _coolerTypes = std::vector<CoolerType>(x * y * z, CoolerType::air);
  _cellActiveCache = std::vector<int>(x * y * z, 0);

  _x = x;
  _y = y;
  _z = z;

  _dirty = true;

  offsets = {
    1,
    -1,
    _z,
    -_z,
    _y * _z,
    -_y * _z
  };
}

Reactor::~Reactor() {
}

bool Reactor::coolerTypeActiveAt(index_t x, index_t y, index_t z, CoolerType ct) {
  switch(ct)
  {
    case CoolerType::water:
      return reactorCellsAdjacentTo(x, y, z) + activeModeratorsAdjacentTo(x, y, z);
    case CoolerType::redstone:
      return reactorCellsAdjacentTo(x, y, z);
    case CoolerType::quartz:
      return activeModeratorsAdjacentTo(x, y, z);
    case CoolerType::gold:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water)
          && activeCoolersAdjacentTo(x, y, z, CoolerType::redstone);
    case CoolerType::glowstone:
      return activeModeratorsAdjacentTo(x, y, z) >= 2;
    case CoolerType::lapis:
      return reactorCellsAdjacentTo(x, y, z) && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::diamond:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water)
          && activeCoolersAdjacentTo(x, y, z, CoolerType::quartz);
    case CoolerType::liquidHelium:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::redstone) == 1
          && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::enderium:
      return reactorCasingsAdjacentTo(x, y, z) == 3;
    case CoolerType::cryotheum:
      return reactorCellsAdjacentTo(x, y, z) >= 2;
    case CoolerType::iron:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::gold);
    case CoolerType::emerald:
      return activeModeratorsAdjacentTo(x, y, z) && reactorCellsAdjacentTo(x, y, z);
    case CoolerType::copper:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::glowstone);
    case CoolerType::tin:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::lapis) >= 2 && (
           (coolerTypeAt(x-1, y, z) == CoolerType::lapis && coolerTypeActiveAt(x-1, y, z, CoolerType::lapis)
           && coolerTypeAt(x+1, y, z) == CoolerType::lapis && coolerTypeActiveAt(x+1, y, z, CoolerType::lapis))
        || (coolerTypeAt(x, y-1, z) == CoolerType::lapis && coolerTypeActiveAt(x, y-1, z, CoolerType::lapis)
           && coolerTypeAt(x, y+1, z) == CoolerType::lapis && coolerTypeActiveAt(x, y+1, z, CoolerType::lapis))
        || (coolerTypeAt(x, y, z-1) == CoolerType::lapis && coolerTypeActiveAt(x, y, z-1, CoolerType::lapis)
           && coolerTypeAt(x, y, z+1) == CoolerType::lapis && coolerTypeActiveAt(x, y, z+1, CoolerType::lapis))
      );
    case CoolerType::magnesium:
      return activeModeratorsAdjacentTo(x, y, z) && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::activeCryotheum:
      return reactorCellsAdjacentTo(x, y, z) >= 2 && _hasPathToOutside(x, y, z);
    case CoolerType::activeWater:
      return (reactorCellsAdjacentTo(x, y, z) + activeModeratorsAdjacentTo(x, y, z) > 0) && _hasPathToOutside(x, y, z);
    default:
      return false;
  }
}

bool Reactor::_hasPathToOutside(index_t x, index_t y, index_t z) {
  std::set<coord_t> visited;

  visited.insert({x, y, z});

  if (_hasPathToOutside_initStep(x-1, y, z, visited)
  || _hasPathToOutside_initStep(x+1, y, z, visited)
  || _hasPathToOutside_initStep(x, y-1, z, visited)
  || _hasPathToOutside_initStep(x, y+1, z, visited)
  || _hasPathToOutside_initStep(x, y, z-1, visited)
  || _hasPathToOutside_initStep(x, y, z+1, visited)) {
    return true;
  }

  return false;
}

bool Reactor::_hasPathToOutside_initStep(index_t x, index_t y, index_t z, std::set<coord_t> & visited) {
  visited.insert({x, y, z});

  if (blockTypeAt(x, y, z) != BlockType::air) {
    return false;
  }

  if (!visited.count({x-1, y, z}) && _hasPathToOutside_followStep(x-1, y, z, visited)) {
    return true;
  }
  if (!visited.count({x+1, y, z}) && _hasPathToOutside_followStep(x+1, y, z, visited)) {
    return true;
  }
  if (!visited.count({x, y-1, z}) && _hasPathToOutside_followStep(x, y-1, z, visited)) {
    return true;
  }
  if (!visited.count({x, y+1, z}) && _hasPathToOutside_followStep(x, y+1, z, visited)) {
    return true;
  }
  if (!visited.count({x, y, z-1}) && _hasPathToOutside_followStep(x, y, z-1, visited)) {
    return true;
  }
  if (!visited.count({x, y, z+1}) && _hasPathToOutside_followStep(x, y, z+1, visited)) {
    return true;
  }

  return false;
}

bool Reactor::_hasPathToOutside_followStep(index_t x, index_t y, index_t z, std::set<coord_t> & visited) {
  visited.insert({x, y, z});

  if (blockTypeAt(x, y, z) == BlockType::casing) {
    return true;
  }

  if (blockTypeAt(x, y, z) != BlockType::air) {
    return false;
  }

  if (!visited.count({x-1, y, z}) && _hasPathToOutside_followStep(x-1, y, z, visited)) {
    return true;
  }
  if (!visited.count({x+1, y, z}) && _hasPathToOutside_followStep(x+1, y, z, visited)) {
    return true;
  }
  if (!visited.count({x, y-1, z}) && _hasPathToOutside_followStep(x, y-1, z, visited)) {
    return true;
  }
  if (!visited.count({x, y+1, z}) && _hasPathToOutside_followStep(x, y+1, z, visited)) {
    return true;
  }
  if (!visited.count({x, y, z-1}) && _hasPathToOutside_followStep(x, y, z-1, visited)) {
    return true;
  }
  if (!visited.count({x, y, z+1}) && _hasPathToOutside_followStep(x, y, z+1, visited)) {
    return true;
  }

  return false;
}

smallcount_t Reactor::_blockTypeAdjacentTo(index_t x, index_t y, index_t z, BlockType bt) {
  smallcount_t ret = 0;

  if (blockTypeAt(x-1, y, z) == bt) ret++;
  if (blockTypeAt(x+1, y, z) == bt) ret++;
  if (blockTypeAt(x, y-1, z) == bt) ret++;
  if (blockTypeAt(x, y+1, z) == bt) ret++;
  if (blockTypeAt(x, y, z-1) == bt) ret++;
  if (blockTypeAt(x, y, z+1) == bt) ret++;

  return ret;
}

smallcount_t Reactor::reactorCellsAdjacentTo(index_t x, index_t y, index_t z) {
  if (blockTypeAt(x, y, z) != BlockType::reactorCell) {
    return _blockTypeAdjacentTo(x, y, z, BlockType::reactorCell);
  }

  smallcount_t ret = 0;

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x-i, y, z) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x-i, y, z) != BlockType::moderator) {
      break;
    }
  }

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x+i, y, z) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x+i, y, z) != BlockType::moderator) {
      break;
    }
  }

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x, y-i, z) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x, y-i, z) != BlockType::moderator) {
      break;
    }
  }

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x, y+i, z) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x, y+i, z) != BlockType::moderator) {
      break;
    }
  }

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x, y, z-i) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x, y, z-i) != BlockType::moderator) {
      break;
    }
  }

  for(int i = 1; i <= 5; i++) {
    if (blockTypeAt(x, y, z+i) == BlockType::reactorCell) {
      ret++;
      break;
    }
    if (blockTypeAt(x, y, z+i) != BlockType::moderator) {
      break;
    }
  }

  return ret;
}

smallcount_t Reactor::activeModeratorsAdjacentTo(index_t x, index_t y, index_t z) {
  smallcount_t ret = 0;

  // if (_dirty) {
  //   _cellModeratorAdjacencyCache = std::vector<int>(_x * _y * _z, -1);
  // }

  if (_cellModeratorAdjacencyCache[x * (_y * _z) + y * (_z) + z] != -1) {
    return _cellModeratorAdjacencyCache[x * (_y * _z) + y * (_z) + z];
  }

  if (blockTypeAt(x-1, y, z) == BlockType::moderator && moderatorActiveAt(x-1, y, z)) ret++;
  if (blockTypeAt(x+1, y, z) == BlockType::moderator && moderatorActiveAt(x+1, y, z)) ret++;
  if (blockTypeAt(x, y-1, z) == BlockType::moderator && moderatorActiveAt(x, y-1, z)) ret++;
  if (blockTypeAt(x, y+1, z) == BlockType::moderator && moderatorActiveAt(x, y+1, z)) ret++;
  if (blockTypeAt(x, y, z-1) == BlockType::moderator && moderatorActiveAt(x, y, z-1)) ret++;
  if (blockTypeAt(x, y, z+1) == BlockType::moderator && moderatorActiveAt(x, y, z+1)) ret++;

  _cellModeratorAdjacencyCache[x * (_y * _z) + y * (_z) + z] = ret;

  return ret;
}

smallcount_t Reactor::activeCoolersAdjacentTo(index_t x, index_t y, index_t z, CoolerType ct) {
  smallcount_t ret = 0;

  if (coolerTypeAt(x-1, y, z) == ct && coolerActiveAt(x-1, y, z)) ret++;
  if (coolerTypeAt(x+1, y, z) == ct && coolerActiveAt(x+1, y, z)) ret++;
  if (coolerTypeAt(x, y-1, z) == ct && coolerActiveAt(x, y-1, z)) ret++;
  if (coolerTypeAt(x, y+1, z) == ct && coolerActiveAt(x, y+1, z)) ret++;
  if (coolerTypeAt(x, y, z-1) == ct && coolerActiveAt(x, y, z-1)) ret++;
  if (coolerTypeAt(x, y, z+1) == ct && coolerActiveAt(x, y, z+1)) ret++;

  return ret;
}

void Reactor::_evaluate(FuelType ft) {
  if (_dirty) {

    _powerGeneratedCache.clear();
    _heatGeneratedCache.clear();
    _cellActiveCache = std::vector<int>(_x * _y * _z, 0);
    _cellModeratorAdjacencyCache = std::vector<int>(_x * _y * _z, -1);
    _inactiveBlocks = 0;

    _reactorCellCache.clear();
    _moderatorCache.clear();
    _coolerCache.clear();

    for(index_t x = 0; x < _x; x++)
    {
      for(index_t y = 0; y < _y; y++)
      {
        for(index_t z = 0; z < _z; z++)
        {
          switch(blockTypeAt(x, y, z))
          {
            case BlockType::reactorCell:
              _reactorCellCache.push_back(_XYZ(x, y, z));
              break;
            case BlockType::moderator:
              _moderatorCache.push_back(_XYZ(x, y, z));
              break;
            case BlockType::cooler:
              _coolerCache.push_back(_XYZ(x, y, z));
              break;
          }
        }
      }
    }

    _dirty = false;

    float totalCooling = 0, genericPower = 0, genericHeat = 0;

    //#pragma omp parallel for
    for (index_t x = 0; x < _x; x++) {
      for (index_t y = 0; y < _y; y++) {
        for (index_t z = 0; z < _z; z++) {

          BlockType bt = blockTypeAt(x, y, z);

          if (bt == BlockType::reactorCell) {
            smallcount_t adjCellCt = reactorCellsAdjacentTo(x, y, z);
            smallcount_t adjModCt = activeModeratorsAdjacentTo(x, y, z);

            //#pragma omp atomic
            genericPower += (1 + adjCellCt) + (1 + adjCellCt) * (adjModCt / 6.0);
            //#pragma omp atomic
            genericHeat += (adjCellCt + 1) * (adjCellCt + 2) / 2.0 + (1 + adjCellCt) * (adjModCt / 3.0);
          }
          else if (bt == BlockType::cooler) {
            //#pragma omp atomic
            if (coolerActiveAt(x, y, z)) {
              totalCooling -= coolerStrengths[coolerTypeAt(x, y, z)];
            }
            else {
              _inactiveBlocks += 1;
            }
          }
          else if (bt == BlockType::moderator) {
            if (!reactorCellsAdjacentTo(x, y, z)) {
              //#pragma omp atomic
              genericHeat += 1;
              _inactiveBlocks += 1;
            }
          }
        }
      }
    }

    _powerGeneratedCache[FuelType::air] = 0;
    _powerGeneratedCache[FuelType::generic] = genericPower * fuel_power[static_cast<int>(FuelType::generic)];
    _heatGeneratedCache[FuelType::air] = totalCooling;
    _heatGeneratedCache[FuelType::generic] = genericHeat * fuel_heat[static_cast<int>(FuelType::generic)];

  }

  if(!_powerGeneratedCache.count(ft))
  {
    _powerGeneratedCache[ft] = _powerGeneratedCache[FuelType::generic] * fuel_power[static_cast<int>(ft)];
    _heatGeneratedCache[ft] = _heatGeneratedCache[FuelType::generic] * fuel_heat[static_cast<int>(ft)] + _heatGeneratedCache[FuelType::air];
  }
}

std::set<coord_t> Reactor::suggestPrincipledLocations()
{
  std::set<coord_t> ret;

  // cells collinear with existing reactor cells
  for (const int & c : _reactorCellCache)
  {
    // fprintf(stderr, "%d %d %d %d\n", c, TO_XYZ(c));
    ret.insert({TO_XYZ(c)});
    for (const auto & o : offsets)
    {
      int n = c + o;
      int i = 0;
      while (i < 4 && n < (int_fast16_t)_x * (int_fast16_t)_y * (int_fast16_t)_z && n >= 0)
      {
        ret.insert({TO_XYZ(n)});
        n += o;
        ++i;
      }
    }
  }

  // cells that are, or are adjacent to existing coolers
  for (const auto & c : _coolerCache)
  {
    // fprintf(stderr, "%d %d %d %d\n", c, TO_XYZ(c));
    ret.insert({TO_XYZ(c)});
    for (const auto & o : offsets)
    {
      if (c + o < (int_fast16_t)_x * (int_fast16_t)_y * (int_fast16_t)_z && c + o >= 0)
      {
        ret.insert({TO_XYZ(c + o)});
      }
    }
  }

  // cells adjacent to moderators that can support heatsinks
  for (const auto & c : _moderatorCache)
  {
    // fprintf(stderr, "%d %d %d %d\n", c, TO_XYZ(c));
    if (moderatorActiveAt(TO_XYZ(c)))
    {
      for (const auto & o : offsets)
      {
        if (c + o < (int_fast16_t)_x * (int_fast16_t)_y * (int_fast16_t)_z && c + o >= 0)
        {
          ret.insert({TO_XYZ(c + o)});
        }
      }
    }
  }

  return ret;
}

std::vector<std::tuple<BlockType, CoolerType, float> > Reactor::suggestedBlocksAt(index_t x, index_t y, index_t z, FuelType ft) {
  std::vector<std::tuple<BlockType, CoolerType, float> > ret;

  ret.push_back(std::make_tuple(BlockType::air, CoolerType::air, 0.1));

  float heatFactor = 1;
  if (heatGenerated(ft) > 0 && heatGenerated(FuelType::air) < 0)
  {
    heatFactor = heatGenerated(FuelType::air) / (heatGenerated(FuelType::air) - heatGenerated(ft));
    heatFactor = heatFactor * heatFactor * heatFactor;
  }
  ret.push_back(std::make_tuple(BlockType::reactorCell, CoolerType::air, 0.1 * heatFactor));

  // would a moderator be active if placed here?
  if(moderatorActiveAt(x, y, z)) {
    ret.push_back(std::make_tuple(BlockType::moderator, CoolerType::air, 1));
  }
  else {
    ret.push_back(std::make_tuple(BlockType::moderator, CoolerType::air, 0.1));
  }

  // would a cooler be active if placed here?
  for (int cti = 1; cti < static_cast<int>(CoolerType::COOLER_TYPE_MAX); cti++)
  {
    CoolerType ct = static_cast<CoolerType>(cti);
    std::set<CoolerType> whitelist = {
      CoolerType::redstone, CoolerType::gold, CoolerType::diamond,
      CoolerType::iron, CoolerType::lapis, CoolerType::tin,
      CoolerType::glowstone, CoolerType::quartz, CoolerType::copper, CoolerType::magnesium, 
      CoolerType::cryotheum, CoolerType::enderium, CoolerType::liquidHelium,
      // CoolerType::activeCryotheum,
    };
    if(whitelist.count(ct) > 0 && coolerTypeActiveAt(x, y, z, ct) && coolerTypeAt(x, y, z) != ct)
    {
      ret.push_back(std::make_tuple(BlockType::cooler, ct, 1));
    }
  }

  return ret;
}

std::string Reactor::describe() {
  std::string r;
  for (int z = 0; z < _z; z++) {
    for (int y = 0; y < _y; y++) {
      for (int x = 0; x < _x; x++) {
        auto bt = blockTypeAt(x,y,z);
        r += blockTypeShort[static_cast<int>(bt)];
        if(bt == BlockType::cooler)
        {
          r += coolerTypeShort[static_cast<int>(coolerTypeAt(x,y,z))];
        }
        else
        {
          r += blockTypeShort[static_cast<int>(bt)];
        }
        r += ' ';
      }
      r += '\n';
    }
    r += '\n';
  }
  return r;
}

void Reactor::toJsonFile(std::string fn) {
  Json::Value out;
  out["SaveVersion"]["Major"] = 1;
  out["SaveVersion"]["Minor"] = 2;
  out["SaveVersion"]["Build"] = 23;
  out["SaveVersion"]["Revision"] = 0;
  out["SaveVersion"]["MajorRevision"] = 0;
  out["SaveVersion"]["MinorRevision"] = 0;

  std::map<std::tuple<BlockType, CoolerType>, std::string> blockTypeToString;

  for (const auto & i : stringToBlockType)
  {
    // out["CompressedReactor"][i.first] = Json::Value(Json::ValueType::arrayValue);
    blockTypeToString[i.second] = i.first;
  }

  for (int x = 0; x < _x; x++)
  {
    for (int y = 0; y < _y; y++)
    {
      for (int z = 0; z < _z; z++)
      {
        BlockType bt = blockTypeAt(x, y, z);
        CoolerType ct = coolerTypeAt(x, y, z);
        
        if (bt != BlockType::air) {

          Json::Value coord = Json::Value(Json::ValueType::objectValue);
          coord["X"] = x + 1;
          coord["Y"] = y + 1;
          coord["Z"] = z + 1;

          std::string blockType = blockTypeToString[std::make_tuple(bt, ct)];
          if(!blockType.empty()) {
            out["CompressedReactor"][blockType].append(coord);
          }
        }
      }
    }
  }

  out["InteriorDimensions"]["X"] = _x;
  out["InteriorDimensions"]["Y"] = _y;
  out["InteriorDimensions"]["Z"] = _z;

  out["UsedFuel"]["Name"] = "LEU-235 Oxide";
  out["UsedFuel"]["BasePower"] = 168.0;
  out["UsedFuel"]["BaseHeat"] = 62.5;
  out["UsedFuel"]["FuelTime"] = 72000.0;

  std::ofstream outfile(fn, std::ios_base::binary);

  outfile << out;
}
