# Εγκατάσταση περιβάλλοντος

Όλα τα εργαστήρια τρέχουν σε Linux. Διαλέξτε **μία** από τις παρακάτω επιλογές και
τρέξτε στο τέλος `scripts/check_env.sh` για να δείτε τι λείπει.

## 1. Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential git openssh-client openssh-server tmux htop hwloc numactl \
                 valgrind openmpi-bin libopenmpi-dev python3 python3-matplotlib
# προαιρετικά:
sudo apt install linux-tools-generic        # perf
pip install pyspark                          # εβδομάδα 8 (χρειάζεται Java: sudo apt install default-jre-headless)
```

## 2. Windows 10/11 → WSL2
1. Σε PowerShell ως διαχειριστής: `wsl --install -d Ubuntu` και επανεκκίνηση.
2. Ανοίξτε το Ubuntu και ακολουθήστε τις οδηγίες της ενότητας 1.
3. Δουλέψτε μέσα στο home του Linux (`~/parallel-labs`), **όχι** στο `/mnt/c/...` (αργό).
4. Περιορισμοί: το `perf` συνήθως δεν δουλεύει· το `nvidia-smi`/CUDA χρειάζεται ειδικό driver.
   Για CUDA χρησιμοποιήστε Colab (ενότητα 5).

## 3. macOS
```bash
xcode-select --install
brew install gcc open-mpi libomp hwloc python
pip3 install matplotlib pyspark
```
* Το `gcc` του Apple είναι στην πραγματικότητα clang· το `config.mk` προσθέτει αυτόματα
  τα flags για `libomp`. Αν προτιμάτε GNU gcc: `make CC=gcc-14` (ή η έκδοση που εγκαταστήσατε)
  και αντικαταστήστε τα OMPFLAGS με `-fopenmp`.
* Δεν υπάρχουν `perf`, `numactl`, `valgrind` (ARM). Χρησιμοποιήστε μόνο μετρήσεις χρόνου.
* Δεν υποστηρίζεται CUDA → Colab.

## 4. Docker (ίδιο περιβάλλον για όλους)
```bash
docker build -t parallel-labs -f docker/Dockerfile .
docker run --rm -it -v "$PWD":/home/student/parallel-labs parallel-labs
```
Περιέχει gcc, OpenMP, Open MPI, Python, PySpark, valgrind. Δεν περιέχει CUDA.

## 5. GPU χωρίς NVIDIA κάρτα → Google Colab
1. Ανεβάστε το `week10/cuda_colab.ipynb` στο <https://colab.research.google.com>.
2. *Runtime → Change runtime type → T4 GPU*.
3. Τρέξτε τα κελιά με τη σειρά: γράφουν τα `.cu` αρχεία, τα κάνουν compile με `nvcc` και τα τρέχουν.

## Συχνά προβλήματα
| Σύμπτωμα | Λύση |
|---|---|
| `mpirun` παραπονιέται: *not enough slots* | `mpirun --oversubscribe -np 4 ...` (ή `export OMPI_MCA_rmaps_base_oversubscribe=1`) |
| `mpirun` ως root (Docker) | `export OMPI_ALLOW_RUN_AS_ROOT=1 OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1` |
| `fatal error: omp.h not found` (macOS) | `brew install libomp` |
| Τα αποτελέσματα του speedup είναι «τυχαία» | Κλείστε άλλες εφαρμογές, τρέξτε 3–5 φορές (`scripts/bench.sh`), μην συγκρίνετε με μπαταρία/λειτουργία εξοικονόμησης |
| Λιγότερα νήματα από όσα περιμένατε | Δείτε `nproc` και `lscpu`· σε VM/WSL ίσως έχουν δοθεί λίγοι πυρήνες |
