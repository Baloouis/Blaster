# About : 
**_Blaster_** is a 3D multiplayer fast-paced shooter game developed in Unreal Engine 5, built primarily in C++ with supporting Blueprints.

It uses Unreal Engine’s Authoritative Client-Server model and was developed while following Stephen Ulibarri’s Udemy course "UE5 C++ Multiplayer Shooter".

It features custom C++ multiplayer systems (lag compensation, prediction, rewind, cheating prevention) along with full gameplay implementation: different weapons, complex characters animations, various game modes, and UI.

# How to play : 

## Preview locally
### Previewing the game locally in the UE Editor
- Download project via this repo.
- Open the corresponding UE project.
- In the Content Browser go to Content/Maps/ and select one of the following Maps : FreeForAllMap_Update / TeamsMap_Update / CaptureTheFlagMap_Update.
- In the editor go to "Play Settings", set "Net Mode" to "Play as Listen Server" and set "Number of Players" to a number between 1 and 4.
- Press Play
## Play Online
### Packaging the game
- Download project via this repo.
- Open the corresponding UE project.
- Package the project inside UE via Platforms>Windows>Package Project.
### Playing from a given build
- Make sure Steam is opened on your PC and you are logged into your account, as Steam is used as the Online Subsystem to connect players in the game.
- Launch the Blaster.exe file from the packaged project folder.
- Host/Join a session with a friend ! ( After creating the session, you’ll wait in a Lobby while at least 1 other player joins).
- That's it ! enjoy :) 

# Current Features : 
- 3 different game modes possible : Free For All, Teams, and Capture the Flag !
- 7 different weapons with different logics such as Hitscan weapons ( Sniper Rifle, Shotgun, Pistol, ... ), Projectile weapons ( AR Rifle, Rocket Launcher, ... ) and Grenades.
- Pickups for health/armor and speed/jump buffs.
- Characters fully animated from scratch using IK, Blend Spaces, and Animation Montages to achieve responsive, layered animations that adapt smoothly to player actions and networked gameplay.
- Start Menu with Multiplayer sessions management between players for every game mode.
  
# Implemented Multiplayer Features : 
- **Multiplayer Plugin** using Steam Online Subsytem : Hosting/Joining sessions online, with associated Lobby level and corresponding logic.

- Lag Detection and associated Warnings and Lag Compensation Techniques

- Cheating Prevention with data validation on the Server ( checking infos like the client weapon’s fire rate )

- **Client-Side Prediction** : 
To prevent delay on the Client side for visualizing protected variables/actions like Ammo decreasing when firing, reloading animations starting locally before Server response, and going on/off aiming state.

- **Server-Side Rewind** : 
To try and compensate for possible Client lag when shooting a target.
On the Server every frame we keep track of all players with approximative BoxCollision objects, and keep the saved info for a couple of seconds.

https://github.com/user-attachments/assets/dde64db3-d37b-41e0-b137-26b2697e0dcc

Then, if on the Client the player’s character has shot somebody, the Server will check if at the exact time of the shot being fired the presumably shot character was in fact really on the path of the bullet. If yes the Client shot is registered on the Server ( and sent to everyone else ), else it is ignored.

https://github.com/user-attachments/assets/4236a1c3-cc3f-4d80-9519-86caa97252e3

This feature is implemented for every type of weapon in the game (Sniper Rifle, Shotgun, AR Rifle, SMG, etc ...) and can be disabled in the Weapon Blueprint or via code if needed.

## Installation : 

Unreal Engine version : 5.4
