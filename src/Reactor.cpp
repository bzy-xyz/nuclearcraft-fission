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
  {CoolerType::water, 55},
  {CoolerType::iron, 50},
  {CoolerType::redstone, 85},
  {CoolerType::quartz, 75},
  {CoolerType::obsidian, 70},
  {CoolerType::netherbrick, 105},
  {CoolerType::glowstone, 100},
  {CoolerType::lapis, 95},
  {CoolerType::gold, 110},
  {CoolerType::prismarine, 115},
  {CoolerType::slime, 135},
  {CoolerType::endstone, 65},
  {CoolerType::purpur, 90},
  {CoolerType::diamond, 195},
  {CoolerType::emerald, 190},
  {CoolerType::copper, 80},
  {CoolerType::tin, 120},
  {CoolerType::lead, 60},
  {CoolerType::boron, 165},
  {CoolerType::lithium, 130},
  {CoolerType::magnesium, 125},
  {CoolerType::manganese, 145},
  {CoolerType::aluminum, 185},
  {CoolerType::silver, 170},
  {CoolerType::fluorite, 175},
  {CoolerType::villiaumite, 160},
  {CoolerType::carobbite, 150},
  {CoolerType::arsenic, 140},
  {CoolerType::nitrogen, 180},
  {CoolerType::helium, 200},
  {CoolerType::enderium, 155},
  {CoolerType::cryotheum, 205},
};

const static float fuel_eff_vanilla[] = {
  0, 1,
  1,  1,  1.05,
  1.1, 1.1, 1.15,
  1.1, 1.1, 1.15,
  1, 1, 1.05,
  1, 1, 1.05,
  1.1, 1.1, 1.15,
  1.1, 1.1, 1.15,
  1.2, 1.2, 1.25,
  1.2, 1.2, 1.25,
  1.25, 1.25, 1.3,
  1.25, 1.25, 1.3,
  1.05, 1.05, 1.1,
  1.15, 1.15, 1.2,
  1.35, 1.35, 1.4,
  1.35, 1.35, 1.4,
  1.45, 1.45, 1.5,
  1.45, 1.45, 1.5,
  1.5, 1.5, 1.55,
  1.5, 1.5, 1.55,
  1.55, 1.55, 1.6,
  1.55, 1.55, 1.6,
  1.65, 1.65, 1.7,
  1.65, 1.65, 1.7,
  1.75, 1.75, 1.8,
  1.75, 1.75, 1.8,
  1.8, 1.8, 1.85,
  1.8, 1.8, 1.85,
  0
};

const static float fuel_heat_vanilla[] = {
  0, 1,
  40,	32,	50,
  216,	172,	270,
  648,	516,	810,
  120,	96,	150,
  360,	288,	450,
  292,	234,	366,
  876,	702,	1098,
  126,	100,	158,
  378,	300,	474,
  182,	146,	228,
  546,	438,	684,
  132,	106,	166,
  192,	154,	240,
  390,	312,	488,
  1170,	936,	1464,
  384,	308,	480,
  1152,	924,	1440,
  238,	190,	298,
  714,	570,	894,
  268,	214,	336,
  804,	642,	1008,
  266,	212,	332,
  798,	636,	996,
  540,	432,	676,
  1620,	1296,	2028,
  288,	230,	360,
  864,	690,	1080,
  0
};

const static int fuel_crit_vanilla[] = {
  0, 1,
  234,	293,	199,
  78,	98,	66,
  39,	49,	33,
  102,	128,	87,
  51,	64,	43,
  70,	88,	60,
  35,	44,	30,
  99,	124,	84,
  49,	62,	42,
  84,	105,	71,
  42,	52,	35,
  94,	118,	80,
  80,	100,	68,
  65,	81,	55,
  32,	40,	27,
  66,	83,	56,
  33,	41,	28,
  75,	94,	64,
  37,	47,	32,
  72,	90,	61,
  36,	45,	30,
  73,	91,	62,
  36, 45,	31,
  60,	75,	51,
  30,	37,	25,
  71,	89,	60,
  35,	44,	30,
  0
};

const static int moderator_flux_vanilla[] = {
  0, 22, 10, 36, 0
};

const static float moderator_eff_vanilla[] = {
  0, 1.05, 1.1, 1.0, 0
};

const static float neutron_source_eff_vanilla[] = {
  0, 0.9, 0.95, 1.0, 0
};

const static float reflector_refl_vanilla[] = {
  0, 1.0, 0.5, 0
};

const static float reflector_eff_vanilla[] = {
  0, 0.5, 0.25, 0
};

