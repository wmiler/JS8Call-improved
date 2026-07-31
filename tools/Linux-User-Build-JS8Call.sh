#!/bin/bash
set -e

# (C) copyright 2025 Chris Olson AC9KH, Joseph Counsil K0OG
# Modded 2026 to prevent KDE system crashes via strict /opt path isolation.
# Builds and installs JS8Call from source on a linux system.
# Requires sudo access. Installs to /opt/lib/js8call and /opt/bin.

# --- Variables ---
red="\033[0;31m"
clear_color="\033[0m"
JS8_VERSION="master"

# New Isolated Safe Paths
JS8_INSTALL_PREFIX="/opt/lib/js8call"
JS8_BIN_DIR="/opt/bin"
JS8_QT_DIR="${JS8_INSTALL_PREFIX}/Qt"

# Toxic Legacy Paths to detect and safely nuke
TOXIC_SYSTEM_BIN="/usr/bin/JS8Call"
TOXIC_SYSTEM_LIB="/usr/lib/js8call"
TOXIC_LDCONFIG="/etc/ld.so.conf.d/js8call.conf"

# Legacy local install paths — used to detect and remove old ~/.local installs
LEGACY_BIN="$HOME/.local/bin/JS8Call"
LEGACY_QT_DIR="$HOME/.local/lib/Qt"
LEGACY_LIB_DIR="$HOME/.local/lib/js8lib"
LEGACY_DESKTOP="$HOME/.local/share/applications/JS8Call.desktop"
LEGACY_ICON="$HOME/.local/share/icons/icon_128.svg"

# --- Functions ---
divider() {
  echo "######################################################################"
}

error() {
  echo "Installation has exited with an error."
  exit 1
}

clear

# --- Must not run as root ---
if [ "$(id -u)" -eq 0 ]; then
  divider
  echo -e "${red}You are logged in as root.

You MUST be logged in as a normal user to run this script.

It is recommended to add your username to the sudo group. We need to
install system libraries that are required to build JS8Call. This must
be done as a sudo user.

This script will NOT RUN AS ROOT!${clear_color}"
  divider
  exit 1
fi

# --- Detect distro ---
if [ -f /etc/os-release ]; then
  . /etc/os-release
  DISTRO=$ID
else
  echo "Cannot detect Linux distribution. Exiting."
  exit 1
fi

# --- Check for .deb managed installation ---
if dpkg -l js8call 2>/dev/null | grep -q "^ii"; then
  divider
  echo -e "${red}JS8Call is currently installed via the .deb package.

Running this script will overwrite the package-managed installation
and may cause conflicts with your package manager. It is strongly
recommended to remove the .deb package first:

  sudo dpkg -r js8call

This ensures a clean installation without package manager conflicts.${clear_color}"
  divider
  read -p "Remove the .deb package now and continue? Yes(y) / No(n): " REMOVE_DEB </dev/tty
  if [ "${REMOVE_DEB}" = "y" ]; then
    sudo dpkg -r js8call
    divider
    echo "Package removed. Continuing with build installation..."
    divider
    sleep 2
  else
    echo "Please remove the .deb package manually and re-run this script."
    exit 0
  fi
fi

