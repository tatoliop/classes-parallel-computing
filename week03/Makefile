include ../common/config.mk
BINS := profile_me profile_me_pg

all: $(BINS)
profile_me: profile_me.c
	$(CC) $(CFLAGS) $< -o $@
# gprof needs -pg and no inlining/optimisation surprises
profile_me_pg: profile_me.c
	$(CC) -O1 -fno-inline -pg -Wall -std=gnu11 $< -o $@
clean:
	rm -f $(BINS) gmon.out perf.data*
.PHONY: all clean
