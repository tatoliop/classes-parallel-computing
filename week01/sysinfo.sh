#!/usr/bin/env bash
# Prints a one-page report about THIS machine (Linux, WSL, macOS).
# usage: ./sysinfo.sh | tee my_machine.txt
set -u
have() { command -v "$1" >/dev/null 2>&1; }
section() { printf '\n=== %s ===\n' "$1"; }

section "OS / kernel"
uname -a
if [ -r /etc/os-release ]; then . /etc/os-release; echo "$PRETTY_NAME"; fi

section "CPU"
if have lscpu; then
  lscpu | grep -E '^(Architecture|Model name|CPU\(s\)|Thread\(s\) per core|Core\(s\) per socket|Socket\(s\)|NUMA node\(s\)|CPU max MHz|L1d|L1i|L2|L3)'
elif have sysctl; then   # macOS
  echo "Model:            $(sysctl -n machdep.cpu.brand_string 2>/dev/null || sysctl -n hw.model)"
  echo "Physical cores:   $(sysctl -n hw.physicalcpu)"
  echo "Logical CPUs:     $(sysctl -n hw.logicalcpu)"
  echo "L1d cache (B):    $(sysctl -n hw.l1dcachesize 2>/dev/null)"
  echo "L2 cache (B):     $(sysctl -n hw.l2cachesize 2>/dev/null)"
  echo "L3 cache (B):     $(sysctl -n hw.l3cachesize 2>/dev/null)"
fi
echo "Logical CPUs (nproc): $(nproc 2>/dev/null || sysctl -n hw.logicalcpu)"

section "Memory"
if have free; then free -h; elif have sysctl; then echo "RAM (bytes): $(sysctl -n hw.memsize)"; fi

section "NUMA"
if have numactl; then numactl --hardware; else echo "numactl not installed (apt install numactl)"; fi

section "GPU"
if have nvidia-smi; then nvidia-smi -L
elif have lspci; then lspci | grep -iE 'vga|3d|display' || echo "no GPU found by lspci"
else echo "no GPU tools available"; fi

section "Toolchain"
for c in gcc g++ make mpicc mpirun nvcc python3; do
  if have "$c"; then printf '%-8s %s\n' "$c" "$($c --version 2>&1 | head -1)"; else printf '%-8s NOT FOUND\n' "$c"; fi
done
