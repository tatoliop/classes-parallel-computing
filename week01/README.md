# Εβδομάδα 1 – Linux terminal, SSH/SCP και αρχιτεκτονική του μηχανήματος

### 1. Βασικά του terminal
```bash
pwd; ls -lah; cd ~; mkdir -p demo && cd demo
echo "γεια" > a.txt; cat a.txt; cp a.txt b.txt; mv b.txt c.txt; ls
cat /etc/os-release | grep -i name
ls -l /etc | head -5            # pipes
ls /etc | grep conf | wc -l     # grep + wc
man ls                           # q για έξοδο;  history;  Tab = autocomplete
```

### 2. SSH, SCP, rsync – δείτε `ssh_cheatsheet.md`
```bash
ssh user@host                    # login
ssh-keygen -t ed25519 && ssh-copy-id user@host      # login χωρίς κωδικό
ssh user@host 'hostname; nproc'  # μία εντολή εξ αποστάσεως
scp hello.c user@host:~/         # αντιγραφή προς τον server
scp user@host:~/hello.c ./back.c
rsync -avz week01/ user@host:~/week01/
tmux new -s work                 # Ctrl-b d  για detach,  tmux attach -t work
```
*Αν δεν υπάρχει server:* `ssh $USER@localhost` (εγκατεστημένο `openssh-server`).

### 3. Αρχιτεκτονική του μηχανήματος
```bash
uname -a                         # kernel, αρχιτεκτονική (x86_64 / aarch64)
lscpu                            # μοντέλο, sockets, cores, threads/core, caches, NUMA
nproc                            # πόσα λογικά νήματα
lscpu | grep -E 'Model name|Socket|Core|Thread|L1d|L2|L3|NUMA'
cat /proc/cpuinfo | grep "model name" | head -2
free -h                          # RAM
numactl --hardware               # NUMA κόμβοι και μνήμη ανά κόμβο
lstopo                           # (ή lstopo --of txt) γραφική τοπολογία
nvidia-smi  ||  lspci | grep -i vga      # GPU
./sysinfo.sh                     # όλα τα παραπάνω σε μία αναφορά
```
**Ζωντανή χρήση πυρήνων:** άνοιξε `htop` σε δεύτερο τερματικό και τρέξε
```bash
make
./burn 20 &                      # ένας πυρήνας 100%
./burn 20 & ./burn 20 & ./burn 20 &
jobs; pkill burn
```

### 4. Το πρώτο πρόγραμμα
```bash
gcc hello.c -o hello && ./hello
time ./hello                     # real / user / sys: τι σημαίνει το καθένα;
cat Makefile; make; make clean; make     # εισαγωγή στο make
```

## Προσοχή
* **macOS:** δεν υπάρχουν `lscpu`, `free`, `numactl`, `lstopo` – το `sysinfo.sh` χρησιμοποιεί `sysctl`.
* **WSL:** το `lscpu` δείχνει τα εικονικά χαρακτηριστικά της VM· οι caches μπορεί να λείπουν.
