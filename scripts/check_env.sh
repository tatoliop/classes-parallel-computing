#!/usr/bin/env bash
# Checks that the tools needed by the course are installed.
#   scripts/check_env.sh
have() { command -v "$1" >/dev/null 2>&1; }
ok=0
ver() { case "$1" in ssh) ssh -V 2>&1 | head -1 ;; tmux) tmux -V ;; *) "$1" --version 2>&1 | head -1 ;; esac; }
check() {   # name  required(yes/no)  hint
  if have "$1"; then printf '  [ OK ] %-10s %s\n' "$1" "$(ver "$1")"
  else
    if [ "$2" = yes ]; then printf '  [MISS] %-10s REQUIRED  -> %s\n' "$1" "$3"; ok=1
    else printf '  [ -- ] %-10s optional  -> %s\n' "$1" "$3"; fi
  fi
}
echo "Core (weeks 1-5):"
check gcc yes "sudo apt install build-essential   |  brew install gcc"
check g++ yes "sudo apt install build-essential"
check make yes "sudo apt install build-essential"
check ssh yes "sudo apt install openssh-client"
check python3 yes "sudo apt install python3"
echo "Optional tools:"
check htop no "sudo apt install htop"
check lstopo no "sudo apt install hwloc"
check numactl no "sudo apt install numactl"
check valgrind no "sudo apt install valgrind  (not on macOS ARM)"
check perf no "sudo apt install linux-tools-generic  (often unavailable in WSL/Docker)"
check tmux no "sudo apt install tmux"
echo "MPI (weeks 6,7,9):"
check mpicc yes "sudo apt install openmpi-bin libopenmpi-dev   |  brew install open-mpi"
check mpirun yes "same as above"
echo "CUDA (weeks 10,11) - only with an NVIDIA GPU, otherwise use Google Colab:"
check nvcc no "install the CUDA Toolkit, or use week10/cuda_colab.ipynb in Colab"
check nvidia-smi no "NVIDIA driver"
echo "OpenMP test:"
tmp=$(mktemp -d); printf '#include <omp.h>\n#include <stdio.h>\nint main(){\n#pragma omp parallel\n{\n#pragma omp single\nprintf("OpenMP ok, %%d threads\\n", omp_get_num_threads());\n}\nreturn 0;}\n' > "$tmp/t.c"
case "$(uname -s)" in
  Darwin) lo=$(brew --prefix libomp 2>/dev/null); gcc -Xpreprocessor -fopenmp -I"$lo/include" -L"$lo/lib" -lomp "$tmp/t.c" -o "$tmp/t" 2>/dev/null ;;
  *) gcc -fopenmp "$tmp/t.c" -o "$tmp/t" 2>/dev/null ;;
esac
if [ -x "$tmp/t" ]; then "$tmp/t"; else echo "  [MISS] OpenMP does not compile (macOS: brew install libomp)"; ok=1; fi
rm -rf "$tmp"
exit $ok
