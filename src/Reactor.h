#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <vector>
#include <set>
#include <array>

#include <Eigen/Dense>

enum struct BlockType {
  air = 0,
  reactorCell, // 1
  moderator, // 2
  cooler, // 3
  conductor,
  reflector,
  casing,
  BLOCK_TYPE_MAX
};

enum struct CoolerType {
  air = 0,
  water,
  iron,
  redstone,
  quartz,
  obsidian,
  glowstone,
  lapis,
  gold,
  prismarine,
  purpur,
  diamond,
  emerald,
  copper,
  tin,
  lead,
  boron,
  lithium,
  magnesium,
  manganese,
  aluminum,
  silver,
  helium,
  enderium,
  cryotheum,
  carobbite,
  fluorite,
  villiaumite,
  arsenic,
  tcalloy,
  endstone,
  slime,
  netherbrick,
  COOLER_TYPE_MAX
};

enum struct ModeratorType {
  air = 0,
  beryllium,
  graphite,
  heavyWater,
  MODERATOR_TYPE_MAX
};

#define CREATE_FUELS(__FUEL_NAME) __FUEL_NAME##_OX, __FUEL_NAME##_NI, __FUEL_NAME##_ZA

enum struct FuelType {
  air = 0,
  generic,
  CREATE_FUELS(TBU),
  CREATE_FUELS(LEU233),
  CREATE_FUELS(HEU233),
  CREATE_FUELS(LEU235),
  CREATE_FUELS(HEU235),
  CREATE_FUELS(LEN236),
  CREATE_FUELS(HEN236),
  CREATE_FUELS(LEP239),
  CREATE_FUELS(HEP239),
  CREATE_FUELS(LEP241),
  CREATE_FUELS(HEP241),
  MOX239, MNI239, MZA239,
  MOX241, MNI241, MZA241,
  CREATE_FUELS(LEA242),
  CREATE_FUELS(HEA242),
  CREATE_FUELS(LECm243),
  CREATE_FUELS(HECm243),
  CREATE_FUELS(LECm245),
  CREATE_FUELS(HECm245),
  CREATE_FUELS(LECm247),
  CREATE_FUELS(HECm247),
  CREATE_FUELS(LEB248),
  CREATE_FUELS(HEB248),
  CREATE_FUELS(LECf249),
  CREATE_FUELS(HECf249),
  CREATE_FUELS(LECf251),
  CREATE_FUELS(HECf251),
  FUEL_TYPE_MAX
};

enum struct PrincipledSearchMode {
  computeCooling = 0,
  optimizeModerators,
  PRINCIPLED_SEARCH_MODE_TYPE_MAX
};

#define index_t int_fast8_t
#define vector_offset_t int_fast32_t
#define smallcount_t int_fast8_t
#define largecount_t int_fast32_t

#define largeindex_t int_fast32_t

typedef Eigen::Vector3i coord_t;
typedef Eigen::Array3i coord_a_t;

namespace std
{
  template <> struct less<coord_t> {
    bool operator()(const coord_t & a, const coord_t & b) const
    {
      return std::lexicographical_compare(
        a.data(),a.data()+a.size(),
        b.data(),b.data()+b.size());
    }
  };
}

#define NEUTRON_REACH 4
#define REFLECTOR_EFFICIENCY 0.5

#define UNPACK(vec) (vec).x(), (vec).y(), (vec).z()

#define _XYZV(vec) ((vec).x() * (_y * _z) + (vec).y() * (_z) + (vec).z())
#define _XYZ(__x, __y, __z) (__x * (_y * _z) + __y * (_z) + __z)

const std::vector<coord_t> offsets = {
  Eigen::Vector3i::UnitX(),
  -Eigen::Vector3i::UnitX(),
  Eigen::Vector3i::UnitY(),
  -Eigen::Vector3i::UnitY(),
  Eigen::Vector3i::UnitZ(),
  -Eigen::Vector3i::UnitZ(),
};

class Reactor {
public:
  Reactor(index_t x = 1, index_t y = 1, index_t z = 1);
  ~Reactor();

  /** Total power generated for fuel type.
    *
    */
  inline float powerGenerated(FuelType ft) {
    _evaluate(ft);
    return _powerGeneratedCache[ft];
  }

  /** Effective power generated for fuel type (modified by required reactor duty cycle).
   *
   */

  inline float effectivePowerGenerated(FuelType ft) {
    _evaluate(ft);
    return _powerGeneratedCache[ft] * _dutyCycleCache[ft];
  }

  inline float dutyCycle(FuelType ft) {
    _evaluate(ft);
    return _dutyCycleCache[ft];
  }