install_deps() {
  if [[ "$DISTRO" == "fedora" || "$DISTRO" == "rhel" || \
        "$DISTRO" == "centos" || "$DISTRO" == "rocky" || \
        "$DISTRO" == "almalinux" ]]; then
    echo "Detected Red Hat-based system ($DISTRO), using dnf..."
    sudo dnf install -y \
      cmake ninja-build gcc-c++ perl python3 git wget file \
      openssl-devel fontconfig-devel freetype-devel \
      harfbuzz-devel libjpeg-turbo-devel libpng-devel \
      zlib-devel brotli-devel dbus-devel glib2-devel \
      at-spi2-core-devel mesa-libGL-devel mesa-libEGL-devel \
      mesa-libgbm-devel libdrm-devel libinput-devel \
      vulkan-loader-devel \
      libopengl-devel \
      libxkbcommon-devel libxkbcommon-x11-devel \
      xcb-util-devel xcb-util-image-devel xcb-util-keysyms-devel \
      xcb-util-renderutil-devel xcb-util-wm-devel xcb-util-cursor-devel \
      libXrender-devel libXi-devel \
      pulseaudio-libs-devel alsa-lib-devel \
      gstreamer1-devel gstreamer1-plugins-base-devel \
      wayland-devel wayland-protocols-devel

  elif [[ "$DISTRO" == "ubuntu" || "$DISTRO" == "debian" || \
          "$DISTRO" == "linuxmint" || "$DISTRO" == "pop" ]]; then
    echo "Detected Debian-based system ($DISTRO), using apt..."
    sudo apt-get update
    sudo apt-get install -y --ignore-missing \
      build-essential file wget git cmake ninja-build perl python3 \
      libssl-dev libfontconfig1-dev libfreetype-dev \
      libharfbuzz-dev libjpeg-dev libpng-dev \
      zlib1g-dev libbrotli-dev libdbus-1-dev libglib2.0-dev \
      libatspi2.0-dev libgl-dev libegl-dev libgbm-dev \
      libopengl-dev \
      libdrm-dev libinput-dev libvulkan-dev \
      mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev \
      libxkbcommon-dev libxkbcommon-x11-dev \
      libxcb-util-dev libxcb-image0-dev libxcb-keysyms1-dev \
      libxcb-render-util0-dev libxcb-icccm4-dev libxcb-cursor-dev \
      libxrender-dev libxi-dev \
      libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
      libwayland-dev wayland-protocols

    # Handle libasound rename in Ubuntu 24+
    sudo apt-get install -y libasound2-dev || sudo apt-get install -y libasound2t64

  elif [[ "$DISTRO" == "arch" || "$DISTRO" == "manjaro" || \
          "$DISTRO" == "endeavouros" || "$DISTRO" == "garuda" ]]; then
    echo "Detected Arch-based system ($DISTRO), using pacman..."
    sudo pacman -Syu --noconfirm \
      cmake ninja gcc perl python git wget \
      openssl fontconfig freetype2 \
      harfbuzz libjpeg-turbo libpng \
      zlib brotli dbus glib2 \
      at-spi2-core mesa libglvnd \
      libdrm libinput \
      vulkan-icd-loader \
      libxkbcommon \
      xcb-util xcb-util-image xcb-util-keysyms \
      xcb-util-renderutil xcb-util-wm xcb-util-cursor \
      libxrender libxi \
      libpulse alsa-lib \
      gst-plugins-base \
      wayland wayland-protocols \
      libxrandr libxext libxfixes \
      libxcb libxshmfence \
      libx11 \
      yasm nasm \
      clang llvm

  else
    echo "Unsupported distribution: $DISTRO"
    echo "Please install dependencies manually and re-run."
    exit 1
  fi
}

# --- Check for legacy ~/.local installation and offer to remove it ---
clear
divider
echo "Checking for legacy local installations....."
if [ -f "${LEGACY_BIN}" ]; then
  divider
  echo -e "${red}A legacy JS8Call installation was found in ~/.local.
The legacy installation should be removed to prevent workspace bugs.${clear_color}"
  divider
  read -p "Remove legacy ~/.local installation? Yes(y) / No(n): " REMOVE_LEGACY </dev/tty
  if [ "${REMOVE_LEGACY}" = "y" ]; then
    echo "Removing legacy JS8Call binary..."
    rm -f "${LEGACY_BIN}"
    echo "Removing legacy Qt..."
    rm -rf "${LEGACY_QT_DIR}"
    echo "Removing legacy libraries..."
    rm -rf "${LEGACY_LIB_DIR}"
    echo "Removing legacy desktop entry..."
    rm -f "${LEGACY_DESKTOP}"
    echo "Removing legacy icon..."
    rm -f "${LEGACY_ICON}"
    divider
    echo "Legacy local installation removed."
    divider
    sleep 2
  fi
fi

# --- NEW STEP: Check and purge toxic legacy system installation that breaks KDE ---
if [ -d "${TOXIC_SYSTEM_LIB}" ] || [ -f "${TOXIC_SYSTEM_BIN}" ] || [ -f "${TOXIC_LDCONFIG}" ]; then
  divider
  echo -e "${red}CRITICAL SYSTEM NOTICE:
