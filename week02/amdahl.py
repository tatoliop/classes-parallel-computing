#!/usr/bin/env python3
"""Amdahl's and Gustafson's laws.

    python3 amdahl.py                      # table + amdahl.png
    python3 amdahl.py -f 0.9 0.99 -p 128   # your own parallel fractions

f = fraction of the *sequential* run time that can be parallelised.
Amdahl    : S(p) = 1 / ((1-f) + f/p)
Gustafson : S(p) = (1-f) + f*p        (f measured on the parallel run)
"""
import argparse


def amdahl(f, p):
    return 1.0 / ((1.0 - f) + f / p)


def gustafson(f, p):
    return (1.0 - f) + f * p


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("-f", "--fractions", type=float, nargs="+", default=[0.5, 0.9, 0.95, 0.99])
    ap.add_argument("-p", "--max-p", type=int, default=64)
    ap.add_argument("-o", "--out", default="amdahl.png")
    a = ap.parse_args()

    procs = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
    procs = [p for p in procs if p <= a.max_p] or [1, a.max_p]
    print("p".rjust(6) + "".join(f"  f={f:<5} (A/G)".rjust(22) for f in a.fractions))
    for p in procs:
        cells = "".join(f"{amdahl(f, p):9.2f} /{gustafson(f, p):8.2f}".rjust(22) for f in a.fractions)
        print(f"{p:6d}{cells}")
    print("\nlimit as p -> infinity (Amdahl): " +
          ", ".join(f"f={f}: {1 / (1 - f):.0f}x" if f < 1 else f"f={f}: inf" for f in a.fractions))

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed - table only (pip install matplotlib)")
        return
    ps = list(range(1, a.max_p + 1))
    plt.figure(figsize=(6, 4))
    for f in a.fractions:
        plt.plot(ps, [amdahl(f, p) for p in ps], label=f"f = {f}")
    plt.plot(ps, ps, "k--", label="ideal")
    plt.xlabel("processors p")
    plt.ylabel("speedup")
    plt.title("Amdahl's law")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(a.out, dpi=120)
    print("wrote", a.out)


if __name__ == "__main__":
    main()
