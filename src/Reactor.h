#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <vector>
#include <set>
#include <array>

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
  TBU, TBUO,
  LEU233, LEU233O,
  HEU233, HEU233O,
  LEU235, LEU235O,
  HEU235, HEU235O,
  LEN236, LEN236O,
  HEN236, HEN236O,
  LEP239, LEP239O,
  HEP239, HEP239O,
  LEP241, LEP241O,
  HEP241, HEP241O,
  MOX239, MOX241,
  LEA242, LEA242O,
  HEA242, HEA242O,
  LECm243, LECm243O,
  HECm243, HECm243O,
  LECm245, LECm245O,
  HECm245, HECm245O,
  LECm247, LECm247O,
  HECm247, HECm247O,
  LEB248, LEB248O,
  HEB248, HEB248O,
  LECf249, LECf249O,
  HECf249, HECf249O,
  LECf251, LECf251O,
  HECf251, HECf251O,
  FUEL_TYPE_MAX
};

#define index_t int_fast8_t
#define vector_offset_t int_fast32_t
#define smallcount_t int_fast8_t
#define largecount_t int_fast32_t

typedef std::array<index_t, 3> coord_t;

class Reactor {
public:
  Reactor(index_t x = 1, index_t y = 1, index_t z = 1);
  ~Reactor();

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

  std::vector<BlockType> _blocks;
  std::vector<CoolerType> _coolerTypes;

  std::map<FuelType, float> _powerGeneratedCache;
  std::map<FuelType, float> _heatGeneratedCache;
  std::vector<int> _cellActiveCache;

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
