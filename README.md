# gxtuner
A simple (linux) guitar  tuner for jack 

![GxTuner](https://github.com/brummer10/gxtuner/raw/master/GxTuner.png)


###### BUILD DEPENDENCY’S 

the following packages are needed to build gxtuner :

- libc6-dev
- libcairo2-dev
- libfftw3-3-dev
- libgcc1-dev
- libglib2.0-0-dev
- libgtk3.0-0-dev
- libstdc++6-dev
- libzita-resampler0-dev
- libjack-jackd2-0-dev or libjack-0.116-dev

note that those packages could have different, but similar names 
on different distributions. There is no configure script, 
make will simply fail when one of those packages isn't found.

to build gxtuner with jack_session support simply run
$ make

to build gxtuner without jack_session support run 
$ make nosession

in the source directory.

If you wish to install[1] gxtuner run
$ make install

to uninstall gxtuner run
$ make uninstall

to build a Debian package, run 
$ make deb

you can run gxtuner from any location you choose without installation.
[1] but to work propper with jack_session manager
you need to install it