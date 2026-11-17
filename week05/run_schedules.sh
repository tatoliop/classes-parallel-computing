#!/usr/bin/env bash
# Βήμα 4: σύγκριση πολιτικών χωρίς επαναμεταγλώττιση.
export OMP_NUM_THREADS=${OMP_NUM_THREADS:-$(nproc)}
for s in static static,1 static,16 dynamic,1 dynamic,16 guided auto; do
  OMP_SCHEDULE=$s ./step04_schedule "$@" | head -1
done
