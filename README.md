# gxtuner
A (linux) tuner for jack, with full jack session management support

![GxTuner](https://github.com/brummer10/gxtuner/raw/master/GxTuner.png)

Besides a regular tuning option, it's possible to use GXtuner for extended Just Intonation.

A4 = 440 Hz 

A4 reference pitch can adjusted at command line and/or runtime
in a half tone range: 200Hz <-> 600Hz

gxtuner uses a default threshold level at 0.001.
The threshold can be adjusted at command line and/or runtime
in a range of 0.001 <-> 0.2

### COMMANDLINE OPTIONS

```
Help Options:
  -h, --help                    Show help options
  --help-all                    Show all help options
  --help-gtk                    GTK configuration options
  --help-jack                   JACK configuration options
  --help-engine                 ENGINE configuration options

GTK configuration options
  -x, --posx=POSITION_X         window position x-axis ( -x 1 . . .)
  -y, --posy=POSITION_Y         window position y-axis ( -y 1 . . .)
  -w, --wigth=WIDTH             'default' width ( -w 500 . . .)
  -l, --height=HEIGHT           'default' height ( -l 300 . . .)
  -d, --desktop=NUM             set to virtual desktop num ( -d 0 . . .)
  -N doremi                     start with Latin notation

JACK configuration options
  -i, --jack-input=PORT         connect to JACK port name 
                                    (-i system:capture_1)

ENGINE configuration options
  -p, --pitch=PITCH             set reference pitch (-p 200.0 <-> 600.0)
  -t, --threshold=THRESHOLD     set threshold level (-t 0.001 <-> 0.2)
```

All settings are optional, they will be all restored by the jack session manager

### BUILD DEPENDENCIES

The following packages are needed to build gxtuner:

- libc6-dev
- libcairo2-dev
- libfftw3-3-dev
- libgcc1-dev
- libglib2.0-0-dev
- libgtk3.0-0-dev
- libstdc++6-dev
- libzita-resampler0-dev
- libjack-jackd2-0-dev or libjack-dev (>= 0.116)

**Note** above packages could have different, yet similar names 
on different distributions. There is no configure script. 
`make` will simply fail when one of those packages isn't found.

To build gxtuner with jack_session support simply run
```bash
$ make
```
To build gxtuner **without jack_session support** run the following in the source directory

```bash
$ make nosession
```

If you wish to install[1] gxtuner
```bash
$ make install
```

To uninstall gxtuner
```bash
$ make uninstall
```

To build a Debian package
```bash
$ make deb
```

**Note:** one can run gxtuner from any location onc chooses without installation.

[1] but to work proper with jack_session manager
you need to install it

### LICENSE

GPLv2 ([LICENSE](LICENSE))