static std::map<CoolerType, float> coolerStrengths = coolerStrengths_vanilla;
const static float * fuel_eff = fuel_eff_vanilla;
const static float * fuel_heat = fuel_heat_vanilla;
const static int * fuel_crit = fuel_crit_vanilla;
const static int * moderator_flux = moderator_flux_vanilla;
const static float * moderator_eff = moderator_eff_vanilla;
const static float * neutron_source_eff = neutron_source_eff_vanilla;
const static float * reflector_refl = reflector_refl_vanilla;
const static float * reflector_eff = reflector_eff_vanilla;

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
  "NB",
  "GL",
  "LA",
  "AU",
  "PR",
  "SL",
  "ES",
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
  "FL",
  "VI",
  "CA",
  "AS",
  "LN",
  "HE",
  "EN",
  "CR",
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
  "NetherBrick",
  "Glowstone",
  "Lapis",
  "Gold",
  "Prismarine",
  "Slime",
  "EndStone",
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
  "Fluorite",
  "Villiaumite",
  "Carobbiite",
  "Arsenic",
  "Nitrogen",
  "Helium",
  "Enderium",
  "Cryotheum",
};

static const char * moderatorTypeLong[] = {
  "Air",
  "Beryllium",
  "Graphite",
  "HeavyWater"
};

static const char * neutronSourceTypeShort[] = {
  "_",
  "R",
  "P",
  "C",
};

static const char * neutronSourceTypeLong[] = {
  "None",
  "Ra-Be",
  "Po-Be",
  "Cf-252",
};

static const char * reflectorTypeShort[] = {
  "__",
  "BC",
  "LS",
};

