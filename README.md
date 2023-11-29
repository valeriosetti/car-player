# What is this all about?

Car multimedia player based on OdroidC2 board running Buildroot.

# How to build

Ensure Docker is installed on your PC, then just issue:

```
cd Docker
./build.sh
./launch.sh "make car_player_defconfig all"
```

If the build completes successfully all artifacts will be placed in `output/images/`

# Prebuilt images

There is a GitHub workflow which runs at every pull request or manually for this repo. Artifacts can be found in `Actions->Build images` workflow, after selecting the desired run.

# Some technical detail

## Kernel

It's a customized version of the upstream `linux-amlogic`:

- sources can be found [here](https://github.com/valeriosetti/linux-amlogic-v6.1)
- it contains:
  - some customization for this board at DT level
  - I2S input driver (which is missing from the mainline version)
 
# Other (TO DO)
LVGL, etc
