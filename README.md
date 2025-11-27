# About : 
**_Blaster_** is a 3D multiplayer fast-paced shooter game developed on Unreal Engine 5, using Unreal Engine's Authoritative Client-Server model.
Developed while following Stephen Ulibarri’s Udemy course "UE5 C++ Multiplayer Shooter".

# How to play : 
- Download project via this repo.
- Package the project.
- Make sure Steam is opened on your PC and you are logged into your account, as Steam is used as the Online Subsystem to connect players in the game.
- Launch the Blaster.exe file from the packaged project folder.
- Host/Join a session with a friend ! ( After creating the session, you’ll wait in a Lobby while at least 1 other player joins).
- That's it ! enjoy :) 

# Current Features : 
- 3 different game modes possible : Free For All, Teams, and Capture the Flag !
- 7 different weapons with different logics such as Hitscan weapons ( Sniper Rifle, Shotgun, ... ), Projectile weapons ( AR Rifle, Rocket Launcher, ... ) and Grenades.
- Pickups for health/armor and speed/jump buffs.
- Characters fully animated from scratch.

# Implemented Multiplayer Features : 
- **Multiplayer Plugin** using Steam Online Subsytem : Hosting/Joining sessions online, with associated Lobby level and corresponding logic.

- Lag Detection and associated Warnings and Lag Compensation Techniques

- Cheating Prevention with data validation on the Server ( checking infos like the client weapon’s fire rate )

- **Client-Side Prediction** : 
To prevent delay on the Client side for visualizing protected variables/actions like Ammo decreasing when firing, reloading and going on/off aiming state.

//video shooting with high lag from Client perspective

- **Server-Side Rewind** : 
To try and compensate for possible Client lag when shooting a target.
On the Server every frame we keep track of all players with approximative BoxCollision objects, and keep the saved info for a couple of seconds.

https://github.com/user-attachments/assets/dde64db3-d37b-41e0-b137-26b2697e0dcc

Then, if on the Client the player’s character has shot somebody, the Server will check if at the exact time of the shot being fired the presumably shot character was in fact really on the path of the bullet. If yes the Client shot is registered on the Server ( and sent to everyone else ), else it is ignored.

https://github.com/user-attachments/assets/4236a1c3-cc3f-4d80-9519-86caa97252e3

## Installation : 

Unreal Engine version : 5.4
