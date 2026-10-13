# SSH / SCP / rsync cheat sheet

```bash
# --- connect ---------------------------------------------------------
ssh user@host                    # login (add -p 2222 for another port)
ssh user@host 'lscpu | head'     # run ONE command remotely and come back
ssh -X user@host                 # X11 forwarding (rarely needed)

# --- passwordless login (do this once) ---------------------------------
ssh-keygen -t ed25519            # creates ~/.ssh/id_ed25519(.pub); press Enter for defaults
ssh-copy-id user@host            # installs the public key on the server
ssh user@host                    # no password asked any more

# --- aliases: ~/.ssh/config ---------------------------------------------
#   Host lab
#       HostName cluster.example.edu
#       User student01
#       Port 22
#   then simply:   ssh lab     scp file lab:~/     rsync -av dir/ lab:~/dir/

# --- copy files --------------------------------------------------------
scp hello.c user@host:~/              # local  -> remote
scp user@host:~/results.txt .         # remote -> local
scp -r week01 user@host:~/            # a whole directory
rsync -avz --progress week01/ user@host:~/week01/   # only sends what changed

# --- keep jobs alive after you disconnect ---------------------------------
tmux new -s work                 # start a session
#   Ctrl-b d                      -> detach
tmux ls
tmux attach -t work              # come back later
```

No server available? Practice on your own machine:
`sudo apt install openssh-server` (Linux/WSL) and then `ssh $USER@localhost`.
On macOS: System Settings → General → Sharing → Remote Login.
