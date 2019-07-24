#include "Reactor.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>
#include <numeric>
#include <algorithm>
#include <set>
#include <array>

#define ADJACENT(self, other) (_cellAdjacencyCache[_XYZV(self)].count(_XYZV(other)))
#define MAKE_ADJACENT(self, other) (_cellAdjacencyCache[_XYZV(self)].insert(_XYZV(other)))

//#define VECTORISE(i) (coord_t((int)(i / (_y * _z)), (int)((i / _z) % _y), (int)(i % (_z))))

static std::map<CoolerType, float> coolerStrengths_vanilla = {
  {CoolerType::air, 0},
  {CoolerType::water, 50},
  {CoolerType::iron, 55},
  {CoolerType::redstone, 85},
  {CoolerType::quartz, 75},
  {CoolerType::obsidian, 70},
  {CoolerType::glowstone, 115},
  {CoolerType::lapis, 95},
  {CoolerType::gold, 100},
  {CoolerType::prismarine, 110},
  {CoolerType::purpur, 90},
  {CoolerType::diamond, 180},
  {CoolerType::emerald, 190},
  {CoolerType::copper, 80},
  {CoolerType::tin, 120},
  {CoolerType::lead, 65},
  {CoolerType::boron, 165},
  {CoolerType::lithium, 130},
  {CoolerType::magnesium, 135},
  {CoolerType::manganese, 140},
  {CoolerType::aluminum, 175},
  {CoolerType::silver, 170},
  {CoolerType::helium, 195},
  {CoolerType::enderium, 185},
  {CoolerType::cryotheum, 200},
  {CoolerType::carobbite, 160},
  {CoolerType::fluorite, 150},
  {CoolerType::villiaumite, 155},
  {CoolerType::arsenic, 145},
  {CoolerType::tcalloy, 205},
  {CoolerType::endstone, 60},
  {CoolerType::slime, 125},
  {CoolerType::netherbrick, 105}
};

const static float fuel_eff_vanilla[] = {
  0, 1,
  1,  1,  1,
  1.1, 1.1, 1.1,
  1.1, 1.1, 1.1,
  1, 1, 1,
  1, 1, 1,
  1.1, 1.1, 1.1,
  1.1, 1.1, 1.1,
  1.2, 1.2, 1.2,
  1.2, 1.2, 1.2,
  1.25, 1.25, 1.25,
  1.25, 1.25, 1.25,
  1.05, 1.05, 1.05,
  1.15, 1.15, 1.15,
  1.35, 1.35, 1.35,
  1.35, 1.35, 1.35,
  1.45, 1.45, 1.45,
  1.45, 1.45, 1.45,
  1.5, 1.5, 1.5,
  1.5, 1.5, 1.5,
  1.55, 1.55, 1.55,
  1.55, 1.55, 1.55,
  1.65, 1.65, 1.65,
  1.65, 1.65, 1.65,
  1.75, 1.75, 1.75,
  1.75, 1.75, 1.75,
  1.8, 1.8, 1.8,
  1.8, 1.8, 1.8,
  0
};

const static float fuel_heat_vanilla[] = {
  0, 1,
  40,	32,	48,
  144,	115,	173,
  432,	345,	519,
  120,	96,	144,
  360,	288,	432,
  90,	72,	108,
  270,	216,	324,
  105,	84,	126,
  315,	252,	378,
  165,	132,	198,
  495,	396,	594,
  112,	90,	134,
  142,	114,	170,
  192,	154,	230,
  576,	462,	690,
  210,	168,	252,
  630,	504,	756,
  162,	130,	194,
  486,	390,	582,
  138,	110,	166,
  414,	330,	498,
  135,	108,	162,
  405,	324,	486,
  216,	173,	259,
  648,	519,	777,
  225,	180,	270,
  675,	540,	810,
  0
};

const static float fuel_crit_vanilla[] = {
  0, 1,
  17.6,	22,	15,
  7.8,	9.8,	6.6,
  3.9,	4.9,	3.3,
  8.5,	10.6,	7.2,
  4.2,	5.3,	3.6,
  9.4,	11.8,	8,
  4.7,	5.9,	4,
  8.9,	11.1,	7.6,
  4.4,	5.5,	3.8,
  7.3,	9.1,	6.2,
  3.6,	4.5,	3.1,
  8.7,	10.9,	7.4,
  7.9,	9.9,	6.7,
  6.5,	8.1,	5.5,
  3.2,	4.0,	2.7,
  6.2,	7.8,	5.3,
  3.1,	3.9,	2.6,
  7.4,	9.3,	6.3,
  3.7,	4.6,	3.1,
  8.0,	10,	6.8,
  4.0,	5,	3.4,
  8.1,	10.1,	6.9,
  4.0,	5,	3.4,
  6.1,	7.6,	5.2,
  3.0,	3.8,	2.6,
  5.8,	7.3,	4.9,
  2.9,	3.6,	2.5,
  0
};

const static float moderator_flux_vanilla[] = {
  0, 2.2, 1.0, 3.6, 0
};

const static float moderator_eff_vanilla[] = {
  0, 1.1, 1.2, 1.0, 0
};

static std::map<CoolerType, float> coolerStrengths = coolerStrengths_vanilla;
const static float * fuel_eff = fuel_eff_vanilla;
const static float * fuel_heat = fuel_heat_vanilla;
const static float * fuel_crit = fuel_crit_vanilla;
const static float * moderator_flux = moderator_flux_vanilla;
const static float * moderator_eff = moderator_eff_vanilla;

