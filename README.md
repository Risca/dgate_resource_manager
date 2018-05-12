# dgate_resource_manager
Graphical tool to explore data files from the Legend Entertainment game Death Gate

## Building
Only tested on Sabayon Linux. Building on other platforms and distros is currently out of scope.

### Prerequisites
Tested and working with
* Qt-5.7.1 (Qt-4.8.7 hanged for me when playing wav)
* WildMIDI >= 0.4.0 (for XMIDI support)

### Getting the sources
Clone the repo and submodules
```
git clone https://github.com/Risca/dgate_resource_manager.git
cd dgate_resource_manager
git submodule update --init
```

### Compiling
This program requires a 3rd party library for playing FLIC animations
```
pushd 3pp/flic/
cmake .
make
popd
```
I like to build in a separate build directory
```
mkdir ../dgate_resource_manager-build
cd ../dgate_resource_manager-build
qmake ../dgate_resource_manager
make
```

## Running

The first time the program is run there will be nothing in the ui.
Use the meny or <kbd>CTRL</kbd>+<kbd>O</kbd> to point out where your death gate files are located.
Once a folder is selected, it should populate all tabs with data.

Double-clicking on any row will try to present the media in some way.
The exception is the *Text* tab, where the text is already presented.
If no music is heard when double-clicking a music item, please check your WildMIDI installation.
Keep in mind that the program hanged for me if I compiled it with Qt-4.8.7 and tried to play any sound.

### Getting sound fonts for wildmidi
Okay, so after reinstalling my laptop, I had to figure out how to get wildmidi to work again.
The default installation (on Arch at least) does not provide any sound fonts for wildmidi.
You can install the timidity-freepats package, but it only came with sound fonts in sf2 format.
I used [unsf](https://github.com/psi29a/unsf) to convert the timidity-freepats sound fonts to GUS patches:
```
make -f Makefile.linux
sudo ./unsf -O /usr/share/soundfonts/ /usr/share/soundfonts/timidity-freepats.sf2
sudo cp /usr/share/soundfonts/timidity-freepats.cfg /etc/wildmidi/wildmidi.cfg
sudo sed -i '1s;^;dir /usr/share/soundfonts\n;' /etc/wildmidi/wildmidi.cfg
```
After this, I was able to play (x)midi files with wildmidi.
