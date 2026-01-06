# Building FTP Client GUI on Linux/WSL

## Install Qt on Ubuntu/Debian (WSL)

```bash
# Update package list
sudo apt update

# Install Qt5 development tools
sudo apt install qt5-qmake qtbase5-dev qtbase5-dev-tools build-essential

# Verify installation
qmake --version
```

## Build the GUI

```bash
cd "/mnt/c/Users/Legion/Documents/Github Repo/FTP-Client-Server"

# Generate Makefile
qmake cpftp-gui.pro

# Build
make -j$(nproc)

# Run (requires X server on WSL)
./cpftp-gui
```

## Running GUI on WSL

WSL doesn't have a display server by default. You need:

1. **Install X server on Windows**:
   - Download and install VcXsrv or Xming
   - Launch XLaunch with default settings

2. **Set DISPLAY variable in WSL**:
   ```bash
   export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
   ```

3. **Run the application**:
   ```bash
   ./cpftp-gui
   ```

## Easier Alternative: Build on Windows

Since your project is in Windows filesystem, it's easier to:
1. Use Qt Creator on Windows
2. Or use Windows Command Prompt with Qt installed
3. See `GUI_README.md` for Windows build instructions
