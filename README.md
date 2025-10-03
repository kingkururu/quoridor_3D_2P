# 2 Player 3D Quoridor Simulation

A simulated 3D implementation of the classic strategic board game Quoridor, built with C++ and SFML. Features multiplayer networking capabilities. Players race to reach the opposite side of the board while placing walls to block their opponent.

<img width="1348" height="479" alt="Image" src="https://github.com/user-attachments/assets/cc61f5af-2a66-423a-92b4-b88ae78cde93" />
<img width="1344" height="472" alt="Image" src="https://github.com/user-attachments/assets/7040c707-69ea-4417-ad64-f8c29a8f4dfb" />
<img width="1365" alt="Image" src="https://github.com/user-attachments/assets/ef6666c5-f0dc-4a39-9f36-463fabf2eb87" />

## Features

- **Real-time Networking**: Berkeley sockets implementation for seamless multiplayer experience
- **2.5D Graphics**: 3D-like board visualization with SFML-powered rendering using raycasting
- **Interactive UI**: Intuitive controls for piece movement and wall placement

## Game Rules

Quoridor is a strategic race game where:
- **Objective**: Be the first player to reach the opposite side of the 9x9 board
- **Movement**: Move your pawn one square per turn (orthogonally)
- **Wall Placement**: Alternatively, place a wall to block your opponent's path
- **Wall Restrictions**: Each player has 10 walls; walls cannot completely block a path to the goal
- **Victory Condition**: First player to reach any square on the opposite side wins

## Controls

- **Click**: Place walls (horizontal/vertical)
- **W**: Move forward
- **S**: Move backward  
- **A**: Rotate camera left / turn player left
- **D**: Rotate camera right / turn player right


## Networking Features

The game implements **Berkeley sockets** (BSD sockets/POSIX sockets) for multiplayer functionality:

- **Cross-Platform Compatibility**: Native socket API support on Unix-like systems
- **Real-time Communication**: Low-latency game state synchronization
- **Connection Management**: Automatic handling of client-server connections
- **Game State Sync**: Seamless board updates across networked players
- **Error Handling**: Robust network error detection and recovery

## Project Structure

```
/quoridor_3D_2P
│
├── src/                       # Source files
│   ├── main.cpp               # Main entry point of the game
│   └── game/                  # Core engine functionalities
│       ├── globals/           # Constants and flags
│       ├── core/              # Game loop and state management
│       ├── physics/           # Physics and collision detection
│       ├── camera/            # SFML window and view management
│       ├── utils/             # Utility functions
│       └── scenes/            # Scene management
│
├── assets/                    # Game assets
│   ├── fonts/                 # Text files and sources
│   ├── sound/                 # Sound effects
│   ├── tiles/                 # Tiles and tilemaps
│   └── sprites/               # Sprites 
│
├── network/                   # Networking system
│   └── network/               # network files
│
├── libs/                      # External libraries
│   └── logging/               # Logging system
│
├── Makefile                   # Build instructions
└── README.md                  # Project documentation
```

## Building & Running

### Prerequisites
- **Compiler**: Requires clang++ (or g++) with C++17 support
- **SFML**: Simple and Fast Multimedia Library for graphics and windowing
- **Network Support**: POSIX-compliant system for Berkeley sockets
- **Custom Game Framework**: Built on top of the SFML Game Framework

### Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/kingkururu/quoridor_3D_2P
   cd quoridor_3D_2P
   ```

2. **Build and Run the Game**:
   ```bash
   make test
   ```

3. **Clean the Build**:
   ```bash
   make clean
   ```

### Alternative Setup (macOS with Homebrew)

1. **Install SFML**:
   ```bash
   # Install Homebrew if not already installed
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   
   # Install SFML
   brew install sfml@2
   brew install .... (other missing libraries)
   ```

2. **Configure PATH** (add to ~/.zshrc or ~/.bash_profile):
   ```bash
   export PATH="/opt/homebrew/bin:$PATH"
   export PATH="/opt/homebrew/opt/sfml@2/bin:$PATH"
   export PATH="path to other libraries if any of them is missing)
   ```

## Key Learning Outcomes

- **Game Logic Implementation**: Complex rule validation and game state management
- **Network Programming**: Berkeley sockets implementation for real-time multiplayer
- **2.5D Graphics Programming**: Raycasting techniques for 2.5D visualization
- **Event-Driven Architecture**: Responsive UI and game state management

## Assets Credits

- **Music**: [Game Background](https://pixabay.com/music/video-games-game-176807/) from Pixabay
- **Graphics**: Custom artwork created using Canva
- **Backgrounds**: Original board textures and visual designs using Canva
- **Framework**: Built using the Custom SFML Game Framework

## Tools Used
- **SFML**: https://github.com/SFML/SFML 
- **Yaml-cpp**: https://github.com/jbeder/yaml-cpp
- **Spdlog**: https://github.com/gabime/spdlog 
- **FMT**: https://github.com/fmtlib/fmt 

## Game Strategy Tips

- **Early Game**: Focus on advancing your pawn while conserving walls
- **Mid Game**: Use walls strategically to create longer paths for your opponent
- **Late Game**: Balance between advancing and defensive wall placement
- **Wall Efficiency**: Place walls to maximize your opponent's path length
- **Path Awareness**: Always ensure you maintain a clear route to your goal