static const char * blockTypeShort[] = {
  "_",
  "r",
  "M",
  "C",
  "#",
  "F",
  "?"
};
static const char * coolerTypeShort[] = {
  "__",
  "WA",
  "FE",
  "RE",
  "QU",
  "OB",
  "GL",
  "LA",
  "AU",
  "PR",
  "PU",
  "DI",
  "EM",
  "CU",
  "SN",
  "PB",
  "BO",
  "LI",
  "MG",
  "MN",
  "AL",
  "AG",
  "HE",
  "EN",
  "CR",
  "CA",
  "FL",
  "VI",
  "AS",
  "TC",
  "ES",
  "SL",
  "NB",
  "??"
};
static const char * moderatorTypeShort[] = {
  "__",
  "BE",
  "GR",
  "HW",
  "??"
};

static const char * coolerTypeLong[] = {
  "Air",
  "Water",
  "Iron",
  "Redstone",
  "Quartz",
  "Obsidian",
  "Glowstone",
  "Lapis",
  "Gold",
  "Prismarine",
  "Purpur",
  "Diamond",
  "Emerald",
  "Copper",
  "Tin",
  "Lead",
  "Boron",
  "Lithium",
  "Magnesium",
  "Manganese",
  "Aluminum",
  "Silver",
  "Helium",
  "Enderium",
  "Cryotheum",
  "Carobbiite",
  "Fluorite",
  "Villiaumite",
  "Arsenic",
  "TCAlloy",
  "EndStone",
  "Slime",
  "NetherBrick"
};

static const char * moderatorTypeLong[] = {
  "Air",
  "Beryllium",
  "Graphite",
  "HeavyWater"
};



#define CREATE_FUEL_STRINGS(__FUEL_NAME) "[OX]" __FUEL_NAME, "[NI]" __FUEL_NAME, "[ZA]" __FUEL_NAME

static std::string fuel_names[] = {
    "air",
    "generic",
    CREATE_FUEL_STRINGS("TBU"),
    CREATE_FUEL_STRINGS("LEU-233"),
    CREATE_FUEL_STRINGS("HEU-233"),
    CREATE_FUEL_STRINGS("LEU-235"),
    CREATE_FUEL_STRINGS("HEU-235"),
    CREATE_FUEL_STRINGS("LEN-236"),
    CREATE_FUEL_STRINGS("HEN-236"),
    CREATE_FUEL_STRINGS("LEP-239"),
    CREATE_FUEL_STRINGS("HEP-239"),
    CREATE_FUEL_STRINGS("LEP241"),
    CREATE_FUEL_STRINGS("HEP241"),
    "[OX]MOX-239", "[NI]MNI-239", "[ZA]MZA-239",
    "[OX]MOX-241", "[NI]MNI-241", "[ZA]MZA-241",
    CREATE_FUEL_STRINGS("LEA-242"),
    CREATE_FUEL_STRINGS("HEA-242"),
    CREATE_FUEL_STRINGS("LECm-243"),
    CREATE_FUEL_STRINGS("HECm-243"),
    CREATE_FUEL_STRINGS("LECm-245"),
    CREATE_FUEL_STRINGS("HECm-245"),
    CREATE_FUEL_STRINGS("LECm-247"),
    CREATE_FUEL_STRINGS("HECm-247"),
    CREATE_FUEL_STRINGS("LEB-248"),
    CREATE_FUEL_STRINGS("HEB-248"),
    CREATE_FUEL_STRINGS("LECf-249"),
    CREATE_FUEL_STRINGS("HECf-249"),
    CREATE_FUEL_STRINGS("LECf-251"),
    CREATE_FUEL_STRINGS("HECf-251"),
    "FUEL_TYPE_MAX"
};

const std::string & fuelNameForFuelType(FuelType f) {
  return fuel_names[static_cast<int>(f)];
}

Reactor::Reactor(index_t x, index_t y, index_t z) {
  _blocks = std::vector<BlockType>(x * y * z, BlockType::air);
  _coolerTypes = std::vector<CoolerType>(x * y * z, CoolerType::air);
  _moderatorTypes = std::vector<ModeratorType>(x * y * z, ModeratorType::air);
  _primedStatus = std::vector<int>(x * y * z, 0);

  _x = x;
  _y = y;
  _z = z;

  _dirty = true;

  _revertToSetup();
}

Reactor::~Reactor() {
}

void Reactor::_setBlockActive(index_t x, index_t y, index_t z)
{
  if(isInBounds(x,y,z))
  _cellActiveCache[_XYZ(x,y,z)] = 1;
}

void Reactor::_setBlockValid(index_t x, index_t y, index_t z)
{
  if(isInBounds(x,y,z))
  _cellValidCache[_XYZ(x,y,z)] = 1;
}
void Reactor::_unsetBlockActive(index_t x, index_t y, index_t z)
{
  if(isInBounds(x,y,z))
  _cellActiveCache[_XYZ(x,y,z)] = -1;
}
void Reactor::_unsetBlockValid(index_t x, index_t y, index_t z)
{
  if(isInBounds(x,y,z))
  _cellValidCache[_XYZ(x,y,z)] = -1;
}

