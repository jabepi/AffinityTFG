#!/bin/bash
#SBATCH --nodelist=ilk-1,ilk-2,ilk-3,ilk-4
#SBATCH -c 32
#SBATCH --mem 64G
#SBATCH -t 00:01:00
#SBATCH --exclusive

numactl -H