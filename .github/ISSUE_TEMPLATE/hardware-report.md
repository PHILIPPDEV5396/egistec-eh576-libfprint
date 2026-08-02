---
name: Hardware report
about: Report how the driver behaves on your machine (the most useful contribution)
title: "[report] <laptop model> — <distro>"
labels: hardware-report
---

<!--
NEVER attach raw fingerprint captures (.bin frames, .pgm/.png images).
They are your biometrics and cannot be reissued. Scores and logs only.
-->

**Laptop model:**
**Distro and version:**
**`lsusb | grep 1c7a`:**

**Did it install?** (`./install.sh` ran clean / errors below)

**Did enrolment complete?** (8 stages / stalled at stage N)

**Does `fprintd-verify` match your enrolled finger?**

**Does fingerprint login work?** (sudo / lock screen / not tried)

### Score table

Output of `python3 tools/test_matching.py`, which prompts each press by name
and prints genuine and impostor scores:

```
paste here
```

### Anything else

Journal output if something failed:
`journalctl -u fprintd --since "10 min ago" | tail -40`
