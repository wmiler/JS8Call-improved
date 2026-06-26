In the tools directory there are the following shell scripts for linux:

- Debian_build-deb.sh is an experimental script that builds JS8Call and produces a Debian-style .deb installation file

- Linux-TestBuild-Qt6.sh is primarily for building Qt6 with the required modules for JS8Call from source code. By setting the QT_VERSION variable in the script you can build whatever version of Qt you want to test or experiment with

- Linux-User-Build-JS8Call.sh is an end-user script that will build and install JS8Call for Debian, Redhat/Fedora and Arch Linux systems for either x86_64 or arm64 architectures. This script should also work for Raspberry Pi 5

- tcp.py is a python script that connects to a server running on your own computer (127.0.0.1) at port 2442 to diagnose the JS8Call API for automated digital logging. Once connected, it automatically asks for the station's status, listens continuously for incoming messages from the server, prints those messages out, and ignores minor status updates it doesn't care about

- udp.py is the same thing as above, except using the UDP protocol instead of TCP

- tracking_diag.cpp is a source file that builds a diagnostic harness for JS8 frequency/timing tracking

- whitening_diag.cpp is a source file that builds a commandline harness for JS8 whitening/noise estimation