static const char * reflectorTypeLong[] = {
  "Air",
  "Beryllium-Carbon",
  "Lead-Steel",
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
  _primedStatus = std::vector<NeutronSourceType>(x * y * z, NeutronSourceType::unprimed);
  _reflectorTypes = std::vector<ReflectorType>(x * y * z, ReflectorType::air);

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
      return validModeratorsAdjacentTo(x, y, z) == 1 && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::boron:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::quartz) == 1 && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::prismarine:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water) >= 2;
    case CoolerType::obsidian:
      return hasAxialCoolers(x, y, z, CoolerType::glowstone);
    case CoolerType::lead:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::iron);
    case CoolerType::aluminum:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::quartz) && activeCoolersAdjacentTo(x, y, z, CoolerType::lapis);
    case CoolerType::lithium:
      return hasAxialCoolers(x, y, z, CoolerType::lead) && reactorCasingsAdjacentTo(x, y, z);
    case CoolerType::manganese:
      return reactorCellsAdjacentTo(x, y, z) >= 2;
    case CoolerType::silver:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::glowstone) >= 2 && activeCoolersAdjacentTo(x, y, z, CoolerType::tin);
    case CoolerType::purpur:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::iron) == 1 && activeCoolersAdjacentTo(x, y, z, CoolerType::endstone);
    case CoolerType::arsenic:
      return hasAxialReflectors(x, y, z);
    case CoolerType::carobbite:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::copper) && activeCoolersAdjacentTo(x, y, z, CoolerType::endstone);
    case CoolerType::villiaumite:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::endstone) && activeCoolersAdjacentTo(x, y, z, CoolerType::redstone);
    case CoolerType::slime:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::water) == 1 && activeCoolersAdjacentTo(x, y, z, CoolerType::lead) >= 2;
    case CoolerType::fluorite:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::gold) && activeCoolersAdjacentTo(x, y, z, CoolerType::prismarine);
    case CoolerType::netherbrick:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::obsidian);
    case CoolerType::endstone:
      return activeReflectorsAdjacentTo(x, y, z);
    case CoolerType::nitrogen:
      return activeCoolersAdjacentTo(x, y, z, CoolerType::copper) >= 2 && activeCoolersAdjacentTo(x, y, z, CoolerType::purpur) >= 1;
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
  _cellModeratorFlux = std::vector<int>(_x * _y * _z, 0);
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
        if(_primedStatus[_XYZV(v)] != NeutronSourceType::unprimed && _hasLineOfSightToOutside(UNPACK(v))) {
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
    int sum_moderator_flux = 0;
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
        _cellModeratorFlux[_XYZV(n)] += 2 * sum_moderator_flux * reflector_refl[static_cast<int>(_reflectorTypes[_XYZV(n)])];
        _cellPositionalEfficiency[_XYZV(n)] += reflector_eff[static_cast<int>(_reflectorTypes[_XYZV(n)])] * sum_moderator_eff /  moderators_in_line;

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
  float eff = _cellPositionalEfficiency[_XYZ(x,y,z)] * fuel_eff[static_cast<int>(ft)] * (1. / (1. + expf(2. * (_cellModeratorFlux[_XYZ(x,y,z)] - 2. * fuel_crit[static_cast<int>(ft)]))));
  if (_primedStatus[_XYZ(x,y,z)] != NeutronSourceType::unprimed)
  {
    eff *= neutron_source_eff[static_cast<int>(_primedStatus[_XYZ(x,y,z)])];
  }
  return eff;
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
        coolingPenaltyMultiplier = std::min((float)1.0, (_heatingPerCluster[i] + COOLING_LENIENCY) / _coolingPerCluster[i]);
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

    _totalPowerOutput *= emptyReactorInefficiencyFactor();

    _powerGeneratedCache[ft] = _totalPowerOutput;
    _dutyCycleCache[ft] = _worstCaseDutyCycle;
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

std::set<std::tuple<BlockType, CoolerType, ModeratorType, NeutronSourceType, ReflectorType, float> >
Reactor::suggestedBlocksAt(index_t x, index_t y, index_t z, PrincipledSearchMode m)
{
  std::set<std::tuple<BlockType, CoolerType, ModeratorType, NeutronSourceType, ReflectorType, float> > ret;

  coord_t c(x, y, z);

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

  if (m == PrincipledSearchMode::optimizeModerators || m == PrincipledSearchMode::hybrid)
  {
    // is this a moderator?
    if (blockTypeAt(x, y, z) == BlockType::moderator)
    {
      if(moderatorTypeAt(x, y, z) != ModeratorType::graphite)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::graphite, NeutronSourceType::unprimed, ReflectorType::air, 0.2));
      if(moderatorTypeAt(x, y, z) != ModeratorType::beryllium)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::beryllium, NeutronSourceType::unprimed, ReflectorType::air, 0.2));
      if(moderatorTypeAt(x, y, z) != ModeratorType::heavyWater)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::heavyWater, NeutronSourceType::unprimed, ReflectorType::air, 0.2));
    }
    // is this a reactorcell?
    if (blockTypeAt(x, y, z) == BlockType::reactorCell)
    {
      if(neutronSourceTypeAt(x, y, z) != NeutronSourceType::ra_be && _hasLineOfSightToOutside(x, y, z))
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::air, NeutronSourceType::ra_be, ReflectorType::air, 0.2));
      if(neutronSourceTypeAt(x, y, z) != NeutronSourceType::po_be && _hasLineOfSightToOutside(x, y, z))
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::air, NeutronSourceType::po_be, ReflectorType::air, 0.2));
      if(neutronSourceTypeAt(x, y, z) != NeutronSourceType::cf_252 && _hasLineOfSightToOutside(x, y, z))
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::air, NeutronSourceType::cf_252, ReflectorType::air, 0.2));
    }
    // is this a reflector?
    if (blockTypeAt(x, y, z) == BlockType::reflector)
    {
      if(reflectorTypeAt(x, y, z) != ReflectorType::beryllium_carbon)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::beryllium_carbon, 0.2));
      if(reflectorTypeAt(x, y, z) != ReflectorType::lead_steel)
        ret.insert(std::make_tuple(BlockType::moderator, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::lead_steel, 0.2));
    }
  }
  if (m == PrincipledSearchMode::computeCooling || m == PrincipledSearchMode::hybrid)
  {
    if (blockTypeAt(x, y, z) == BlockType::moderator || blockTypeAt(x, y, z) == BlockType::reactorCell || blockTypeAt(x, y, z) == BlockType::reflector)
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
        ret.insert(std::make_tuple(BlockType::cooler, ct, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air, 1 + dh_pct));
      }
    }

    // adjacent to an active reactor cell or cooler or conductor or casing?
    if (activeCoolersAdjacentTo(x, y, z) || reactorCellsAdjacentTo(x, y, z)
    || _blockTypeAdjacentTo(x, y, z, BlockType::conductor)
    || _blockTypeAdjacentTo(x, y, z, BlockType::casing))
    {
      ret.insert(std::make_tuple(BlockType::conductor, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air, 1));
    }

    // is this a cooler in an overcooled cluster?
    if (_heatingPerCluster[clusterIdAt(x, y, z)] < _coolingPerCluster[clusterIdAt(x, y, z)])
    {
      ret.insert(std::make_tuple(BlockType::air, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air, _coolingPerCluster[clusterIdAt(x, y, z)] / std::max(_heatingPerCluster[clusterIdAt(x, y, z)], (float)1)));
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
        if(blockTypeAt(x,y,z) != BlockType::air && blockTypeAt(x,y,z) != BlockType::conductor && !blockActiveAt(x, y, z) && !blockValidAt(x, y, z) && _primedStatus[_XYZ(x,y,z)] == NeutronSourceType::unprimed && !_fluxedModeratorCache[_XYZ(x,y,z)])
        {
          setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air);
        }
        if(!ignoreConductors && blockTypeAt(x,y,z) == BlockType::conductor && _conductorIdCache[_XYZ(x,y,z)] == -1)
        {
          setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air);
        }
        if(blockTypeAt(x, y, z) == BlockType::reactorCell && _primedStatus[_XYZ(x,y,z)] != NeutronSourceType::unprimed && !blockActiveAt(x, y, z))
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
            setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air);
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
              setCell(x, y, z, BlockType::air, CoolerType::air, ModeratorType::air, NeutronSourceType::unprimed, ReflectorType::air);
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
        else if(bt == BlockType::reactorCell && _primedStatus[_XYZ(x,y,z)] != NeutronSourceType::unprimed)
        {
          r += neutronSourceTypeShort[static_cast<int>(neutronSourceTypeAt(x,y,z))];
          r += blockTypeShort[static_cast<int>(bt)];
        }
        else if(bt == BlockType::reflector)
        {
          r += reflectorTypeShort[static_cast<int>(reflectorTypeAt(x,y,z))];
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

  std::string r = "{\"SaveVersion\":{\"Major\":2,\"Minor\":0,\"Build\":37,\"Revision\":0,\"MajorRevision\":0,\"MinorRevision\":0},";

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
      r += "{\"X\":";
      r += std::to_string(coord.x()+1);
      r += ",\"Y\":";
      r += std::to_string(coord.y()+1);
      r += ",\"Z\":";
      r += std::to_string(coord.z()+1);
      r += "},";
    }

    if(c.second.size())
    {
      r.pop_back();
    }

    r += "],";
  }
  if(coolersByType.size())
  {
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
    r += "{\"X\":";
    r += std::to_string(coord.x()+1);
    r += ",\"Y\":";
    r += std::to_string(coord.y()+1);
    r += ",\"Z\":";
    r += std::to_string(coord.z()+1);
    r += "},";
    }

    if(c.second.size())
    {
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
    r += "{\"X\":";
    r += std::to_string(coord.x()+1);
    r += ",\"Y\":";
    r += std::to_string(coord.y()+1);
    r += ",\"Z\":";
    r += std::to_string(coord.z()+1);
    r += "},";
  }
  if(_conductorCache.size())
  {
    r.pop_back();
  }
  r += "],\"Reflectors\":{";
  std::map<ReflectorType, std::vector<coord_t> > reflectorsByType;
  for(const coord_t & c : _reflectorCache)
  {
    reflectorsByType[reflectorTypeAt(UNPACK(c))].push_back(c);
  }

  for(const auto & c : reflectorsByType)
  {
    r += "\"";
    r += reflectorTypeLong[static_cast<int>(c.first)];
    r += "\":[";

    for(const auto & coord : c.second)
    {
      r += "{\"X\":";
      r += std::to_string(coord.x()+1);
      r += ",\"Y\":";
      r += std::to_string(coord.y()+1);
      r += ",\"Z\":";
      r += std::to_string(coord.z()+1);
      r += "},";
    }

    if(c.second.size())
    {
      r.pop_back();
    }

    r += "],";
  }
  if(reflectorsByType.size())
  {
    r.pop_back();
  }
  r += "},\"FuelCells\":{";

  std::map<NeutronSourceType, std::vector<coord_t> > cellPrimersByType;
  for(const coord_t & c : _reactorCellCache)
  {
    cellPrimersByType[_primedStatus[_XYZV(c)]].push_back(c);
  }

  for(const auto & c : cellPrimersByType)
  {
    r += "\"";
    r += fuel_names[static_cast<int>(f)];
    r += ";";
    r += c.first == NeutronSourceType::unprimed ? "False" : "True";
    r += ";";
    r += neutronSourceTypeLong[static_cast<int>(c.first)];
    r += "\":[";
    for(const auto & coord : c.second)
    {
      r += "{\"X\":";
      r += std::to_string(coord.x()+1);
      r += ",\"Y\":";
      r += std::to_string(coord.y()+1);
      r += ",\"Z\":";
      r += std::to_string(coord.z()+1);
      r += "},";
    }
    if(_reactorCellCache.size())
    {
      r.pop_back();
    }
    r += "],"; 
  }
  if(cellPrimersByType.size())
  {
    r.pop_back();
  }
  r += "},\"InteriorDimensions\":{\"X\":";
  r += std::to_string(_x);
  r += ",\"Y\":";
  r += std::to_string(_y);
  r += ",\"Z\":";
  r += std::to_string(_z);
  r += "}}";

  return r;

}