Legacy components were detected in ${TOXIC_SYSTEM_LIB} or ${TOXIC_SYSTEM_BIN}.
These paths leak custom Qt 6.12.0 dependencies globally, which crashes KDE Plasma!${clear_color}"
  divider
  read -p "Purge conflicting /usr system-wide paths to fix/protect KDE? Yes(y) / No(n): " PURGE_TOXIC </dev/tty
  if [ "${PURGE_TOXIC}" = "y" ]; then
    echo "Purging legacy conflicting binary..."
    sudo rm -f "${TOXIC_SYSTEM_BIN}"
    echo "Purging legacy conflicting system Qt library folder..."
    sudo rm -rf "${TOXIC_SYSTEM_LIB}"
    echo "Removing dangerous global linker configurations..."
    sudo rm -f "${TOXIC_LDCONFIG}"
    sudo ldconfig
    echo "System environment cleared and restored successfully."
    sleep 2
  fi
fi

# --- Check for existing safe /opt system installation ---
divider
echo "Checking for existing /opt installation....."
if [ -f "${JS8_BIN_DIR}/JS8Call" ] || [ -d "${JS8_INSTALL_PREFIX}" ]; then
  divider
  echo "An existing /opt JS8Call installation was found. Do you want to
uninstall it? Selecting No will overwrite it with the new version."
  read -p "Uninstall current /opt JS8Call? Yes(y) / No(n): " UNINSTALL </dev/tty
  if [ "${UNINSTALL}" = "y" ]; then
    echo "Removing JS8Call application binary wrapper..."
    sudo rm -f "${JS8_BIN_DIR}/JS8Call"
    echo "Removing isolated assets and libraries from /opt..."
    sudo rm -rf "${JS8_INSTALL_PREFIX}"
    echo "Removing desktop entry..."
    sudo rm -f /usr/share/applications/JS8Call.desktop
    echo "Removing icon..."
    sudo rm -f /usr/share/icons/hicolor/scalable/apps/js8call.svg
    divider
    echo "JS8Call has been cleanly uninstalled from /opt.
To reinstall, run this script again."
    divider
    exit 0
  fi
fi

# --- Introduction ---
clear
divider
echo -e "This script will fetch necessary sources and dependencies to build
JS8Call and install it inside an isolated container path (/opt/lib/js8call).
It requires sudo access to write to /opt/lib and establish a launch command 
wrapper inside /opt/bin/.

This execution model completely isolates the custom Qt 6.12.0 libraries, ensuring
that your KDE Desktop Environment or system applications are never corrupted."
divider
read -p "Press Enter to continue" </dev/tty

clear
echo "NOTES:
The newest versions of JS8Call require Qt v6.12.0 to run correctly. Most
linux distributions do not package Qt6.12.0. The JS8Call project provides
pre-compiled Qt6.12.0 libraries that this script will fetch and install strictly 
to /opt/lib/js8call/Qt. 

By building directly into /opt and launching via a targeted runtime wrapper,
the two Qt versions can safely coexist on the exact same disk without any friction."
divider
read -p "Press Enter to proceed to Dependency Installation..." </dev/tty

# --- Trigger dependencies ---
install_deps

# Ensure target staging folders exist under /opt
sudo mkdir -p "${JS8_INSTALL_PREFIX}"
sudo mkdir -p "${JS8_BIN_DIR}"

# --- Create development directory ---
mkdir -p "$HOME/development"

# --- Create system library directory owned by root ---
sudo mkdir -p "${JS8_INSTALL_PREFIX}"
sudo chown root:root "${JS8_INSTALL_PREFIX}"

# --- Detect architecture ---
clear
divider
JS8_ARCH=$(uname -m)
echo "System architecture: $JS8_ARCH"
divider
sleep 2

# --- Fetch Qt6 and js8lib if not already installed ---
echo "Checking for Qt 6.12.0..."
divider
sleep 2

cd "$HOME/development"

