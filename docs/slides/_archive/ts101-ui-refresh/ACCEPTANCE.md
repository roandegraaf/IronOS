## Your turn (4)

1. [done 2026-10-30, `--skip-build` run: RDY, ejected] **Flash the fork** — hold the front button while plugging the TS101 into the Mac, then run `scripts/flash_ts101_mac.sh`
   → script builds `TS101_EN.hex`, reports RDY and ejects `/Volumes/TS101_DFU`
2. **Check the panel** — boot on USB power with no tip
   → full screen aligned and uncropped, like v2.23
3. **Solder for real** — on a 45 W+ PD charger (or DC jack) with a tip, heat to 320 and solder; toggle boost
   → screens update smoothly, heat column and boost chevron react, no reboots or glitches
4. **Confirm the design calls** — check boost (chevron + boost temp), heat column on the button side, USB-C/battery/bolt icon switching on DC vs PD
   → all look right on the iron, or note what to change
