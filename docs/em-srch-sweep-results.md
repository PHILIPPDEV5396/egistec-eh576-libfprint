# EM_SRCH / EM_MIN_OVERLAP sweep, second unit

Measured 2026-09-17, in reply to @PHILIPPDEV5396's finding that the shipped
translation search half-width of 6 px is the dominant cause of this matcher's
false-reject rate (issue #1 here, and PHILIPPDEV5396/libfprint-egis0576#5).

His result, on his unit: widening `EM_SRCH` and changing nothing else took FRR
from 35.0% to 3.3% at the shipped NCC threshold, with no false accepts. This file
is the same question asked of a second unit and a second person's hands.

## Setup

- Hardware: Lenovo Yoga 7 16IRL8, Intel. His was a Yoga 7 14ARB7, AMD.
- Dataset: 5 fingers x 8 presses, one person, captured with `tools/test_matching.py`
  (interactive, every press labelled and lift-confirmed).
- Protocol: enroll presses 0-4, probe presses 5-7, score best-of-5 against the
  template set. 15 genuine and 60 impostor comparisons.
- Impostors are other fingers of the same person, not other people. This differs
  from his protocol and is worth keeping in mind below.
- The raw frames are not in this repo and will not be; they are biometrics.

`driver/egis_match.c` now guards its three tuning constants with `#ifndef`, so the
scorer can be rebuilt at any operating point without editing the file. Defaults
are unchanged and the default build is bit-identical to the previous revision.

```sh
# shipped operating point
cc -O2 tools/em_srch_sweep.c driver/egis_match.c -o sweep -lm
./sweep <dataset-dir> 0.53

# any other point
cc -O2 -DEM_SRCH=19 -DEM_MIN_OVERLAP=1600 \
   tools/em_srch_sweep.c driver/egis_match.c -o sweep -lm
./sweep <dataset-dir> 0.53
```

The last two columns are the honest operating point: the strictest threshold that
admits zero false accepts on this data, and the false-reject rate there.

## EM_MIN_OVERLAP 800 (shipped)

| EM_SRCH | FRR @ 0.53 | FAR @ 0.53 | gen med | imp med | imp max | zero-FAR th | FRR there |
|---|---|---|---|---|---|---|---|
| 6 (shipped) | 73.3% | 1.7% | 0.443 | 0.109 | 0.548 | 0.549 | 73.3% |
| 10 | 53.3% | 5.0% | 0.526 | 0.140 | 0.621 | 0.622 | 66.7% |
| 14 | 26.7% | 8.3% | 0.638 | 0.154 | 0.643 | 0.644 | 53.3% |
| 16 | 20.0% | 10.0% | 0.677 | 0.160 | 0.668 | 0.669 | 46.7% |
| 18 | 6.7% | 11.7% | 0.717 | 0.172 | 0.668 | 0.669 | 40.0% |
| 19 | 6.7% | 11.7% | 0.717 | 0.177 | 0.668 | 0.669 | 33.3% |
| 20 | 6.7% | 11.7% | 0.743 | 0.185 | 0.668 | 0.669 | 26.7% |
| 21 | 6.7% | 15.0% | 0.792 | 0.188 | 0.668 | 0.669 | 26.7% |
| 22 | 6.7% | 15.0% | 0.816 | 0.190 | 0.668 | 0.669 | 13.3% |
| 24 | 6.7% | 21.7% | 0.861 | 0.229 | 0.668 | 0.669 | 13.3% |
| 26 | 6.7% | 21.7% | 0.861 | 0.237 | 0.680 | 0.681 | 13.3% |
| 28 | 0.0% | 23.3% | 0.861 | 0.245 | 0.727 | 0.728 | 13.3% |
| 30 | 0.0% | 23.3% | 0.861 | 0.251 | 0.735 | 0.736 | 13.3% |

Sanity check on the build: at 6/800 the strictest zero-false-accept threshold comes
out at 0.549, which is the 0.53 the driver ships. The harness reproduces the
operating point the driver was tuned to.

## The two constants are coupled

At `EM_SRCH` 6 the overlap floor is inert. 800, 1200, 1600 and 2000 produce
identical output to the digit, because at +/-6 the geometric overlap never comes
near 800 pixels and the floor never binds.

| EM_SRCH | EM_MIN_OVERLAP | FRR @ 0.53 | FAR @ 0.53 | imp max | zero-FAR th | FRR there |
|---|---|---|---|---|---|---|
| 6 | 800 | 73.3% | 1.7% | 0.548 | 0.549 | 73.3% |
| 6 | 1200 | 73.3% | 1.7% | 0.548 | 0.549 | 73.3% |
| 6 | 1600 | 73.3% | 1.7% | 0.548 | 0.549 | 73.3% |
| 6 | 2000 | 73.3% | 1.7% | 0.548 | 0.549 | 73.3% |
| 19 | 800 | 6.7% | 11.7% | 0.668 | 0.669 | 33.3% |
| 19 | 1600 | 6.7% | 10.0% | 0.643 | 0.644 | 26.7% |
| 19 | 2000 | 13.3% | 8.3% | 0.621 | 0.622 | 40.0% |
| 26 | 800 | 6.7% | 21.7% | 0.680 | 0.681 | 13.3% |
| 26 | 1200 | 6.7% | 18.3% | 0.680 | 0.681 | 13.3% |
| 26 | 1600 | 6.7% | 11.7% | 0.654 | 0.655 | 20.0% |
| 26 | 2000 | 13.3% | 8.3% | 0.621 | 0.622 | 40.0% |

At `EM_SRCH` 26, raising the floor from 800 to 1600 cuts FAR at 0.53 from 21.7% to
11.7% and pulls the impostor maximum down from 0.680 to 0.654, with FRR unchanged
at 6.7%.

## Reading

1. **The finding replicates.** FRR at the shipped threshold falls from 73.3% to
   6.7%. The shipped +/-6 is too narrow on this unit as well. Two units, two
   people, same direction and similar magnitude.

2. **"No threshold change needed" does not replicate here.** On this data the
   impostor distribution moves with the search width: the impostor maximum climbs
   0.548 to 0.668 to 0.735, so FAR at a fixed 0.53 rises from 1.7% to 11.7% by
   `EM_SRCH` 19. On his unit it stayed at 0% out to 20. Here both constants have to
   move together.

3. **The win survives that anyway.** At a zero-false-accept threshold: 73.3% FRR
   at 6, 33.3% at 19, 13.3% from 22 up.

4. **The EM_MIN_OVERLAP and EM_COH_TH negatives are not established yet.** Both
   were swept at `EM_SRCH` 6, where the overlap floor provably cannot bind. They
   need re-running at a wide search. This is also the most likely mechanism behind
   the sharp knee he saw at 21: not a property of the translation search, but the
   point at which a fixed absolute floor of 800 begins admitting alignments too
   small to score reliably. NCC over a thin masked overlap is exactly what gives an
   impostor a lucky number. The prediction is that the knee moves right as the
   floor rises.

5. **Next measurement:** a 2D sweep over (`EM_SRCH`, `EM_MIN_OVERLAP`) rather than
   picking a search width against a fixed floor, so the shipped number has a reason
   under it and the threshold moves with it.

## Caveats

- 15 genuine comparisons. One press is 6.7 percentage points, so 13.3% against
  20.0% is a single press and no difference is claimed between those.
- Impostors are the same person's other fingers. That should make this impostor
  set harder to fool rather than easier, which makes the livelier impostor tail
  here more surprising, not less.
- This is a different harness and protocol from his `evaluate.py` two-fold
  evaluation. Treat the direction and the shape as the result; the absolute rates
  are not directly comparable to his table.
