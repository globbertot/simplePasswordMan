## NOTE: THIS IS AN EARLY PREVIEW WHICH IS SUBJECTED TO CONTAIN BUGS AND WILL NOT BE THE FINALIZED VERSION ##
# Simple Password Manager (Qt Version)

## Overview
This is a simple password manager written in C++ with the use of the Qt framework. It is a port of the original terminal-based password manager, which you can find [here](https://github.com/globbertot/simple-password-manager).

## Libraries Used
- Qt
- OpenSSL

## Features in Progress
- Settings Page (more settings)
- Better Error Handling
- Improving the source code (and readability)

## Known Issues
- **SOMETIMES**, when updating from V0.2 to V0.3, your old theme was still a thing and is found on the theme picker (FIX: delete that theme and select any other theme and this is fixed)

## Download Instructions
### Windows
Installing on Windows is straightforward. Follow these steps:

1. Go to the [releases page](https://github.com/globbertot/simplePasswordMan/releases).
2. Download the latest version for your windows machine. (note that if a pop-up shows up saying this file is not commonly downloaded/is mallicous, try either right clicking and pressing keep or look up a tutorial on how to keep the file)
3. Run the installer, and go through the process (once again, if smartscreen pops up, clicking More Info and run anyway should fix the problem)
4. Run the shortcut on your desktop (as admin), and you're ready to use the password manager.

### Linux
Installing on linux is also easy, Follow these steps:

1. Go to the [releases page](https://github.com/globbertot/simplePasswordMan/releases).
2. Download the latest version for your linux machine instead.
3. Run the application and you are ready to use the password manager
4. If you happen to have an error message when starting up saying going something like "SSL handshake failed: the issuer certificate of a locally looked up certificate could not be found", you should run this command: `sudo apt-get update && sudo apt-get install --reinstall ca-certificates`