bool Reactor::coolerTypeActiveAt(index_t x, index_t y, index_t z, CoolerType ct) {
  switch(ct)
  {
    case CoolerType::water:
      return reactorCellsAdjacentTo(x, y, z);
    case CoolerType::redstone:
      return reactorCellsAdjacentTo(x, y, z) && validModeratorsAdjacentTo(x, y, z);
    case CoolerType::quartz:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::redstone);
    case CoolerType::gold:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::iron) >= 2;
    case CoolerType::glowstone:
      return validModeratorsAdjacentTo(x, y, z) >= 2;
    case CoolerType::lapis:
      return reactorCellsAdjacentTo(x, y, z) && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::diamond:
      return reactorCellsAdjacentTo(x, y, z) && activeCoolersAdjacentTo(x, y, z, CoolerType::gold);
    case CoolerType::helium:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::redstone) == 2 && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::enderium:
      return validModeratorsAdjacentTo(x, y, z) >= 3;
    case CoolerType::cryotheum:
      return reactorCellsAdjacentTo(x, y, z) >= 3;
    case CoolerType::iron:
      return validModeratorsAdjacentTo(x, y, z);
    case CoolerType::emerald:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::prismarine) && validModeratorsAdjacentTo(x, y, z);
    case CoolerType::copper:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water);
    case CoolerType::tin:
      return hasAxialCoolers(x, y, z, CoolerType::lapis);
    case CoolerType::magnesium:
      return validModeratorsAdjacentTo(x, y, z) && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::boron:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::quartz) == 1 && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::prismarine:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water) >= 2;
    case CoolerType::obsidian:
      return hasAxialCoolers(x, y, z, CoolerType::glowstone);
    case CoolerType::lead:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::iron);
    case CoolerType::aluminum:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::quartz) && activeCoolersAdjacentTo(x, y, z, CoolerType::tin);
    case CoolerType::lithium:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::lead) == 1 && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::manganese:
      return reactorCellsAdjacentTo(x, y, z) >= 2;
    case CoolerType::silver:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::glowstone) && activeCoolersAdjacentTo(x, y, z, CoolerType::lapis);
    case CoolerType::purpur:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::iron) == 1 && activeCoolersAdjacentTo(x, y, z, CoolerType::endstone);
    case CoolerType::arsenic:
      return hasAxialReflectors(x, y, z);
    case CoolerType::carobbite:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::copper) && activeCoolersAdjacentTo(x, y, z, CoolerType::endstone);
    case CoolerType::villiaumite:
      return activeReflectorsAdjacentTo(x, y, z) && activeCoolersAdjacentTo(x, y, z, CoolerType::redstone);
    case CoolerType::slime:
      return activeReflectorsAdjacentTo(x, y, z) && activeCoolersAdjacentTo(x, y, z, CoolerType::water) == 1;
    case CoolerType::fluorite:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::gold) && activeCoolersAdjacentTo(x, y, z, CoolerType::prismarine);
    case CoolerType::tcalloy:
      // TODO hasvertex
      return false;
    case CoolerType::netherbrick:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::obsidian);
    case CoolerType::endstone:
      return activeReflectorsAdjacentTo(x, y, z);
    default:
      return false;
  }
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
  smallcount_t ret = 0;

  if (blockTypeAt(x-1, y, z) == BlockType::reactorCell && reactorCellActiveAt(x-1, y, z)) ret++;
  if (blockTypeAt(x+1, y, z) == BlockType::reactorCell && reactorCellActiveAt(x+1, y, z)) ret++;
  if (blockTypeAt(x, y-1, z) == BlockType::reactorCell && reactorCellActiveAt(x, y-1, z)) ret++;
  if (blockTypeAt(x, y+1, z) == BlockType::reactorCell && reactorCellActiveAt(x, y+1, z)) ret++;
  if (blockTypeAt(x, y, z-1) == BlockType::reactorCell && reactorCellActiveAt(x, y, z-1)) ret++;
  if (blockTypeAt(x, y, z+1) == BlockType::reactorCell && reactorCellActiveAt(x, y, z+1)) ret++;

  return ret;
}

smallcount_t Reactor::validModeratorsAdjacentTo(index_t x, index_t y, index_t z) {
  smallcount_t ret = 0;

  if (blockTypeAt(x-1, y, z) == BlockType::moderator && moderatorValidAt(x-1, y, z)) ret++;
  if (blockTypeAt(x+1, y, z) == BlockType::moderator && moderatorValidAt(x+1, y, z)) ret++;
  if (blockTypeAt(x, y-1, z) == BlockType::moderator && moderatorValidAt(x, y-1, z)) ret++;
  if (blockTypeAt(x, y+1, z) == BlockType::moderator && moderatorValidAt(x, y+1, z)) ret++;
  if (blockTypeAt(x, y, z-1) == BlockType::moderator && moderatorValidAt(x, y, z-1)) ret++;
  if (blockTypeAt(x, y, z+1) == BlockType::moderator && moderatorValidAt(x, y, z+1)) ret++;

  return ret;
}

