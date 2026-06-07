# ScopeForge

> A real-time power quality analyzer and fault signature engine for UPS and stabilizer systems.

**Status** Active Development (foundations phase). Not yet usable; follow along at it gets built.

--- 

## What this is

ScopeForge is an open-source desktop application for capturing, analyzing and replaying electrical waveforms from Uninterruptible Power Supply (UPS) systems and voltage stabilizers. It computes industry standard power quality metrics (IEEE 519 and IEEE 1159), visualizes them through an oscilloscope-style interface, and eventually classifies fault signatures against a library curated from real field service experience.

The short version: a Fluke 435 costs USD5,000+ and is closed-source. ScopeForge aims to give field technicians, students and small operators a free, open alternative thats good enough for diagnostic and learning work.

## Why I'm building it

I've spent roughly 10 years as a field technician servicing UPS and stabilizer systems from 500VA to 500KVA in East Africa. Actoss that time, two things have been consistently true: power quality on the grid is sometimes worse than the design assumptions of the equipment, and the tools to diagnose it properly cost more than most technicians earn in a year. I'm now in my final year of B.Software engineering, and ScopeForge is the project iI wish had existed when I started.

This is being built as a portfolio piece and a useful tool.

## Planned Features 

These are the destination, **not** the current state. Tracking issues will appear as each is started.

- Real-time waveform acquisition from multiple sources; synthetic generator, CSV/file playback, Modbus RTU/TCP, and a custom serial protocol for cheap microcontroller-based frontends. 
- IEEE 519-compliant harmonic analysis (THD, individual harmonics through the 50th)
- IEEE 1159 event detection: sag, swell, interruption, rapid voltage change.
- Single and Three phase support, with symmetrical components for unbalanced analysis
- Oscilloscope-style live visualization (wavefom, spectrum, phasor diagram, scrolling metrics)
- Fault classification against a curated library of UPS-specif signatures 
- Fault injection and replay - record a real disturbance, parameterize it, play it back 
- Protocols buffers over TCP for remote telemetry 
- Linux (primary), Windows (best-effort)

## What works today 

- A real-time scrolling waeform window using ImGui + ImPlot + GLFW
- Synthetic 50Hz/230Vrms signal generation with configurable harmonics and Gaussian noise 
- Interative controls for frequency, amplitude, 3rd/5th/7th harmonic content and noise level.

Thats it for now. Alot more is coming 

## Tech Stack 

- **Language:** C++20 (GCC 13+ / Clang 16+)
- **Build:** CMake 3.20+, Ninja
- **UI:** Dear ImGui + ImPlot + GLFW (OpenGL 3.3 Core)
- **Numerics (planned):** Eigen, KissFFT
- **Networking (planned):** Asio (standalone), Protocol Buffers
- **Industrial protocol (planned):** libmodbus 
- **Persistence (planned):** SQLite
- **Testing (planned):** Catch2 

## Building 

> Linux is the primary target. Windows is best-effort and currently untested

**Dependencies (Ubuntu/Debian):*

```bash 
sudo apt update
sudo apt install -y build-essential cmake git ninja-build \
                    libgl1-mesa-dev libglfw3-dev 

```       

**Clone and build:**

```
``` 
```bash 
git clone --recurse-submodules 
https://github.com/rw3h4/scopeforge.git
cd scopeforge
cmake -B build build 
cmake --build build 
./build/scopeforge
```  
```   
```                


If you forgot `--recurse-submodules` on clone 

```bash 
git submodule update --init --recursive 
```    


## Safety 

ScopeForge will eventually interface with circuits carrying mains voltage. **Mains-voltage measurement is dangerous and can be lethal.** When hardware support arrives, this README will document the required isolation (transformer-coupled voltage sensing, properly rates CT's opto-isolation between the analog front-end and host computer). Until then, the project runs entirely on synthetic data and recorded files.
**Do no connect any DIY circuit to mains voltage without certified isolation and review by a qualified electrical technician.** 

## Contributing 

Issues and discussions are welcome. Pull requests are welcome too, but please open an issue first - the architecture is still settling and I'd rather avoid wasted work.

## Disclaimer 

ScopeForge is **not** (nor is it intended to be) a certified instrument. It does not pursue IEC 61000-4-30 Class A compliance and its outputs must not be used for regulatory reporting, contractual measuement or safety-critical decisions. It is intended for diagnostic, educational and research use. 

## License 

MIT 

## Acknowledgements 

Built on the work of many: Dear ImGui (Omar Cornut), ImPlot (Evan Pezent), GLFW, KissFFT, Eigen, Asio (Christopher Kohlhoff), libmodbus (Stephane Raimbault), Catch2 and the IEEE Power & Energy Society for the standards that define this field. 
