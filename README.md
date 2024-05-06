```bash
sudo apt install build-essential
sudo apt install git
sudo snap install --classic code
sudo snap install cmake --classic
```


## Install bake
```bash
cd ~/Downloads
git clone https://github.com/SanderMertens/bake
bake/setup.sh
```


## Build flecs
```bash
git clone https://github.com/SanderMertens/flecs
bake flecs
```


## Build eg
```bash
cd ~/Documents
git clone https://github.com/KenthJohan/eg
```

### Build vendor/*
```bash
bake vendor
```

```bash
fatal error: GL/gl.h: No such file or directory

sudo apt install libgl1-mesa-dev
```

```bash
fatal error: X11/extensions/XInput2.h: No such file or directory

sudo apt install libxi-dev
```

```bash
fatal error: X11/Xcursor/Xcursor.h: No such file or directory

sudo apt install libxcursor-dev
```

```bash
fatal error: GL/glew.h: No such file or directory

sudo apt-get install libglew-dev
```


## Build libwebsockets
```bash
sudo apt-get install libssl-dev
cd ~/Downloads
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets/
mkdir build
cd build
cmake ..
make
sudo make install
```


## Build SDL2
```bash
git clone https://github.com/libsdl-org/SDL.git -b SDL2
cd SDL
mkdir build
cd build
../configure
make
sudo make install
```