smallcount_t Reactor::activeReflectorsAdjacentTo(index_t x, index_t y, index_t z) {
  smallcount_t ret = 0;

  if (blockTypeAt(x-1, y, z) == BlockType::reflector && reflectorActiveAt(x-1, y, z)) ret++;
  if (blockTypeAt(x+1, y, z) == BlockType::reflector && reflectorActiveAt(x+1, y, z)) ret++;
  if (blockTypeAt(x, y-1, z) == BlockType::reflector && reflectorActiveAt(x, y-1, z)) ret++;
  if (blockTypeAt(x, y+1, z) == BlockType::reflector && reflectorActiveAt(x, y+1, z)) ret++;
  if (blockTypeAt(x, y, z-1) == BlockType::reflector && reflectorActiveAt(x, y, z-1)) ret++;
  if (blockTypeAt(x, y, z+1) == BlockType::reflector && reflectorActiveAt(x, y, z+1)) ret++;

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

bool Reactor::hasAxialCoolers(index_t x, index_t y, index_t z, CoolerType ct)
{
  return (
       (coolerTypeAt(x-1, y, z) == ct && coolerTypeActiveAt(x-1, y, z, ct)
       && coolerTypeAt(x+1, y, z) == ct && coolerTypeActiveAt(x+1, y, z, ct))
    || (coolerTypeAt(x, y-1, z) == ct && coolerTypeActiveAt(x, y-1, z, ct)
       && coolerTypeAt(x, y+1, z) == ct && coolerTypeActiveAt(x, y+1, z, ct))
    || (coolerTypeAt(x, y, z-1) == ct && coolerTypeActiveAt(x, y, z-1, ct)
       && coolerTypeAt(x, y, z+1) == ct && coolerTypeActiveAt(x, y, z+1, ct))
  );
}

bool Reactor::hasAxialReflectors(index_t x, index_t y, index_t z) {
  return (
       (blockTypeAt(x-1, y, z) == BlockType::reflector && reflectorActiveAt(x-1, y, z)
       && blockTypeAt(x+1, y, z) == BlockType::reflector && reflectorActiveAt(x+1, y, z))
    || (blockTypeAt(x, y-1, z) == BlockType::reflector && reflectorActiveAt(x, y-1, z)
       && blockTypeAt(x, y+1, z) == BlockType::reflector && reflectorActiveAt(x, y+1, z))
    || (blockTypeAt(x, y, z-1) == BlockType::reflector && reflectorActiveAt(x, y, z-1)
       && blockTypeAt(x, y, z+1) == BlockType::reflector && reflectorActiveAt(x, y, z+1))
  );
}

void Reactor::_revertToSetup() {
  // TODO: clear these only on dirty reactor
  _powerGeneratedCache.clear();
  _dutyCycleCache.clear();

  _cellActiveCache = std::vector<int>(_x * _y * _z, 0);
  _cellValidCache = std::vector<int>(_x * _y * _z, 0);
  _cellVisitedCache = std::vector<int>(_x * _y * _z, 0);
  _clusterIdCache = std::vector<largeindex_t>(_x * _y * _z, -1);
  _cellPositionalEfficiency = std::vector<float>(_x * _y * _z, 0);
  _cellModeratorFlux = std::vector<float>(_x * _y * _z, 0);
  _fluxedModeratorCache = std::vector<int>(_x * _y * _z, 0);
  _validClusterIds.clear();
  _inactiveBlocks = 0;
  _cellAdjacencyCache.clear();
  _reactorCellObservedCellCount.clear();
  _primedCellCache.clear();

  _reactorCellCache.clear();
  _moderatorCache.clear();
  _reflectorCache.clear();
  _coolerCache.clear();
  _conductorCache.clear();

  _fluxedModerators = 0;
  _sandwichedModerators = 0;

  _largestAssignedClusterId = -1;

  _outputPerCluster.clear();
  _heatingPerCluster.clear();
  _coolingPerCluster.clear();
  _perClusterSumCellEff.clear();
  _perClusterSumHeatMult.clear();
  _perClusterCellCount.clear();
  _perClusterEff.clear();

  _conductorIdCache = std::vector<largeindex_t>(_x * _y * _z, -1);
  _largestAssignedConductorId = -1;
  _validConductorIds.clear();

  for(index_t x = 0; x < _x; x++)
  {
    for(index_t y = 0; y < _y; y++)
    {
      for(index_t z = 0; z < _z; z++)
      {
        coord_t v(x, y, z);
        switch(blockTypeAt(x, y, z))
        {
          case BlockType::reactorCell:
            _reactorCellCache.push_back(v);
            break;
          case BlockType::moderator:
            _moderatorCache.push_back(v);
            break;
          case BlockType::reflector:
            _reflectorCache.push_back(v);
            break;
          case BlockType::cooler:
            _coolerCache.push_back(v);
            break;
          case BlockType::conductor:
            _conductorCache.push_back(v);
            break;
        }
        if(_primedStatus[_XYZV(v)] == 1 && _hasLineOfSightToOutside(UNPACK(v))) {
          _primedCellCache.push_back(v);
        }
      }
    }
  }

}

void Reactor::_fuelCellBroadcastFlux(index_t x, index_t y, index_t z, FuelType ft) {

  if(blockTypeAt(x, y, z) != BlockType::reactorCell)
  {
    return;
  }

  if(_cellVisitedCache[_XYZ(x, y, z)])
  {
    return;
  }

  _cellVisitedCache[_XYZ(x, y, z)] = 1;

  std::vector<coord_t> toBroadcast;

  coord_t n(x, y, z);

  for(const coord_t & o : offsets) {
    coord_t p(x, y, z);
    std::vector<coord_t> moderators_touched;
    float sum_moderator_flux = 0;
    float sum_moderator_eff = 0;
    int moderators_in_line = 0;
    for(int i = 0; i <= NEUTRON_REACH; i++) {
      p += o;

      BlockType b = blockTypeAt(UNPACK(p));

      if(b == BlockType::moderator)
      {
        sum_moderator_flux += moderator_flux[static_cast<int>(moderatorTypeAt(UNPACK(p)))];
        sum_moderator_eff += moderator_eff[static_cast<int>(moderatorTypeAt(UNPACK(p)))];
        ++moderators_in_line;
        moderators_touched.push_back(p);
        ++_fluxedModerators;
        continue;
      }
      else if(b == BlockType::reactorCell && i > 0
        && !ADJACENT(p, n))
      {
        MAKE_ADJACENT(p, n);
        ++_reactorCellObservedCellCount[_XYZV(n)];
        _cellPositionalEfficiency[_XYZV(p)] += sum_moderator_eff / moderators_in_line;
        _cellModeratorFlux[_XYZV(p)] += sum_moderator_flux;

        _sandwichedModerators += moderators_in_line;

        for(const auto & z : moderators_touched)
        {
          _fluxedModeratorCache[_XYZV(z)] = 1;
        }

        if(_cellModeratorFlux[_XYZV(p)] >= fuel_crit[static_cast<int>(ft)])
        {
          _setBlockActive(UNPACK(p));
          _setBlockValid(UNPACK(p));
          if(!_cellVisitedCache[_XYZV(p)]) {
            // _fuelCellBroadcastFlux(UNPACK(p), ft);
            toBroadcast.push_back(p);
          }
        }
        break;
      }
      else if(b == BlockType::reflector)
      {
        if(ADJACENT(n, p))
        {
          if(_cellActiveCache[_XYZV(n)])
          {
            _cellActiveCache[_XYZV(p)] = 1;
          }
          break;
        }
        if(i == 0 || i > NEUTRON_REACH / 2)
        {
          break;
        }
        _cellModeratorFlux[_XYZV(n)] += 2 * sum_moderator_flux;
        _cellPositionalEfficiency[_XYZV(n)] += REFLECTOR_EFFICIENCY * sum_moderator_eff /  moderators_in_line;

        _sandwichedModerators += moderators_in_line;

        MAKE_ADJACENT(p, n);
        if(_cellModeratorFlux[_XYZV(n)] >= fuel_crit[static_cast<int>(ft)])
        {
          _setBlockActive(UNPACK(n));
          _setBlockValid(UNPACK(n));
          _setBlockActive(UNPACK(p));
        }
        MAKE_ADJACENT(n, p);
        break;
      }
      else
      {
        break;
      }
    }
  }

  for(const coord_t & b : toBroadcast)
  {
    _fuelCellBroadcastFlux(UNPACK(b), ft);
  }
}

void Reactor::_fuelCellBroadcastModeratorActivations(index_t x, index_t y, index_t z) {

  if(blockTypeAt(x, y, z) != BlockType::reactorCell || !blockActiveAt(x, y, z))
  {
    return;
  }

  coord_t n(x, y, z);

  for(const coord_t & o : offsets) {
    coord_t p(x, y, z);
    std::vector<coord_t> moderators_touched;
    for(int i = 0; i <= NEUTRON_REACH; i++) {
      p += o;

      BlockType b = blockTypeAt(UNPACK(p));

      if(b == BlockType::moderator)
      {
        moderators_touched.push_back(p);
        continue;
      }
      else if(b == BlockType::reactorCell && i > 0)
      {
        if(blockValidAt(UNPACK(p))) {
          _setBlockValid(UNPACK(n + o));
          _setBlockValid(UNPACK(p - o));
          for(const auto & m : moderators_touched)
          {
            _setBlockActive(UNPACK(m));
          }
        }
        break;
      }
      else if(b == BlockType::reflector)
      {
        if(i == 0 || i > NEUTRON_REACH / 2)
        {
          break;
        }
        _setBlockValid(UNPACK(n + o));
        for(const auto & m : moderators_touched)
        {
          _setBlockActive(UNPACK(m));
        }
        break;
      }
      else
      {
        break;
      }
    }
  }
}

void Reactor::_fuelCellFilterAdjacency(index_t x, index_t y, index_t z)
{
  if(blockTypeAt(x, y, z) != BlockType::reactorCell)
  {
    return;
  }
  for(auto i = _cellAdjacencyCache[_XYZ(x, y, z)].begin();
      i != _cellAdjacencyCache[_XYZ(x, y, z)].end(); )
  {
    if(_blocks[*i] == BlockType::reactorCell && (!_cellActiveCache[_XYZ(x,y,z)] || !_cellActiveCache[*i]))
    {
      i = _cellAdjacencyCache[_XYZ(x, y, z)].erase(i);
    }
    else
    {
      i++;
    }
  }
  return;
}

void Reactor::_reflectorUpdate(index_t x, index_t y, index_t z)
{
  if(blockTypeAt(x,y,z) != BlockType::reflector)
  {
    return;
  }
  for(auto && i : _cellAdjacencyCache[_XYZ(x, y, z)])
  {
    if(_blocks[i] == BlockType::reactorCell && _cellValidCache[i])
    {
      _cellActiveCache[_XYZ(x,y,z)] = 1;
    }
  }
}

void Reactor::_floodFillConductor(index_t x, index_t y, index_t z, largeindex_t conductorID)
{
  if(blockTypeAt(x, y, z) != BlockType::conductor)
  {
    return;
  }
  if(_conductorIdCache[_XYZ(x, y, z)] != -1)
  {
    return;
  }

  coord_t p(x,y,z);

  _conductorIdCache[_XYZ(x, y, z)] = conductorID;

  for(const coord_t & o : offsets)
  {
    if(blockTypeAt(UNPACK(p + o)) == BlockType::casing)
    {
      _validConductorIds.insert(conductorID);
    }
    if(blockTypeAt(UNPACK(p + o)) == BlockType::conductor && _conductorIdCache[_XYZ(x, y, z)] == -1)
    {
      _floodFillConductor(UNPACK(p + o), conductorID);
    }
  }
}

void Reactor::_floodFillCluster(index_t x, index_t y, index_t z, largeindex_t clusterID)
{
  if(clusterIdAt(x, y, z) != -1)
  {
    return;
  }
  if(blockTypeAt(x, y, z) != BlockType::reactorCell
  && blockTypeAt(x, y, z) != BlockType::cooler)
  {
    return;
  }

  if(blockTypeAt(x, y, z) == BlockType::reactorCell
  && !blockValidAt(x, y, z))
  {
    return;
  }

  if(blockTypeAt(x, y, z) == BlockType::cooler
  && !coolerActiveAt(x, y, z))
  {
    return;
  }

  coord_t p(x,y,z);

  _clusterIdCache[_XYZV(p)] = clusterID;

  for(const coord_t & o : offsets)
  {
    if(blockTypeAt(UNPACK(p + o)) == BlockType::casing
    || (blockTypeAt(UNPACK(p + o)) == BlockType::conductor && blockActiveAt(UNPACK(p + o))))
    {
      _validClusterIds.insert(clusterID);
    }
    if(clusterIdAt(UNPACK(p + o)) == -1)
    {
      _floodFillCluster(UNPACK(p + o), clusterID);
    }
  }
}

bool Reactor::_hasLineOfSightToOutside(index_t x, index_t y, index_t z)
{
  coord_t n(x, y, z);
  for(const coord_t & o : offsets)
  {
    coord_t p = n;

    do {
      p += o;
      if(blockTypeAt(UNPACK(p)) == BlockType::casing)
      {
        return true;
      }
    } while (blockTypeAt(UNPACK(p)) != BlockType::reactorCell && blockTypeAt(UNPACK(p)) != BlockType::reflector);
  }
  return false;
}

float Reactor::_fuelCellEfficiencyAt(index_t x, index_t y, index_t z, FuelType ft)
{
  return _cellPositionalEfficiency[_XYZ(x,y,z)] * fuel_eff[static_cast<int>(ft)] * (1. / (1. + expf(2. * (_cellModeratorFlux[_XYZ(x,y,z)] - 2. * fuel_crit[static_cast<int>(ft)]))));
}

float Reactor::averageEfficiencyForFuel(FuelType ft)
{
  if(!_reactorCellCache.size())
  {
    return 0;
  }
  float ret = 0;
  for(const coord_t & c : _reactorCellCache)
  {
    ret += _fuelCellEfficiencyAt(UNPACK(c), ft);
  }
  return ret / _reactorCellCache.size();
}

void Reactor::_evaluate(FuelType ft) {
  if (_dirty) {

    _revertToSetup();

    _dirty = false;

    // compute conductor groups
    largeindex_t nextConductorId = 0;
    for(const coord_t & c : _conductorCache)
    {
      if(_conductorIdCache[_XYZV(c)] == -1)
      {
        _largestAssignedConductorId = std::max(_largestAssignedConductorId, nextConductorId);
        _floodFillConductor(UNPACK(c), nextConductorId++);
      }
    }

    for(const coord_t & c : _conductorCache)
    {
      if(_validConductorIds.count(_conductorIdCache[_XYZV(c)]))
      {
        _setBlockActive(UNPACK(c));
        _setBlockValid(UNPACK(c));
      }
    }

    // run fuel cell activations
    for(const coord_t & c : _primedCellCache)
    {
      _fuelCellBroadcastFlux(UNPACK(c), ft);
    }

    for(const coord_t & c : _reactorCellCache)
    {
      _fuelCellBroadcastModeratorActivations(UNPACK(c));
    }

    for(const coord_t & c : _reactorCellCache)
    {
      _fuelCellFilterAdjacency(UNPACK(c));
    }

    for(const coord_t & c : _reflectorCache)
    {
      _reflectorUpdate(UNPACK(c));
    }

    // make clusters
    largeindex_t nextClusterId = 0;

    for(const coord_t & c : _reactorCellCache)
    {
      if(clusterIdAt(UNPACK(c)) == -1 && blockValidAt(UNPACK(c)))
      {
        _largestAssignedClusterId = std::max(_largestAssignedClusterId, nextClusterId);
        _floodFillCluster(UNPACK(c), nextClusterId++);
      }
    }

    // calculate stuff per reactor cell

    for(const coord_t & c : _reactorCellCache)
    {
      largeindex_t cluster = clusterIdAt(UNPACK(c));
      _heatingPerCluster[cluster] += _cellAdjacencyCache[_XYZV(c)].size() * fuel_heat[static_cast<int>(ft)]; // heatmultiplier * fuelbaseheat
      _outputPerCluster[cluster] += _fuelCellEfficiencyAt(UNPACK(c), ft) * fuel_heat[static_cast<int>(ft)];
      _perClusterSumCellEff[cluster] += _fuelCellEfficiencyAt(UNPACK(c), ft);
      _perClusterSumHeatMult[cluster] += _cellAdjacencyCache[_XYZV(c)].size(); // heatmultiplier
      _perClusterCellCount[cluster] += 1;
    }

    // calculate stuff per cooler

    for(const coord_t & c : _coolerCache)
    {
      largeindex_t cluster = clusterIdAt(UNPACK(c));
      CoolerType ct = coolerTypeAt(UNPACK(c));
      _coolingPerCluster[cluster] += coolerStrengths[ct];
    }

    // calculate penalties per cluster
    for(largeindex_t i = 0; i <= _largestAssignedClusterId; i++)
    {
      float rawEfficiency = _perClusterSumCellEff[i] / _perClusterCellCount[i];
      float coolingPenaltyMultiplier = 0.0;
      if(_heatingPerCluster[i] > 0 && _coolingPerCluster[i] > 0)
      {
        coolingPenaltyMultiplier = std::min((float)1.0, _heatingPerCluster[i] / _coolingPerCluster[i]);
      }

      _perClusterEff[i] = rawEfficiency * coolingPenaltyMultiplier;
      _outputPerCluster[i] *= coolingPenaltyMultiplier;
    }

    // calculate overall stats
    float _totalPowerOutput = 0.;
    float _worstCaseDutyCycle = 1.0;
    for(largeindex_t i = 0; i <= _largestAssignedClusterId; i++)
    {
      _totalPowerOutput += _outputPerCluster[i];

      if(!_validClusterIds.count(i) && _heatingPerCluster[i] > 0)
      {
        _worstCaseDutyCycle = 0;
      }
      else if(_heatingPerCluster[i] > _coolingPerCluster[i]) {
        _worstCaseDutyCycle = std::min(_worstCaseDutyCycle, _coolingPerCluster[i] / _heatingPerCluster[i]);
      }
    }
    _powerGeneratedCache[ft] = _totalPowerOutput;
    _dutyCycleCache[ft] = _worstCaseDutyCycle;

    // summarize inactive blocks
    _inactiveBlocks = 0;
    for(const coord_t & i : _reactorCellCache)
    {
      if(!blockActiveAt(UNPACK(i)) && !blockValidAt(UNPACK(i)))
      {
        ++_inactiveBlocks;
      }
    }
    for(const coord_t & i : _moderatorCache)
    {
      if(!blockActiveAt(UNPACK(i)) && !blockValidAt(UNPACK(i)))
      {
        ++_inactiveBlocks;
      }
    }
    for(const coord_t & i : _coolerCache)
    {
      if(!blockActiveAt(UNPACK(i)) && !blockValidAt(UNPACK(i)))
      {
        ++_inactiveBlocks;
      }
    }
    for(const coord_t & i : _reflectorCache)
    {
      if(!blockActiveAt(UNPACK(i)) && !blockValidAt(UNPACK(i)))
      {
        ++_inactiveBlocks;
      }
    }
  }
}

std::set<coord_t> Reactor::suggestPrincipledLocations()
{
  std::set<coord_t> ret;

  // cells collinear with existing reactor cells
  for (const auto & c : _reactorCellCache)
  {
    for (const auto & o : offsets)
    {
      coord_t n = c;
      while (isInBounds(UNPACK(n)))
      {
        n += o;
        ret.insert(n);
      }
    }
  }

  // cells that are, or are adjacent to existing coolers
  for (const auto & c : _coolerCache)
  {
    ret.insert(c);
    for (const auto & o : offsets)
    {
      ret.insert(c + o);
    }
  }

  // cells adjacent to moderators that can support heatsinks
  for (const auto & c : _moderatorCache)
  {
    if (_cellValidCache[_XYZV(c)])
    {
      for (const auto & o : offsets)
      {
        ret.insert(c + o);
      }
    }
  }

  return ret;
}

std::set<std::tuple<BlockType, CoolerType, ModeratorType, float> >
Reactor::suggestedBlocksAt(index_t x, index_t y, index_t z, PrincipledSearchMode m)
{
  std::set<std::tuple<BlockType, CoolerType, ModeratorType, float> > ret;

  coord_t c(x, y, z);

  if(blockTypeAt(x, y, z) == BlockType::reactorCell || blockTypeAt(x, y, z) == BlockType::reflector)
  {
    return ret;
  }

  // collinear with an existing reactor cell?
  // for (const auto & o : offsets)
  // {
  //   coord_t n = c;
  //   int i = 0;
  //   while (isInBounds(UNPACK(n)) && i <= NEUTRON_REACH + 1)
  //   {
  //     n += o;
  //     i++;
  //     if(blockTypeAt(UNPACK(n)) == BlockType::reactorCell)
  //     {
  //       // reactor cells are valid if non adjacent in this direction
  //       if (i > 1)
  //         ret.insert(std::make_tuple(BlockType::reactorCell, CoolerType::air, ModeratorType::air, 1));
  //       // moderators are valid if within neutron reach
  //       if (i <= NEUTRON_REACH)
  //       {
  //         ret.insert(std::make_tuple(BlockType::reactorCell, CoolerType::air, ModeratorType::beryllium, 1));
  //         ret.insert(std::make_tuple(BlockType::reactorCell, CoolerType::air, ModeratorType::graphite, 1));
  //         ret.insert(std::make_tuple(BlockType::reactorCell, CoolerType::air, ModeratorType::heavyWater, 1));
  //       }
  //     }
  //   }
  // }

  if (m == PrincipledSearchMode::optimizeModerators)
  {
    // is this a moderator?
    if (blockTypeAt(x, y, z) == BlockType::moderator)
    {
      if(moderatorTypeAt(x, y, z) != ModeratorType::graphite)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::graphite, 0.2));
      if(moderatorTypeAt(x, y, z) != ModeratorType::beryllium)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::beryllium, 0.2));
      if(moderatorTypeAt(x, y, z) != ModeratorType::heavyWater)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::heavyWater, 0.2));
    }
  }
  if (m == PrincipledSearchMode::computeCooling)
  {
    if (blockTypeAt(x, y, z) == BlockType::moderator)
    {
      return ret;
    }
    // would a cooler be active if placed here?
    for (int cti = 1; cti < static_cast<int>(CoolerType::COOLER_TYPE_MAX); cti++)
    {
      CoolerType ct = static_cast<CoolerType>(cti);
      if(coolerTypeActiveAt(x, y, z, ct) && coolerTypeAt(x, y, z) != ct)
      {
        // am I adjacent to an undercooled cluster
        float dh_pct = 0;
        coord_t p(x, y, z);
        for(const auto & o : offsets)
        {
          largeindex_t cid = clusterIdAt(UNPACK(p+o));
          dh_pct = std::max(dh_pct, std::min(_heatingPerCluster[cid] / std::max(_coolingPerCluster[cid], (float)1.0), (float)2.0));
        }
        ret.insert(std::make_tuple(BlockType::cooler, ct, ModeratorType::air, 1 + dh_pct));
      }
    }

    // adjacent to an active reactor cell or cooler or conductor or casing?
    if (activeCoolersAdjacentTo(x, y, z) || reactorCellsAdjacentTo(x, y, z)
    || _blockTypeAdjacentTo(x, y, z, BlockType::conductor)
    || _blockTypeAdjacentTo(x, y, z, BlockType::casing))
    {
      ret.insert(std::make_tuple(BlockType::conductor, CoolerType::air, ModeratorType::air, 1));
    }

    // is this a cooler in an overcooled cluster?
    if (_heatingPerCluster[clusterIdAt(x, y, z)] < _coolingPerCluster[clusterIdAt(x, y, z)])
    {
      ret.insert(std::make_tuple(BlockType::air, CoolerType::air, ModeratorType::air, _coolingPerCluster[clusterIdAt(x, y, z)] / std::max(_heatingPerCluster[clusterIdAt(x, y, z)], (float)1)));
    }
  }

  return ret;
}

