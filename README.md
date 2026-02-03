# About : 
**_Blaster_** is a 3D multiplayer fast-paced shooter game developed in Unreal Engine 5, built primarily in C++ with supporting Blueprints.

It uses Unreal Engine’s Authoritative Client-Server model and was developed while following Stephen Ulibarri’s Udemy course "UE5 C++ Multiplayer Shooter".

It features custom multiplayer systems (lag compensation, client-side prediction, server-side rewind, cheating prevention) along with full gameplay implementation: different weapons, complex characters animations, various game modes, and UI.

https://github.com/user-attachments/assets/48d607d5-0da4-4f6b-8945-6a2bccedb21b

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

- **3 fully playable game modes**: Free For All, Teams, and Capture the Flag, each with its own rules and match flow.
- **7 unique weapons**, featuring distinct behaviors such as hitscan (Sniper, Shotgun, Pistol), projectile-based logic (AR, Rocket Launcher), and throwable grenades.
- **Complete weapon system** including firing, reloading, aiming, recoil, crosshair behavior, ammo management, and weapon swapping.
- **Pickups** for health, armor, temporary movement power-ups (speed / jump height), and ammunitions.
- **Characters fully animated** from scratch, using IK, Blend Spaces, and Animation Montages for responsive, layered animation states that adapt smoothly to player actions and networked gameplay.
- **UI/GUI** including Start Menu for Multiplayer sessions management, and in-game HUD with replicated game state indicators.
- **Multiplayer-ready gameplay design**, ensuring all core systems (weapons, movement, abilities, scoring, pickups) behave consistently across clients and server.

https://github.com/user-attachments/assets/199ca9f0-3054-401c-8a0e-1fa123093888

# Implemented Multiplayer Features : 
- **Custom Multiplayer Plugin** integrating Steam Online Subsystem: Hosting/Joining sessions online, with associated Lobby level and session travel for all game modes.

- **Lag Detection** including delay detection, and warnings.

- **Full Replication Layer** for weapons, characters, gameplay states, scoring, match flow, and UI elements, to try and ensure consistent experience across clients.

- **Cheat Mitigation** through server-side data validation (weapon fire rate, movement constraints, etc...) to prevent modified client behavior.
  
- **Client-Side Prediction** to prevent perceived delay on the Client side for visualizing replicated variables/actions like firing, ammo updates, aiming state, movement responsiveness, and animation triggers.

- **Server-Side Rewind** to try and compensate for possible Client lag when shooting a target :

--> On the Server every frame we keep track of all players with approximative BoxCollision objects, and keep the saved info for a couple of seconds.

https://github.com/user-attachments/assets/dde64db3-d37b-41e0-b137-26b2697e0dcc

--> Then, if on the Client the player’s character has shot somebody, the Server will check if at the exact time of the shot being fired the presumably shot character was in fact really on the path of the bullet. If yes the Client shot is registered on the Server ( and sent to everyone else ), else it is ignored.

https://github.com/user-attachments/assets/4236a1c3-cc3f-4d80-9519-86caa97252e3

This feature is implemented for every type of weapon in the game (Sniper Rifle, Shotgun, AR Rifle, SMG, etc ...) and can be disabled in the Weapon Blueprint or via code if needed.

## Installation : 

Unreal Engine version : 5.4

## Asset Used :

- Animation Starter Pack - by Epic Games :

https://www.fab.com/listings/98ff449d-79db-4f54-9303-75486c4fb9d9

- Unreal Learning Kit: Games - by Epic Education Learning and Training :

https://www.unrealengine.com/marketplace/en-US/product/unreal-learning-kit-games

- Military Weapon Pack - by Project89 :

https://www.unrealengine.com/marketplace/en-US/product/weapon-pack

- Paragon: Wraith et Paragon: Dekker- by Epic Games :

https://www.fab.com/listings/17e0840f-8651-4933-bd6d-211a34b1dd17
https://www.fab.com/listings/6cc4f913-db56-44cc-9a42-6aeeeb147c79

- And various Mixamo animations :

https://www.mixamo.com/