  inline largecount_t inactiveBlocks() {
    return _inactiveBlocks;
  }

  inline largecount_t numAirBlocks() {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::air);
  }

  inline largecount_t numModerators() {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::moderator);
  }

  inline largecount_t numCoolers() {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::cooler);
  }

  inline largecount_t numReflectors() {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::reflector);
  }

  inline largecount_t numConductors() {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::conductor);
  }

  inline largecount_t numEmptyBlocks() {
    return numAirBlocks() + numConductors() + inactiveBlocks();
  }

  inline largecount_t moderatorsAdjacentToReactorCells() {
    largecount_t s = 0;
    for(const auto & c : _reactorCellCache)
    {
      s += _blockTypeAdjacentTo(UNPACK(c), BlockType::moderator);
    }
    return s;
  }

  inline largecount_t moderatorsAdjacentToPrimedCells() {
    largecount_t s = 0;
    for(const auto & c : _reactorCellCache)
    {
      if(_primedStatus[_XYZV(c)])
        s += _blockTypeAdjacentTo(UNPACK(c), BlockType::moderator);
    }
    return s;
  }

  inline largecount_t fluxedModerators() {
    return _fluxedModerators;
  }

  inline largecount_t sandwichedModerators() {
    return _sandwichedModerators;
  }

  inline largecount_t numValidClusters() {
    return _validClusterIds.size();
  }

  inline largecount_t numPrimedCells() {
    return _primedCellCache.size();
  }

  inline largecount_t numTrappedCells() {
    largecount_t ret = 0;
    for(const auto & c : _reactorCellCache)
    {
      if(_blockTypeAdjacentTo(UNPACK(c), BlockType::reactorCell)
         + _blockTypeAdjacentTo(UNPACK(c), BlockType::moderator)
         + _blockTypeAdjacentTo(UNPACK(c), BlockType::reflector)
         + _blockTypeAdjacentTo(UNPACK(c), BlockType::casing) == 6)
         {
           ++ret;
         }
    }
    return ret;
  }

  float averageEfficiencyForFuel(FuelType ft);

  inline bool isSelfSustaining() {
    for(const auto & c : _primedCellCache)
    {
      if(!blockActiveAt(UNPACK(c)))
      {
        return false;
      }
    }
    return true;
  }

  inline bool isBalanced() {
    if(_largestAssignedClusterId < 0) return false;
    for(int i = 0; i <= _largestAssignedClusterId; i++)
    {
      float dh = _heatingPerCluster[i] - _coolingPerCluster[i];
      if(dh > 0 || dh < -10)
      {
        return false;
      }
    }
    return true;
  }

  inline float heatBalance() {
    if(_largestAssignedClusterId < 0) return 0;
    float ret = 0;
    for(int i = 0; i <= _largestAssignedClusterId; i++)
    {
      ret += _heatingPerCluster[i] - _coolingPerCluster[i];
    }
    return ret;
  }

  inline float totalCooling() {
    if(_largestAssignedClusterId < 0) return 0;
    float ret = 0;
    for(int i = 0; i <= _largestAssignedClusterId; i++)
    {
      ret += _coolingPerCluster[i];
    }
    return ret;
  }

  inline bool isInBounds(index_t x, index_t y, index_t z)
  {
    return !(x < 0 || y < 0 || z < 0 || x >= _x || y >= _y || z >= _z);
  }

  inline void setCell(index_t x, index_t y, index_t z, BlockType bt, CoolerType ct, ModeratorType mt, bool primed) {
    if (!isInBounds(x, y, z)) {
      return;
    }

    _dirty = true;
    _blocks[_XYZ(x, y, z)] = bt;
    _coolerTypes[_XYZ(x, y, z)] = bt == BlockType::cooler ? ct : CoolerType::air;
    _moderatorTypes[_XYZ(x, y, z)] = bt == BlockType::moderator ? mt : ModeratorType::air;
    _primedStatus[_XYZ(x, y, z)] = bt == BlockType::reactorCell ? primed : false;
  }


  inline BlockType blockTypeAt(index_t x, index_t y, index_t z) {
    if (!isInBounds(x, y, z)) {
      return BlockType::casing;
    }
    return _blocks[x * (_y * _z) + y * (_z) + z];
  }

  inline CoolerType coolerTypeAt(index_t x, index_t y, index_t z) {
    if (!isInBounds(x, y, z)) {
      return CoolerType::air;
    }
    return _coolerTypes[x * (_y * _z) + y * (_z) + z];
  }

  inline ModeratorType moderatorTypeAt(index_t x, index_t y, index_t z) {
    if (!isInBounds(x, y, z)) {
      return ModeratorType::air;
    }
    return _moderatorTypes[x * (_y * _z) + y * (_z) + z];
  }

  inline BlockType blockTypeAt(coord_t pos) {
    return blockTypeAt(UNPACK(pos));
  }

  inline CoolerType coolerTypeAt(coord_t pos) {
    return coolerTypeAt(UNPACK(pos));
  }

  inline bool blockValidAt(index_t x, index_t y, index_t z) {
    return isInBounds(x, y, z) && _cellValidCache[_XYZ(x, y, z)] == 1;
  }

  inline bool blockActiveAt(index_t x, index_t y, index_t z) {
    return isInBounds(x, y, z) && _cellActiveCache[_XYZ(x, y, z)] == 1;
  }

  bool coolerTypeActiveAt(index_t x, index_t y, index_t z, CoolerType ct = CoolerType::air);

  inline bool reactorCellActiveAt(index_t x, index_t y, index_t z) {
    return blockTypeAt(x,y,z) == BlockType::reactorCell && blockActiveAt(x,y,z);
  }


  inline bool coolerActiveAt(index_t x, index_t y, index_t z)
  {
    if(blockTypeAt(x,y,z) != BlockType::cooler) {
      return false;
    }
    if(_cellActiveCache[x * (_y * _z) + y * (_z) + z])
    {
      return _cellActiveCache[x * (_y * _z) + y * (_z) + z] == 1;
    }

    CoolerType ct = coolerTypeAt(x, y, z);
    bool r = coolerTypeActiveAt(x, y, z, ct);

    if(r) {
      _setBlockActive(x, y, z);
      _setBlockValid(x, y, z);
    }
    else {
      _unsetBlockActive(x, y, z);
      _unsetBlockValid(x, y, z);
    }

    return r;
  }

  inline bool moderatorActiveAt(index_t x, index_t y, index_t z) {
    return blockTypeAt(x,y,z) == BlockType::moderator && blockActiveAt(x,y,z);
  }

  inline bool moderatorValidAt(index_t x, index_t y, index_t z) {
    return blockTypeAt(x,y,z) == BlockType::moderator && blockValidAt(x,y,z);
  }

  inline bool reflectorActiveAt(index_t x, index_t y, index_t z) {
    return blockTypeAt(x,y,z) == BlockType::reflector && blockActiveAt(x,y,z);
  }

  smallcount_t reactorCellsAdjacentTo(index_t x, index_t y, index_t z);
  smallcount_t moderatorsAdjacentTo(index_t x, index_t y, index_t z);
  smallcount_t validModeratorsAdjacentTo(index_t x, index_t y, index_t z);
  inline smallcount_t reactorCasingsAdjacentTo(index_t x, index_t y, index_t z) {
    return _blockTypeAdjacentTo(x, y, z, BlockType::casing);
  }

  smallcount_t activeReflectorsAdjacentTo(index_t x, index_t y, index_t z);

  smallcount_t reactorCellsObservedByCell(index_t x, index_t y, index_t z)
  {
    if(blockTypeAt(x, y, z) != BlockType::reactorCell)
    {
      return 0;
    }
    return _reactorCellObservedCellCount[_XYZ(x, y, z)];
  }

  /** Number of coolers of a certain type adjacent to a cell.
   *
   * @note CoolerType::air := any cooler type
   */
  smallcount_t activeCoolersAdjacentTo(index_t x, index_t y, index_t z, CoolerType ct = CoolerType::air);

  bool hasAxialCoolers(index_t x, index_t y, index_t z, CoolerType ct);

  bool hasAxialReflectors(index_t x, index_t y, index_t z);

  largeindex_t clusterIdAt(index_t x, index_t y, index_t z)
  {
    if(!isInBounds(x, y, z))
    {
      return -2;
    }
    return _clusterIdCache[_XYZ(x,y,z)];
  }

  inline bool operator==(const Reactor &b) const {
    return  _x == b._x && _y == b._y && _z == b._z
        &&  _blocks == b._blocks && _coolerTypes == b._coolerTypes && _moderatorTypes == b._moderatorTypes;
  }

  inline bool operator<(const Reactor & b) const {
    return _blocks < b._blocks
        || (_blocks == b._blocks && _coolerTypes < b._coolerTypes)
        || (_blocks == b._blocks && _coolerTypes == b._coolerTypes && _moderatorTypes < b._moderatorTypes);
  }

  inline largecount_t totalCells() const {
    //return std::count(_blocks.begin(), _blocks.end(), BlockType::reactorCell);
    largecount_t r = 0;
    for(vector_offset_t i = 0; i < _x * _y * _z; i++)
    {
      if(_blocks[i] == BlockType::reactorCell && _cellActiveCache[i] == 1)
      {
        r += 1;
      }
    }
    return r;
  }

  std::string describe();
  std::string clusterStats();
  std::string jsonExport(FuelType f);

  index_t x() { return _x; }
  index_t y() { return _y; }
  index_t z() { return _z; }

  smallcount_t numCoolerTypes() const {
    std::set<CoolerType> types;
    for (auto && i : _coolerTypes) {
      if (i != CoolerType::air) {
        types.insert(i);
      }
    }
    return types.size();
  }

  void pruneInactives(bool ignoreConductors = false);

  std::set<coord_t> suggestPrincipledLocations();
  std::set<std::tuple<BlockType, CoolerType, ModeratorType, float> > suggestedBlocksAt(index_t x, index_t y, index_t z, PrincipledSearchMode m = PrincipledSearchMode::computeCooling);

  void floodFillWithConductors()
  {
    std::replace(_blocks.begin(), _blocks.end(), BlockType::air, BlockType::conductor);
  }

  void clearInfeasibleClusters();

