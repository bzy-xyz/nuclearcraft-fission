# nuclearcraft-fission

A program to search for viable NuclearCraft fission reactors.

## Requirements

* A c++17 compatible C compiler with OpenMP support.

## Limitations

* Many.

## Usage

`search [x y z] [fuelType] [coolerRestrictions] [strategy]`

`x y z` dimensions of reactor (default 5x5x5).

`fuelType` (2 - ???) is the fuel to optimize for.

`coolerRestrictions` (0 - 2):

* doesn't matter right now

`strategy` (0 - 2):

* 1: effective output

(everything else is untested)

## Strategy

* First spends a few hundred steps proposing a cell + moderator + reflector topology.
* Then tries to cool that topology.
* If it balances, optimizes moderators for output.

## Output

Upon finishing or aborting early, produces a report:

* `N` number of reactor cells.
* `Cl` number of clusters.
* `O` raw objective function value.
* `i` number of inactive cells.
* `[fuel] totalOutput effectiveOutput effectiveOutputPerCell`

followed by the reactor structure and per-cluster stats.
