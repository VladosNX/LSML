# Linux Subsystem Manager for Linux

Version v0.1-BETA

## 📝 What is this?

LSML Subsystem is a simple tool to download and start AMD64 (x86-64) Linux Subsystem in an existing Linux OS. In this version you can use:

- Arch Linux
- Debian 12
- Ubuntu 24.10
- Alpine Linux

These systems are stored in [RootFS Repository](https://github.com/VladosNX/lsml-rootfs).

## ⚒ Building from sources

Dependencies for building:

- Git *(to download sources)*
- Meson *(to prepare and configure build)*
- Ninja *(to build and instsall)*
- libcurl *(to download rootfs)*
- libarchive *(to unpack packages downloaded from RootFS repository)*
- ncurses *(to make a beautiful user interface)*

Installation for Arch Linux:

`sudo pacman -S git meson ninja curl ncurses libarchive`

Installation for Debian:

`sudo apt install git meson ninja-build curl libncurses-dev libarchive-dev`

### Building

```bash
git clone https://github.com/VladosNX/lsml
cd lsml
mkdir build && cd build
meson setup ..
ninja
sudo ninja install
```

Try to start it: type `lsml` to your bash!

## 🐞 Bug Reporting

If you found a bug, please - [make an issue](https://github.com/VladosNX/LSML/issues). Try to describe your problem in details and your bug will be fixed!

## 🤝 Contributing

Thanks for reading this section! If you want to help LSML Subsystem, you can **contribute**. Before you sending Pull Request, please follow these simple rules:

1. Name the branch clearly
2. Describe your pull request in details
3. Don't place lots of new features into one pull request - break it into a little pieces
