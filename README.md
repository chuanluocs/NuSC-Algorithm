# NuSC
- An Effective Local Search Algorithm for Solving the Set Covering Problem


## 1. Run NuSC solver

#### 1. Clone the repository
```bash
git clone https://github.com/chuanluocs/NuSC-Algorithm.git
```

#### 2. Run binary file
NuSC algorithm can be run with binary file: **NuSC-algorithm/NuSC/NuSC**

##### Example (Run NuSC on rail benchmark)
```
cd NuSC
```
```
./NuSC  <instance> <cutoff time> <seed> [λ] [prob] [γ]
```
```bash
./NuSC ../Benchmarks/Rail/rail-4872.txt 1000 1
```


## 2. Output format
The output file of our solver consists of the solution searching path. A typical output file make looks like:

```bash
o 181 0.14

o 180 0.21

v 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 ......
```
The notation 'o' means the recorded solution path followed with objective value (e.g. 180) and the time this solution was found (in seconds, e.g. 0.21).

The notation 'v' followed with the allocation result.