private:

  bool _dirty;

  index_t _x;
  index_t _y;
  index_t _z;

  std::vector<BlockType> _blocks;
  std::vector<CoolerType> _coolerTypes;
  std::vector<ModeratorType> _moderatorTypes;
  std::vector<int> _primedStatus;

  std::map<FuelType, float> _powerGeneratedCache;
  std::map<FuelType, float> _dutyCycleCache;

  std::vector<int> _cellActiveCache;
  std::vector<int> _cellValidCache;
  std::vector<int> _cellVisitedCache;

  std::vector<largeindex_t> _clusterIdCache;
  largeindex_t _largestAssignedClusterId;
  std::set<largeindex_t> _validClusterIds;

  std::vector<largeindex_t> _conductorIdCache;
  largeindex_t _largestAssignedConductorId;
  std::set<largeindex_t> _validConductorIds;

  std::vector<float> _cellPositionalEfficiency;
  std::vector<float> _cellModeratorFlux;

  std::vector<coord_t> _unlabeledReactorCellCache;

  std::vector<coord_t> _reactorCellCache;
  std::vector<coord_t> _primedCellCache;

  std::vector<coord_t> _moderatorCache;
  std::vector<coord_t> _reflectorCache;
  std::vector<coord_t> _coolerCache;
  std::vector<coord_t> _conductorCache;

  largecount_t _fluxedModerators;
  std::vector<int> _fluxedModeratorCache;
  largecount_t _sandwichedModerators;

  std::map<vector_offset_t, std::set<vector_offset_t>> _cellAdjacencyCache;
  std::map<vector_offset_t, smallcount_t> _reactorCellObservedCellCount;

  std::map<largeindex_t, float> _outputPerCluster;
  std::map<largeindex_t, float> _heatingPerCluster;
  std::map<largeindex_t, float> _coolingPerCluster;
  std::map<largeindex_t, float> _perClusterSumCellEff;
  std::map<largeindex_t, float> _perClusterSumHeatMult;
  std::map<largeindex_t, largecount_t> _perClusterCellCount;
  std::map<largeindex_t, float> _perClusterEff;


  largecount_t _inactiveBlocks;

  void _revertToSetup();

  void _evaluate(FuelType ft = FuelType::generic);

  smallcount_t _blockTypeAdjacentTo(index_t x, index_t y, index_t z, BlockType bt);

  void _floodFillCluster(index_t x, index_t y, index_t z, largeindex_t clusterID);

  void _floodFillConductor(index_t x, index_t y, index_t z, largeindex_t conductorID);

  bool _hasLineOfSightToOutside(index_t x, index_t y, index_t z);

  float _fuelCellEfficiencyAt(index_t x, index_t y, index_t z, FuelType ft);

  void _fuelCellBroadcastFlux(index_t x, index_t y, index_t z, FuelType ft);
  void _fuelCellBroadcastModeratorActivations(index_t x, index_t y, index_t z);
  void _fuelCellFilterAdjacency(index_t x, index_t y, index_t z);
  void _reflectorUpdate(index_t x, index_t y, index_t z);

  void _setBlockActive(index_t x, index_t y, index_t z);
  void _setBlockValid(index_t x, index_t y, index_t z);
  void _unsetBlockActive(index_t x, index_t y, index_t z);
  void _unsetBlockValid(index_t x, index_t y, index_t z);
};

const std::string & fuelNameForFuelType(FuelType f);

#endif
