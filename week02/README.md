# Εβδομάδα 2 – Χρονομέτρηση και ο νόμος του Amdahl

## Benchmarking & Amdahl
```bash
make
# 1. Timing
time ./matmul_seq 512                # real vs user vs sys
../scripts/bench.sh 5 ./matmul_seq 512      # 5 φορές: min/mean/max/stddev
./matmul_seq 256; ./matmul_seq 512; ./matmul_seq 1024   # πώς κλιμακώνεται ο χρόνος (n^3);

# 2. Amdahl
python3 amdahl.py                    # πίνακας + amdahl.png
python3 amdahl.py -f 0.8 0.95 -p 32
```