void Reactor::pruneInactives(bool ignoreConductors) {
  for(int x = 0; x < _x; x++)
  {
    for(int y = 0; y < _y; y++)
    {
      for(int z = 0; z < _z; z++)
      {
        if(blockTypeAt(x,y,z) != BlockType::air && blockTypeAt(x,y,z) != BlockType::conductor && !blockActiveAt(x, y, z) && !blockValidAt(x, y, z) && !_primedStatus[_XYZ(x,y,z)] && !_fluxedModeratorCache[_XYZ(x,y,z)])
        {
          setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, false);
        }
        if(!ignoreConductors && blockTypeAt(x,y,z) == BlockType::conductor && _conductorIdCache[_XYZ(x,y,z)] == -1)
        {
          setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, false);
        }
        if(blockTypeAt(x, y, z) == BlockType::reactorCell && _primedStatus[_XYZ(x,y,z)] && !blockActiveAt(x, y, z))
        {
          bool prune = true;
          coord_t n(x, y, z);
          for(const coord_t & o : offsets)
          {
            if(!prune)
            {
              break;
            }
            coord_t p = n + o;
            for(int i = 1; i <= NEUTRON_REACH + 1; i++)
            {
              BlockType b = blockTypeAt(UNPACK(p));
              if(i == 1 && b == BlockType::reactorCell)
              {
                break;
              }
              if(i >= 2 && b == BlockType::reactorCell && blockActiveAt(UNPACK(p)))
              {
                prune = false;
                break;
              }
              if(b != BlockType::reactorCell && b != BlockType::moderator)
              {
                break;
              }
              p += o;
            }
          }
          if(prune)
            setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, false);
        }
      }
    }
  }
}

