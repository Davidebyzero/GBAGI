![mockup](https://github.com/user-attachments/assets/4aa2ce8b-7da4-47d4-844d-2bf692cea799)



This repo fixes a game-breaking issue in Leisure Suit Larry 1 on GBAGI where the Quikimart phone sequence didn’t work correctly on GBA.

🧩 The Issue
On GBA, the Quikimart sequence didn’t work the way it should.
You could dial the number, but after that the game started to break:
Honeymoon suite was never accepted. 
Previously entered numbers can reappeared. 
One wrong attempt can break all future tries

👉 Result: the puzzle can become unwinnable

🛠️ The Fix
This patch applies a small, focused workaround so the sequence works again.
Detects LSL1 via game ID
Runs only in the Quikimart phone scene
Automatically provides required inputs:
wine
honeymoon suite
Fixes stale/reused phone input
Fixes repeated-dial rejection

🎯 Scope
Area
Impact
LSL1

✅ Fixed
Other AGI games
➖ Unaffected
Engine changes
⚙ Minimal

🚀 Usage
Bash
Build GBAGI with this patch
Run LSL1
Use the Quikimart phone
The sequence should now complete reliably.

⚠️ Notes
This is a targeted workaround, not a full parser fix
Designed to restore playability without affecting other games
🔧 Background
Based on:
GBAGI (Davidebyzero)
Brian Provinciano’s AGI interpreter

💡 Future Improvements
More robust get.string handling
Improved virtual keyboard input
General solution for parser-heavy scenes

![1000043481](https://github.com/user-attachments/assets/7e19964d-0024-4059-8525-a03e4b72793d)
![1000043435](https://github.com/user-attachments/assets/da8a3ab6-f16c-466f-8dba-8003cb852b2c)
![1000043438](https://github.com/user-attachments/assets/abe88bf2-57c9-41fe-b15e-2a4c59b3b72d)
<img width="2340" height="1080" alt="1000043505" src="https://github.com/user-attachments/assets/37081562-1a75-4b1c-a584-3b8b30e5fc39" />




