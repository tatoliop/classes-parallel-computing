CC     ?= gcc
CFLAGS ?= -O2 -Wall -fopenmp
LDLIBS  = -lm
STEPS = step01_team step02_scope step02_bug step02_bug_O2 step03_for step04_schedule \
        step05_collapse_ordered step06_sync step07_reduction step08_sections
all: $(STEPS)
# Η εκδοχή με το λάθος: με -O0, γιατί με -O2 ο compiler συνήθως κρύβει το race
step02_bug: step02_scope.c sensors.h
	$(CC) -O0 -Wall -fopenmp -DBUG $< -o $@ $(LDLIBS)
step02_bug_O2: step02_scope.c sensors.h
	$(CC) $(CFLAGS) -DBUG $< -o $@ $(LDLIBS)
%: %.c sensors.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)
clean:
	rm -f $(STEPS) report.csv
.PHONY: all clean
