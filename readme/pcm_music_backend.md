PCM Hybrid Music Backend
========================

Purpose
-------
This backend lets AGI music commands play ROM-resident PCM tracks instead of
running the live SN76496 software mixer. The original live Tandy path remains
available and selectable.

Audio Format
------------
- Encoding: 8-bit signed PCM
- Channels: mono
- Sample rate: 15360 Hz
- Playback path: Direct Sound FIFO A via DMA
- Buffering: double-buffered 384-sample chunks

Asset Storage
-------------
Store PCM tracks as `const S8` arrays in ROM and wrap each one in a
`pcm_track_asset` descriptor:

```c
static const S8 my_track_data[] = { ... };
static const pcm_track_asset my_track = {
	my_track_data,
	sizeof(my_track_data),
	0,
	TRUE
};
```

Track Mapping
-------------
Add a row to `s_pcm_track_mappings` in [pcm_music.c](/H:/CODEX/GBAGI/GBAGI-fresh-2026-04-16/GBAGI/pcm_music.c):

```c
{ "KQ1", 5, &my_track },
```

Fields:
- `game_id`: AGI game id such as `KQ1`
- `sound_num`: AGI sound resource number
- `track`: pointer to the PCM asset descriptor

Behavior
--------
- In `Live Tandy` backend mode, music uses the existing live SN76496 path.
- In `Hybrid PCM` backend mode:
  - mapped music tracks stream from ROM as PCM
  - unmapped music falls back to the legacy live path
  - sound effects still run through the live lightweight PSG path

