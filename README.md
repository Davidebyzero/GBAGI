![mockup](https://github.com/user-attachments/assets/4aa2ce8b-7da4-47d4-844d-2bf692cea799)

🎥 Demo Video
Watch GBAGI in action:
https://youtu.be/MRohIiQh68I?si=ul9iWzklJaf0n1uS

# GBAGI 2.40

Game Boy Advance Sierra AGI packer and runtime with a modernized injector, per-game vocabulary editing, and walkthrough analysis tools.

This version focuses on making AGI games easier to customize for GBA without hand-editing packer code every time. The injector can now edit vocabulary at the game level, preview the picker layout, and work together with an HTML analyzer that exports injector-compatible presets.

## What's New

### Vocabulary editor in the injector

The Win32 injector now includes a per-game vocabulary editor.

You can:

- keep or remove existing words
- add aliases to existing groups
- hide words from the picker
- force a word to the left or right column
- force a word to appear in normal view or only in `More`
- preview the picker layout inside the editor

The editor stores changes as a vocabulary plan and uses that plan during ROM building.

### Generic vocab plan support

The packer is no longer limited to hardcoded game-specific survivor edits.

The GUI builder now supports a generic vocab plan that can describe:

- explicit keeps and removals
- hidden words
- added aliases
- picker visibility overrides
- column overrides

This allows game-specific vocabulary tuning from the injector UI instead of only through manual source edits.

### Walkthrough analyzer

The project now includes an HTML walkthrough and vocabulary analyzer:

- `gbagi_vocab_analyzer.html`

It can:

- analyze ROM vocabulary against `WORDS.TOK`
- analyze walkthrough words and command phrases
- export `gbagi_vocab_preset.tsv`
- export a plain-text change summary
- sync analyzer edits back into the injector workflow

The injector opens this analyzer from the `Walkthrough Analyzer` button.

### Sync bridge between analyzer and injector

When the analyzer is opened from the injector, it can work from injector-generated data and write changes back through a sync TSV bridge.

Current flow:

1. Open `Walkthrough Analyzer` from the injector.
2. Make vocabulary changes in the HTML analyzer.
3. Click `Update Injector`.
4. Return to the injector and continue with `Vocabulary...`, `Walkthrough Analyzer`, or `Build ROM`.

### Runtime picker improvements

The runtime now supports:

- hidden word flags
- show-normal and show-more word flags
- filtering of internal debug words when debug mode is off

Debug words such as `tp`, `sp`, `var`, `xy`, `sv`, `sf`, `h`, `hn`, `sn`, and `cm` remain available internally for debug mode but are hidden from the normal picker.

### Built-in hardware-safe ROM output

The injector no longer depends on an external donor ROM for header fixing.

The build flow now applies the required GBA header/logo fix internally, including:

- Nintendo logo block
- fixed header byte
- checksum repair
- SRAM signature handling

### Version and branding updates

- injector version updated to `2.40`
- runtime splash/version updated to `2.40`
- project link updated to:
  [https://github.com/VisionaiR3D/GBAGI](https://github.com/VisionaiR3D/GBAGI)

## Main Components

- `romgui/`
  Windows GUI injector and vocabulary editor
- `gbarom/`
  CLI packer
- `parse.c`
  runtime input picker and parser behavior
- `gbagi_vocab_analyzer.html`
  standalone analyzer and walkthrough helper

## Typical Workflow

### Build and edit a game

1. Open `gbinject.exe`
2. Select:
   - runtime binary
   - vocabulary data
   - output ROM
3. Add a game folder
4. Open `Vocabulary...`
5. Adjust the per-game vocabulary
6. Build the ROM

### Use the analyzer

1. Build or select a game in the injector
2. Click `Walkthrough Analyzer`
3. Review vocabulary groups and walkthrough compatibility
4. Export `gbagi_vocab_preset.tsv` or click `Update Injector`

## Notes

- The source tree contains both GUI and CLI packers. The GUI path has received the most vocabulary-editor integration work.
- The vocab editor preview is a planning aid. It does not reproduce every room-specific runtime condition exactly, but it reflects the current effective plan well enough for editing.
- The analyzer export format is compatible with the injector's preset loader.

## Credits

Based on the original GBAGI work by Davidebyzero and Brian Provinciano's AGI interpreter foundations, with additional injector, parser, analyzer, and workflow changes in this fork.

![1000043481](https://github.com/user-attachments/assets/7e19964d-0024-4059-8525-a03e4b72793d)
![1000043438](https://github.com/user-attachments/assets/abe88bf2-57c9-41fe-b15e-2a4c59b3b72d)
<img width="2340" height="1080" alt="1000043505" src="https://github.com/user-attachments/assets/37081562-1a75-4b1c-a584-3b8b30e5fc39" />














