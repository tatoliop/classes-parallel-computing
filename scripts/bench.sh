#!/usr/bin/env bash
# Run a command N times and report min / mean / max / stddev of the wall time.
# usage: scripts/bench.sh N command [args...]
#   e.g. scripts/bench.sh 5 ./matmul_seq 512
set -eu
if [ $# -lt 2 ]; then echo "usage: $0 N command [args...]" >&2; exit 1; fi
n=$1; shift
TIMEFORMAT=%R
for _ in $(seq "$n"); do
  { time "$@" >/dev/null 2>&1; } 2>&1
done | awk '{ s+=$1; ss+=$1*$1; if(NR==1||$1<mn)mn=$1; if(NR==1||$1>mx)mx=$1 }
  END { m=s/NR; v=(NR>1)?(ss-NR*m*m)/(NR-1):0; if(v<0)v=0;
        printf "runs=%d  min=%.4fs  mean=%.4fs  max=%.4fs  stddev=%.4fs\n", NR, mn, m, mx, sqrt(v) }'
