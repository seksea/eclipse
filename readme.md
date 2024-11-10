<p align="center">
  <a align="center" href="https://www.youtube.com/watch?v=gXosAXXt2UM"><img src="http://i.ytimg.com/vi/gXosAXXt2UM/hqdefault.jpg" alt="Youtube Video"></a>
  <p align="center">Previously private gameplay enhancer for Counter-Strike: Global Offensive (csgo_legacy) on Linux.</p>
  <div align="center">
    <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-purple.svg" alt="MIT License"></a>
    <a><img src="https://img.shields.io/badge/Platform-Linux-purple.svg" alt="Linux"></a>
    <a><img src="https://img.shields.io/badge/Game-csgo_legacy-purple.svg" alt="CS:GO"></a>
    <a><img src="https://img.shields.io/badge/Detection-Undetected-purple.svg" alt="Detection: Undetected"></a>
    <a><img src="https://img.shields.io/badge/State-Finished-Green.svg" alt="Finished"></a>
    <a href="https://gitter.im/gamesneeze/community?utm_source=share-link&utm_medium=link&utm_campaign=share-link"><img src="https://img.shields.io/badge/Chat-Gitter-green.svg" alt="Gitter"></a>
    <a href="https://discord.gg/edT3MNztx5"><img src="https://img.shields.io/badge/Chat-Discord-blue.svg" alt="Discord"></a>
  </div>
</p>