if [ ! -d "${JS8_QT_DIR}" ]; then
  if [ "${JS8_ARCH}" = "aarch64" ]; then
    wget -c https://github.com/JS8Call-improved/js8lib/releases/download/lib%2F4.0/Qt6.12.0_Linux_aarch64_pkg.tar.gz
    wget -c https://github.com/JS8Call-improved/js8lib/releases/download/lib%2F4.0/js8lib4.0-Linux_aarch64_pkg.tar.gz
    sudo tar -xzvf Qt6.12.0_Linux_aarch64_pkg.tar.gz -C "${JS8_INSTALL_PREFIX}"
    rm Qt6.12.0_Linux_aarch64_pkg.tar.gz
    sudo tar -xzvf js8lib4.0-Linux_aarch64_pkg.tar.gz -C "${JS8_INSTALL_PREFIX}" --strip-components=1
    rm js8lib4.0-Linux_aarch64_pkg.tar.gz
  else
    wget -c https://github.com/JS8Call-improved/js8lib/releases/download/lib%2F4.0/Qt6.12.0_Linux_x86_64_pkg.tar.gz
    wget -c https://github.com/JS8Call-improved/js8lib/releases/download/lib%2F4.0/js8lib4.0-Linux_x86_64_pkg.tar.gz
    sudo tar -xzvf Qt6.12.0_Linux_x86_64_pkg.tar.gz -C "${JS8_INSTALL_PREFIX}"
    rm Qt6.12.0_Linux_x86_64_pkg.tar.gz
    sudo tar -xzvf js8lib4.0-Linux_x86_64_pkg.tar.gz -C "${JS8_INSTALL_PREFIX}" --strip-components=1
    rm js8lib4.0-Linux_x86_64_pkg.tar.gz
  fi
  echo "Qt 6.12.0 and library archives extracted and removed."
  
  # REMOVED DANGEROUS SYSTEM LINKER INJECTIONS (ld.so.conf.d) TO PROTECT KDE PLASMA
  echo "Libraries isolated inside ${JS8_INSTALL_PREFIX}. Global paths untouched."

else
  echo "Qt 6.12.0 already installed — skipping download."
  divider
  sleep 2
fi

# ====================================================================
# FIX PKGCONFIG PREFIX PATHS AND INTERNAL QT BINARY PATHS ON-THE-FLY
# ====================================================================
echo "Updating isolated development configuration paths..."