void Reactor::clearInfeasibleClusters()
{
  for(int i = 0; i < _largestAssignedClusterId; i++)
  {
    if(!_validClusterIds.count(i) || _coolingPerCluster[i] == 0)
    {
      for (int z = 0; z < _z; z++) {
        for (int y = 0; y < _y; y++) {
          for (int x = 0; x < _x; x++) {
            if(clusterIdAt(x, y, z) == i)
            {
              setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, false);
            }
          }
        }
      }
    }
  }
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
        else if(bt == BlockType::moderator)
        {
          r += moderatorTypeShort[static_cast<int>(moderatorTypeAt(x,y,z))];
        }
        else if(bt == BlockType::reactorCell && _primedStatus[_XYZ(x,y,z)] == 1)
        {
          r += '+';
          r += blockTypeShort[static_cast<int>(bt)];
        }
        else
        {
          r += blockTypeShort[static_cast<int>(bt)];
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

std::string Reactor::clusterStats() {
  std::string r;
  for(largeindex_t i = 0; i <= _largestAssignedClusterId; i++)
  {
    r += "cluster ";
    r += std::to_string(i);
    r += ": ";
    r += (_validClusterIds.count(i) ? " valid " : "invalid");
    r += ", power ";
    r += std::to_string(_outputPerCluster[i]);
    r += ", heating ";
    r += std::to_string(_heatingPerCluster[i]);
    r += ", cooling ";
    r += std::to_string(_coolingPerCluster[i]);
    r += "\n";
  }
  return r;
}

std::string Reactor::jsonExport(FuelType f) {
  _evaluate(f);

  std::string r = "{\"SaveVersion\":{\"Major\":2,\"Minor\":0,\"Build\":18,\"Revision\":0,\"MajorRevision\":0,\"MinorRevision\":0},";

  r += "\"HeatSinks\":{";

  std::map<CoolerType, std::vector<coord_t> > coolersByType;
  for(const coord_t & c : _coolerCache)
  {
    coolersByType[coolerTypeAt(UNPACK(c))].push_back(c);
  }

  for(const auto & c : coolersByType)
  {
    r += "\"";
    r += coolerTypeLong[static_cast<int>(c.first)];
    r += "\":[";

    for(const auto & coord : c.second)
    {
      r += "\"";
      r += std::to_string(coord.x()+1);
      r += ",";
      r += std::to_string(coord.y()+1);
      r += ",";
      r += std::to_string(coord.z()+1);
      r += "\",\n";
    }

    if(c.second.size())
    {
      r.pop_back();
      r.pop_back();
    }

    r += "],\n";
  }
  if(coolersByType.size())
  {
    r.pop_back();
    r.pop_back();
  }
  r += "},\"Moderators\":{";

  std::map<ModeratorType, std::vector<coord_t> > moderatorsByType;
  for(const coord_t & c : _moderatorCache)
  {
    moderatorsByType[moderatorTypeAt(UNPACK(c))].push_back(c);
  }

  for(const auto & c : moderatorsByType)
  {
    r += "\"";
    r += moderatorTypeLong[static_cast<int>(c.first)];
    r += "\":[";

    for(const auto & coord : c.second)
    {
      r += "\"";
      r += std::to_string(coord.x()+1);
      r += ",";
      r += std::to_string(coord.y()+1);
      r += ",";
      r += std::to_string(coord.z()+1);
      r += "\",\n";
    }

    if(c.second.size())
    {
      r.pop_back();
      r.pop_back();
    }

    r += "],";
  }
  if(moderatorsByType.size())
  {
    r.pop_back();
  }
  r += "},\"Conductors\":[";
  for(const auto & coord : _conductorCache)
  {
    r += "\"";
    r += std::to_string(coord.x()+1);
    r += ",";
    r += std::to_string(coord.y()+1);
    r += ",";
    r += std::to_string(coord.z()+1);
    r += "\",\n";
  }
  if(_conductorCache.size())
  {
    r.pop_back();
    r.pop_back();
  }
  r += "],\"Reflectors\":[";
  for(const auto & coord : _reflectorCache)
  {
    r += "\"";
    r += std::to_string(coord.x()+1);
    r += ",";
    r += std::to_string(coord.y()+1);
    r += ",";
    r += std::to_string(coord.z()+1);
    r += "\",\n";
  }
  if(_reflectorCache.size())
  {
    r.pop_back();
    r.pop_back();
  }
  r += "],\"FuelCells\":{\"";
  r += fuel_names[static_cast<int>(f)];
  r += ";True\":[";
  for(const auto & coord : _reactorCellCache)
  {
    r += "\"";
    r += std::to_string(coord.x()+1);
    r += ",";
    r += std::to_string(coord.y()+1);
    r += ",";
    r += std::to_string(coord.z()+1);
    r += "\",\n";
  }
  if(_reactorCellCache.size())
  {
    r.pop_back();
    r.pop_back();
  }
  r += "]},\"InteriorDimensions\":\"";
  r += std::to_string(_x);
  r += ",";
  r += std::to_string(_y);
  r += ",";
  r += std::to_string(_z);
  r += "\"}";

  return r;

}