## Table of Contents
- [Table of Contents](#table-of-contents)
- [Features](#features)
- [Usage](#usage)
- [toolbox.sh help](#toolboxsh-help)
- [Common Issues](#common-issues)
  - [Successfuly building but having issues injecting?](#successfuly-building-but-having-issues-injecting)
- [Contributing to the Project](#contributing-to-the-project)
- [Donations](#donations)
  - [Donate](#donate)

> [!NOTE]
> I have not tested this at all and I am unsure if it even still works on the current `csgo_legacy`

## Features

### Visuals
 - Chams
 - Player ESP
 - Glow
 - Nightmode
 - Skybox Changer

### Movement

 - Just bhop in the stock cheat
 - Good [movement luas](https://github.com/seksea/eclipse-luas/blob/master/advancededgebug.lua) $$

### Misc

 - Discord Game SDK
 - Clantag Changer

### Lua

 - Shitty Lua docs can be found [here](https://seksea.github.io/luaDocs/), you can use [this file](https://github.com/seksea/eclipse-luas/blob/master/lib/eclipse/library/eclipse.lua) to tell your IDE about the lua API. And the best way to learn is to look at all of the public example lua scripts [here](https://github.com/seksea/eclipse-luas)
 - Here are some good example scripts:
   - [advancededgebug.lua](https://github.com/seksea/eclipse-luas/blob/master/advancededgebug.lua) - Utilises the games' movement prediction to predict future edgebugs and force whatever input is required to hit the edgebug.
   - [chickenglow.lua](https://github.com/seksea/eclipse-luas/blob/master/chickenglow.lua) - Causes all chickens to glow.
   - [deadesp.lua](https://github.com/seksea/eclipse-luas/blob/master/deadesp.lua) - Only enable the ESP when you are dead. 
   - [dlights.lua](https://github.com/seksea/eclipse-luas/blob/master/dlights.lua) - Replaces all in-game light entities with fancy dynamic lights.
   - [espeverything.lua](https://github.com/seksea/eclipse-luas/blob/master/espeverything.lua) - Render an ESP box around every single entity on the map. :o
   - [hitmarker.lua](https://github.com/seksea/eclipse-luas/blob/master/hitmarker.lua) - Fancy hitmarkers.
   - [legitbhop.lua](https://github.com/seksea/eclipse-luas/blob/master/legitbhop.lua) - A more legit bhop cheat, not constant perfs and spams +JUMP and -JUMP before and after hitting the ground, so it is "fake scrollwheel bhop".
   - [lobbytricks.lua](https://github.com/seksea/eclipse-luas/blob/master/lobbytricks.lua) - Many small lobby exploits executed via panorama JS such as: non-host queue cancel, popup spam for everyone else, and custom lobby error messages.
   - [massinvite.lua](https://github.com/seksea/eclipse-luas/blob/master/massinvite.lua) - Invites everyone from "Looking to Play" using panorama JS.
   - [material-pearlescent.lua](https://github.com/seksea/eclipse-luas/blob/master/material-pearlescent.lua) - An example of how to add a custom material to the chams system.
   - [memeesp.lua](https://github.com/seksea/eclipse-luas/blob/master/memeesp.lua) - Draw an ESP box around chicken and fish.
   - [overridenameable.lua](https://github.com/seksea/eclipse-luas/blob/master/overridenameable.lua) - Awesome nametag exploit, to use it first go to rename a weapon with a nametag, and while in the UI where you type the new name in, put a new name in the "overridenameable name" textbox in the eclipse GUI (this can include any special characters, without the normal restriction, meaning you can have newlines etc in your weapon names) and press the "rename" button, then continue to rename the weapon.
   - [pigeon.lua](https://github.com/seksea/eclipse-luas/blob/master/pigeon.lua) - Replace the character on the lobby screen with a pigeon.
   - [precacheTest.lua](https://github.com/seksea/eclipse-luas/blob/master/precacheTest.lua) - Custom model example.
   - [sand.lua](https://github.com/seksea/eclipse-luas/blob/master/sand.lua) - Falling sand simulator.
   - [trail.lua](https://github.com/seksea/eclipse-luas/blob/master/trail.lua) - Draws a pretty trail behind the player using beams
   - [translate.lua](https://github.com/seksea/eclipse-luas/blob/master/translate.lua) - Chat translator that translates to and from other languages. Sadly no way of concurrent networking in eclipse currently, so this causes the game to lock up while contacting the API :(
   - [weather.lua](https://github.com/seksea/eclipse-luas/blob/master/weather.lua) - Uses the source engines' built in CPrecipitation entity to add pretty particle rain/snow fall.
   - [420.lua](https://github.com/seksea/eclipse-luas/blob/master/420.lua) - Plays an image sequence of the "MLG Smoke Weed every day first person" video on your screen whenever a kill is achieved.

## Usage

```sh
git clone https://github.com/seksea/eclipse
cd eclipse
chmod +x toolbox.sh
./toolbox.sh -p -u -b -l # pull, unload trainer if currently injected, build, and then load. Use -h for help
```

## toolbox.sh help

| Argument           | Description                               |
| ------------------ | ----------------------------------------- |
| -u (--unload)      | Unload the trainer from CS2 if loaded.    |
| -l (--load)        | Load/inject the trainer via GDB.          |
| -ld (--load_debug) | Load/inject the trainer and debug via GDB.|
| -b (--build)       | Build to the build/ directory in release. |
| -bd (--build_debug)| Build to the build/ directory in debug.   |
| -p (--pull)        | Update the trainer                        |
| -h (--help)        | Show help                                 |

All arguments are executed in the order they are written in, for example, `./toolbox.sh -u -b -l` would unload the trainer (if loaded), build it, and then load it back into CS2.


## Common Issues

### Successfuly building but having issues injecting?

If Steam is installed as a Snap or Flatpak, you can use [CoreInject](https://github.com/Sumandora/CoreInject) with `--workaroundSandboxes` to inject the trainer.

Example:

```
sudo ./CoreCLI -p $(pidof cs2) -m /path/to/libeclipse.so --workaroundSandboxes --overwriteRelocationTarget --deleteAfterInjection
```

Alternatively, you can uninstall the Snap or Flatpak version of Steam with
`snap uninstall steam` or
`flatpak uninstall steam` and then reinstall it through your **real** package manager (`pacman`/`apt`/etc)

## Contributing to the Project

Feel free to report a bug to the issues page, or contribute to the project via a pull request.

 - [Create a pull request](https://github.com/seksea/eclipse/compare) ([tutorial](https://github.com/yangsu/pull-request-tutorial))
 - [Report a bug](https://github.com/seksea/eclipse/issues/new)


## Donations

### Donate

Donations are accepted in BTC, Monero, and via Buy Me a Coffee.

> [!IMPORTANT]
> If sending more than 5 USD contact me on Discord (`sekc1337`) letting me know, and I will add you to the credits as a donor in the readme and the menu, along with the amount donated and some custom text (up to 128 chars, and deemed not offensive).

```
BTC:
bc1qwrqhhlam4rl7yh2a09ntgdduw3vg9er3ce8rjq

Monero:
86xJi2jQEocYZ7o6BTrbmTPJKLxbfHAdKdPcTgro3PAw6z3MCcYLq28Ehg4tzRUCPFKDQM1SKbp4RRygnk9FCBux3uxXCDN

Buy Me a Coffee:
www.buymeacoffee.com/sekc
```