# 1. Broadly fix pkgconfig files by changing any prefix matching /usr/ to /opt/lib/js8call
# This safely handles variations like prefix=/usr, prefix=/usr/lib/js8lib, or prefix=/usr/lib/js8call
if [ -d "${JS8_INSTALL_PREFIX}/lib/pkgconfig" ]; then
  for pc in "${JS8_INSTALL_PREFIX}"/lib/pkgconfig/*.pc; do
    if [ -f "$pc" ]; then
      sudo sed -i "s|^prefix=/usr.*|prefix=${JS8_INSTALL_PREFIX}|g" "$pc"
    fi
  done
fi

if [ -d "${JS8_INSTALL_PREFIX}/Qt/lib/pkgconfig" ]; then
  for pc in "${JS8_INSTALL_PREFIX}"/Qt/lib/pkgconfig/*.pc; do
    if [ -f "$pc" ]; then
      sudo sed -i "s|^prefix=/usr.*|prefix=${JS8_QT_DIR}|g" "$pc"
    fi
  done
fi

# 2. FIX QT BINARY PATH TRAP: Force Qt tools to recognize the new /opt layout
# We inject a qt.conf file into the binary directory. Qt tools look for this file
# to override their hardcoded internal cross-compilation strings.
sudo mkdir -p "${JS8_QT_DIR}/bin"
sudo tee "${JS8_QT_DIR}/bin/qt.conf" > /dev/null << EOF
[Paths]
Prefix = ${JS8_QT_DIR}
Plugins = plugins
Libraries = lib
EOF

echo "Paths fixed. Compilation links will successfully target /opt."

# --- Fetch JS8Call source ---
clear
echo "Fetching JS8Call source code..."
divider

if [ ! -d "$HOME/development/JS8Call-improved" ]; then
  cd "$HOME/development"
  git clone "https://github.com/JS8Call-improved/JS8Call-improved.git"
  cd "$HOME/development/JS8Call-improved"
  git checkout "${JS8_VERSION}"
else
  echo "Source directory already exists — checking for updates..."
  cd "$HOME/development/JS8Call-improved"
  git fetch origin
  git checkout "${JS8_VERSION}"
  git pull origin "${JS8_VERSION}"
fi
sleep 2

# --- Build JS8Call ---
cd "$HOME/development/JS8Call-improved"
BRANCH=$(git branch --show-current)

clear
divider
echo "JS8Call Build Details:
  Qt version : 6.12.0 with FFmpeg audio (requires PulseAudio or PipeWire)
  Branch     : JS8Call-improved ${BRANCH}
  Distro     : ${DISTRO} / ${JS8_ARCH}"
divider
read -p "Press Enter to continue" </dev/tty

# Remove any previous build directory to avoid stale config
cd "$HOME/development/JS8Call-improved"
rm -rf "$HOME/development/JS8Call-improved/build"
mkdir "$HOME/development/JS8Call-improved/build"
cd "$HOME/development/JS8Call-improved/build"

# Enhanced CMake to pass proper installation paths into the build tree
cmake \
  -DCMAKE_INSTALL_PREFIX="${JS8_INSTALL_PREFIX}" \
  -DCMAKE_PREFIX_PATH="${JS8_INSTALL_PREFIX};${JS8_QT_DIR}" \
  -DHAMLIB_ROOT="${JS8_INSTALL_PREFIX}" \
  ..
cmake --build . --parallel $(nproc)

# --- Install binary and desktop integration ---
# Copy raw binary to the isolated build structure rather than system paths
sudo mkdir -p "${JS8_INSTALL_PREFIX}/bin"
sudo cp JS8Call "${JS8_INSTALL_PREFIX}/bin/js8call"
sudo chmod 755 "${JS8_INSTALL_PREFIX}/bin/js8call"

# Create the Runtime Wrapper Launcher inside /opt/bin/
echo "Generating isolated runtime launch wrapper..."
sudo tee "${JS8_BIN_DIR}/JS8Call" > /dev/null << 'EOF'
#!/bin/bash
# Isolate environment strings explicitly for this binary tree process
export OPT_RUN_PREFIX="/opt/lib/js8call"
export LD_LIBRARY_PATH="$OPT_RUN_PREFIX/lib:$OPT_RUN_PREFIX/Qt/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$OPT_RUN_PREFIX/Qt/plugins"
exec "$OPT_RUN_PREFIX/bin/js8call" "$@"
EOF
sudo chmod 755 "${JS8_BIN_DIR}/JS8Call"


# ====================================================================
# NEW STAGE: CONSTRUCT THE HAM RADIO MENUS FOR KDE/FREEDESKTOP
# ====================================================================
echo "Creating Ham Radio application menu category..."

# 1. Define the formal 'Ham Radio' visual directory properties
sudo mkdir -p /usr/share/desktop-directories
sudo tee /usr/share/desktop-directories/HamRadio.directory > /dev/null << 'EOF'
[Desktop Entry]
Value=1.0
Type=Directory
Name=Ham Radio
Comment=Amateur Radio Applications
Icon=js8call
EOF

# 2. Inject the custom category menu rule into the system-wide XDG layout
# This forces KDE Plasma's application menu to map the 'HamRadio' category tag to a real visual section.
sudo mkdir -p /etc/xdg/menus/applications-merged
sudo tee /etc/xdg/menus/applications-merged/hamradio.menu > /dev/null << 'EOF'
<!DOCTYPE Menu PUBLIC "-//freedesktop//DTD Menu 1.0//EN"
 "http://freedesktop.org">
<Menu>
  <Name>Applications</Name>
  <Menu>
    <Name>Ham Radio</Name>
    <Directory>HamRadio.directory</Directory>
    <Include>
      <Category>HamRadio</Category>
    </Include>
  </Menu>
</Menu>
EOF

# 3. Create Desktop entry pointing to the new Ham Radio category mapping
sudo bash -c "cat > /usr/share/applications/JS8Call.desktop << EOF
[Desktop Entry]
Type=Application
Name=JS8Call
GenericName=Weak Signal Digital Communications
Exec=${JS8_BIN_DIR}/JS8Call
Icon=js8call
Terminal=false
Categories=HamRadio;
EOF"
# ====================================================================


# Install icon to standard hicolor theme location
sudo mkdir -p /usr/share/icons/hicolor/scalable/apps
sudo cp ../artwork/icon_128.svg /usr/share/icons/hicolor/scalable/apps/js8call.svg

# Refresh desktop menu database
if command -v update-desktop-database > /dev/null 2>&1; then
  sudo update-desktop-database /usr/share/applications
fi

# --- Cleanup prompt ---
clear
divider
echo "DONE!"
echo "Do you want to remove the JS8Call development directory?
The program will run fine without it and you can re-fetch it with this
script at any time."
divider
read -p "Remove JS8Call source tree? Yes(y) / No(n): " CLEANUP </dev/tty

if [ "${CLEANUP}" = "y" ]; then
  rm -rf "$HOME/development"
  echo "Source tree removed."
else
  echo "Source tree kept at ~/development."
fi

divider
echo "JS8Call is safely installed inside /opt! 
The launcher should appear in your application menu.
KDE Plasma environments remain fully protected from library leaking."
divider
exit 0
