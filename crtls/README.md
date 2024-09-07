# GBA link scripts and CRTLs

This folder contains linkscripts and C Runtime Library (CRT/CRTL) initialization code for multiple GBA development toolchains.

The files are tightly coupled with the compiler and linker versions, so you have to pick one closest to your environment:

- `devkitadv-r4` - original `lnkscript` and `crt0.s` from GBAGI, slightly updated to DevKitAdvance R4
- `gcc-12-custom` - using linkscript from DevKitPro, original `crt0.s` from GBAGI, supports newer GCC/LD versions

## Notes on migration to linkscript from DevKitPro

Source: https://github.com/devkitPro/devkitarm-crtls/blob/master/gba_cart.ld

DevKitPro's gba_cart.ld is the newest I found, it's newer than gba_mb.ld in same repo and newer than linkscripts found in other homebrew project. Specifically, gba_cart.ld covers the most internal symbols introduced by recent gcc/ld versions.

This linkscript was adopted with minimal changes, it is tightly coupled with crt0.s. The original GBAGI crt0.s has more features than the one from devkitarm-crtls, so the original was retained and modified to work with this linkscript.

Notable differences between linkscript from devkitpro and original GBAGI linkscript:

- Expects `.crt0` section (renamed from `.TEXT` in crt0.s)
- No EWRAM overlay (not used and commented from crt0.s)
- Renamed `__data_start` -> `__data_start__` in `crt0.s`
- Renamed `__data_end` -> `__data_end__` in `crt0.s`
- Renamed `__iwram_end` -> `__iwram_end__` in `crt0.s`
- Renamed `__bss_end` -> `__bss_end__` in `crt0.s`
- No `__intr_vector_buf`, `__sp_usr_offset` (restored in linkscript, required by old crt0.s)
- No `end` symbol (restored in linkscript, required by libnosys sbrk())

Other notes:

- DevKitAdvance's ld does not support the ORIGIN or SORT_NONE expressions in linkscripts
- DevKitPro has different linkscripts for cart and multiboot
- Original GBAGI linkscript supports both cart and multiboot, but built for cart by cefault
- To diagnose linker errors or broken image, enable errors on orphan sections - a newer version of the tools likely introduced a new section that was misplaced by ld's heuristics
