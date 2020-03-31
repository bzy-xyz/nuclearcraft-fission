#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <vector>
#include <set>
#include <array>

#include <json/json.h>

enum struct BlockType {
  air = 0,
  reactorCell, // 1
  moderator, // 2
  cooler, // 3
  casing,
  BLOCK_TYPE_MAX
};

enum struct CoolerType {
  air = 0,
  water, // B
  redstone, // C
  quartz, // D
  gold, // E
  glowstone, // F
  lapis, // G
  diamond, // H
  liquidHelium, // I
  enderium, // J
  cryotheum, // K
  iron, // L
  emerald, // M
  copper, // N
  tin, // O
  magnesium, // P
  activeWater, // Q
  activeCryotheum, // R
  COOLER_TYPE_MAX
};

enum struct FuelType {
  air = 0,
  generic,
  TBU, TBUO, // 3
  LEU233, LEU233O, // 5
  HEU233, HEU233O, // 7
  LEU235, LEU235O, // 9
  HEU235, HEU235O, // 11
  LEN236, LEN236O, // 13
  HEN236, HEN236O, // 15
  LEP239, LEP239O, // 17
  HEP239, HEP239O, // 19
  LEP241, LEP241O, // 21
  HEP241, HEP241O, // 23
  MOX239, MOX241,  // 25
  LEA242, LEA242O, // 27
  HEA242, HEA242O, // 29
  LECm243, LECm243O, // 31
  HECm243, HECm243O, // 33
  LECm245, LECm245O, // 35
  HECm245, HECm245O, // 37
  LECm247, LECm247O, // 39
  HECm247, HECm247O, // 41
  LEB248, LEB248O, // 43
  HEB248, HEB248O, // 45
  LECf249, LECf249O, // 47
  HECf249, HECf249O, // 49
  LECf251, LECf251O, // 51
  HECf251, HECf251O, // 53
  FUEL_TYPE_MAX
};

#define index_t int_fast8_t
#define vector_offset_t int_fast32_t
#define smallcount_t int_fast8_t
#define largecount_t int_fast32_t

typedef std::array<index_t, 3> coord_t;

#define _XYZ(__x, __y, __z) (__x * (_y * _z) + __y * (_z) + __z)
#define UNPACK(vec) (vec)[0], (vec)[1], (vec)[2]
#define TO_XYZ(n) (n) / (_y * _z), ((n) % (_y * _z)) / _z, (n) % _z

class Reactor {
public:
  Reactor(index_t x = 1, index_t y = 1, index_t z = 1);
  ~Reactor();

  static Reactor * fromJsonFile(std::string fn);

  void toJsonFile(std::string fn);

  /** Total power generated for fuel type.
    *
    * @note FuelType::air is empty reactor;
    *       FuelType::generic return raw multiplier (i.e. normalized to 1).
    */
  inline float powerGenerated(FuelType ft) {
    _evaluate(ft);
    return _powerGeneratedCache[ft];
  }
  /** Total heat generated for fuel type.
    *
    * @note FuelType::air is empty reactor (raw cooling);
    *       FuelType::generic is raw multiplier WITHOUT COOLING.
    */
  inline float heatGenerated(FuelType ft) {
    _evaluate(ft);
    return _heatGeneratedCache[ft];
  }

  inline float effectivePowerGenerated(FuelType ft) {
    _evaluate(ft);
    if (heatGenerated(ft) < 0) {
      return powerGenerated(ft);
    }
    else if (heatGenerated(FuelType::air) == 0) {
      return 0;
    }
    else {
      return powerGenerated(ft) * heatGenerated(FuelType::air) / (heatGenerated(FuelType::air) - heatGenerated(ft));
    }
  }

  inline largecount_t inactiveBlocks() {
    return _inactiveBlocks;
  }

  inline void setCell(index_t x, index_t y, index_t z, BlockType bt, CoolerType ct) {
    if (x < 0 || y < 0 || z < 0 || x >= _x || y >= _y || z >= _z) {
      return;
    }

    _dirty = true;
    _blocks[x * (_y * _z) + y * (_z) + z] = bt;
    _coolerTypes[x * (_y * _z) + y * (_z) + z] = bt == BlockType::cooler ? ct : CoolerType::air;
  }

  inline bool isInBounds(index_t x, index_t y, index_t z)
  {
    return !(x < 0 || y < 0 || z < 0 || x >= _x || y >= _y || z >= _z);
  }

  inline BlockType blockTypeAt(index_t x, index_t y, index_t z) {
    if (x < 0 || y < 0 || z < 0 || x >= _x || y >= _y || z >= _z) {
      return BlockType::casing;
    }
    return _blocks[x * (_y * _z) + y * (_z) + z];
  }

  inline CoolerType coolerTypeAt(index_t x, index_t y, index_t z) {
    if (x < 0 || y < 0 || z < 0 || x >= _x || y >= _y || z >= _z) {
      return CoolerType::air;
    }
    return _coolerTypes[x * (_y * _z) + y * (_z) + z];
  }

