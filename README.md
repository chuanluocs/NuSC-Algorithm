# *NuSC*: An Effective Local Search Algorithm for Solving the Set Covering Problem

*NuSC* is an effective local search algorithm for solving the set covering problem. This repository includes the implementation of *NuSC* and the testing instances.

## Run *NuSC* solver

### 1. Clone the repository
```
git clone https://github.com/chuanluocs/NuSC-Algorithm.git
```

### 2. Build *NuSC*
```
cd NuSC-Algorithm/NuSC
make
```

### 3. Run executable
*NuSC* algorithm can be run with binary file: **NuSC-algorithm/NuSC/NuSC**

#### Instruction for running *NuSC*
```
./NuSC  <instance> <cutoff time> <seed> [λ] [prob] [γ]
```

#### Example (Run *NuSC* on rail benchmark)

```
./NuSC ../Benchmarks/Rail/rail-4872.txt 1000 1
```


## Output format
The output file of our solver consists of the solution searching path. A typical output file make looks like:

```
o 181 0.14

o 180 0.21

v 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 ......
```
The notation 'o' means the recorded solution path followed with objective value (e.g. 180) and the time this solution was found (in seconds, e.g. 0.21).

The notation 'v' followed with the allocation result.

## Authors

- Chuan Luo (<chuanluophd@outlook.com>)
- Wenqian Xing (<wx2261@columbia.edu>)

## Related Paper

Chuan Luo, Wenqian Xing, Shaowei Cai, and Chunming Hu. *NuSC: An Effective Local Search Algorithm for Solving the Set Covering Problem.* To appear in IEEE Transactions on Cybernetics. 