  bool coolerTypeActiveAt(index_t x, index_t y, index_t z, CoolerType ct = CoolerType::air);


  inline bool coolerActiveAt(index_t x, index_t y, index_t z)
  {
    if(_dirty)
    {
      _cellActiveCache = std::vector<int>(_x * _y * _z, 0);
    }

    if(_cellActiveCache[x * (_y * _z) + y * (_z) + z])
    {
      return _cellActiveCache[x * (_y * _z) + y * (_z) + z] == 1;
    }

    CoolerType ct = coolerTypeAt(x, y, z);
    bool r = coolerTypeActiveAt(x, y, z, ct);

    /*if(!r && ct != CoolerType::air)
    {
      fprintf(stderr, "inactive cooler type %u at %d %d %d\n", ct, x, y, z);
    }*/

    if(r) {
      _cellActiveCache[x * (_y * _z) + y * (_z) + z] = 1;
    }
    else {
      _cellActiveCache[x * (_y * _z) + y * (_z) + z] = -1;
    }

    return r;
  }

  inline bool moderatorActiveAt(index_t x, index_t y, index_t z) {
    bool r = reactorCellsAdjacentTo(x, y, z);
    /*if(!r)
    {
      fprintf(stderr, "inactive moderator at %d %d %d\n", x, y, z);
    }*/
    return r;
  }

  smallcount_t reactorCellsAdjacentTo(index_t x, index_t y, index_t z);
  smallcount_t moderatorsAdjacentTo(index_t x, index_t y, index_t z);
  smallcount_t activeModeratorsAdjacentTo(index_t x, index_t y, index_t z);
  inline smallcount_t reactorCasingsAdjacentTo(index_t x, index_t y, index_t z) {
    return _blockTypeAdjacentTo(x, y, z, BlockType::casing);
  }

  /** Number of coolers of a certain type adjacent to a cell.
   *
   * @note CoolerType::air := any cooler type
   */
  smallcount_t activeCoolersAdjacentTo(index_t x, index_t y, index_t z, CoolerType ct = CoolerType::air);

  std::set<coord_t> suggestPrincipledLocations();
  std::vector<std::tuple<BlockType, CoolerType, float> > suggestedBlocksAt(index_t x, index_t y, index_t z, FuelType ft);

  inline bool operator==(const Reactor &b) const {
    return  _x == b._x && _y == b._y && _z == b._z
        &&  _blocks == b._blocks && _coolerTypes == b._coolerTypes;
  }

  inline bool operator<(const Reactor & b) const {
    return _blocks < b._blocks
        || (_blocks == b._blocks && _coolerTypes < b._coolerTypes);
  }

  inline largecount_t totalCells() const {
    return std::count(_blocks.begin(), _blocks.end(), BlockType::reactorCell);
  }

  inline smallcount_t numCoolerTypes() const {
    std::set<CoolerType> types(_coolerTypes.begin(), _coolerTypes.end());
    return types.size();
  }

  inline std::set<CoolerType> coolerTypes() const {
    return std::set<CoolerType>(_coolerTypes.begin(), _coolerTypes.end());
  }

  std::string describe();

  friend struct std::hash<Reactor>;

  index_t x() { return _x; }
  index_t y() { return _y; }
  index_t z() { return _z; }

private:

  bool _dirty;

  index_t _x;
  index_t _y;
  index_t _z;

  std::vector<int> offsets;

  std::vector<BlockType> _blocks;
  std::vector<CoolerType> _coolerTypes;

  std::map<FuelType, float> _powerGeneratedCache;
  std::map<FuelType, float> _heatGeneratedCache;
  std::vector<int> _cellActiveCache;
  std::vector<int> _cellModeratorAdjacencyCache;

  std::vector<int> _reactorCellCache;
  std::vector<int> _moderatorCache;
  std::vector<int> _coolerCache;


  largecount_t _inactiveBlocks;

  void _evaluate(FuelType ft = FuelType::generic);

  smallcount_t _blockTypeAdjacentTo(index_t x, index_t y, index_t z, BlockType bt);

  bool _hasPathToOutside(index_t x, index_t y, index_t z);
  bool _hasPathToOutside_initStep(index_t x, index_t y, index_t z, std::set<coord_t> & visited);
  bool _hasPathToOutside_followStep(index_t x, index_t y, index_t z, std::set<coord_t> & visited);
};

namespace std {
  template <> struct hash<Reactor>
  {
    std::size_t operator()(const Reactor & r) const
    {
      std::size_t ret = 0x811c9dc5;
      /*for(auto z = r._blocks.begin(); z == r._blocks.end(); z++)
      {
        ret *= 16777619;
        ret ^= static_cast<std::size_t>(*z);
      }*/
      for(auto z = r._coolerTypes.begin(); z == r._coolerTypes.end(); z++)
      {
        ret *= 16777619;
        ret ^= static_cast<std::size_t>(*z);
      }
      return ret;
    }
  };
};

const std::string & fuelNameForFuelType(FuelType f);

#endif
