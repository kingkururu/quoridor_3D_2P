# 2 Player 3D Quoridor Simulation

A 3D implementation of the classic strategic board game Quoridor, built with C++ and SFML. Features multiplayer networking capabilities and simulated 3D graphics. Players race to reach the opposite side of the board while placing walls to block their opponent.

<img width="1365" alt="Image" src="https://github.com/user-attachments/assets/ef6666c5-f0dc-4a39-9f36-463fabf2eb87" />
<img width="1362" alt="Image" src="https://github.com/user-attachments/assets/946d1da7-77c0-4e1d-a79e-bdbab224b09c" />

## 🎯 Features

- **Strategic Gameplay**: Classic Quoridor rules with modern 3D visualization
- **Real-time Networking**: Berkeley sockets implementation for seamless multiplayer experience
- **3D Graphics**: 3D-like board visualization with SFML-powered rendering using raycasting
- **Interactive UI**: Intuitive controls for piece movement and wall placement

## 🎮 Game Rules

Quoridor is a strategic race game where:
- **Objective**: Be the first player to reach the opposite side of the 9x9 board
- **Movement**: Move your pawn one square per turn (orthogonally)
- **Wall Placement**: Alternatively, place a wall to block your opponent's path
- **Wall Restrictions**: Each player has 10 walls; walls cannot completely block a path to the goal
- **Victory Condition**: First player to reach any square on the opposite side wins

## 🎮 Controls

- **Click**: Place walls (horizontal/vertical)
- **Tab**: Switch between move and wall placement modes
- **W**: Move forward
- **S**: Move backward  
- **A**: Rotate camera left / turn player left
- **D**: Rotate camera right / turn player right


## 🌐 Networking Features

The game implements **Berkeley sockets** (BSD sockets/POSIX sockets) for multiplayer functionality:

- **Cross-Platform Compatibility**: Native socket API support on Unix-like systems
- **Real-time Communication**: Low-latency game state synchronization
- **Connection Management**: Automatic handling of client-server connections
- **Game State Sync**: Seamless board updates across networked players
- **Error Handling**: Robust network error detection and recovery

## 📁 Project Structure

```
/quoridor_3D_2P
│
├── .vscode/                   # VSCode settings and configurations
│
├── src/                       # Source files
│   ├── main.cpp              # Main entry point of the game
│   └── game/                 # Core engine functionalities
│       ├── globals/          # Constants and flags
│       ├── core/             # Game loop and state management
│       ├── physics/          # Physics and collision detection
│       ├── camera/           # SFML window and view management
│       ├── utils/            # Utility functions
│       └── scenes/           # Scene management
│
├── assets/                    # Game assets
│   ├── fonts/                # Text files and sources
│   ├── sound/                # Sound effects
│   ├── tiles/                # Tiles and tilemaps
│   └── sprites/              # Sprite images
│
├── libs/                      # External libraries
│   └── logging/              # Logging system
│
├── test/                      # Test game (example project)
├── Makefile                   # Build instructions
└── README.md                  # Project documentation
```

## 🚀 Building & Running

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

2. **Build the Project**:
   ```bash
   make
   ```
   This will compile the Quoridor game along with the networking components.

3. **Run the Game**:
   ```bash
   make run
   ```

4. **Clean the Build**:
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

## 🎯 Key Learning Outcomes

- **Game Logic Implementation**: Complex rule validation and game state management
- **Network Programming**: Berkeley sockets implementation for real-time multiplayer
- **3D Graphics Programming**: Raycasting techniques for immersive visualization
- **Event-Driven Architecture**: Responsive UI and game state management

## 🌐 Networking Architecture

### Berkeley Sockets Implementation
The game uses native system socket APIs without external libraries:

- **Socket Creation**: Low-level socket initialization and configuration
- **Connection Management**: Client-server architecture with automatic reconnection
- **Data Serialization**: Efficient game state encoding for network transmission
- **Error Handling**: Robust network error detection and recovery mechanisms
- **Cross-Platform Support**: POSIX-compliant socket implementation

### Multiplayer Modes
- **Local Play**: Two players on the same computer
- **Network Play**: Players on different computers via TCP/IP
- **AI vs Human**: Single-player mode against computer opponents
- **AI vs AI**: Watch computer players compete against each other

## 🎨 Assets Credits

- **Music**: [Game Background](https://pixabay.com/music/video-games-game-176807/) from Pixabay
- **Graphics**: Custom artwork created using Canva and Canva ai
- **Backgrounds**: Original board textures and visual designs using Canva and Canva ai
- **Framework**: Built using the Custom SFML Game Framework

## 🏆 Game Strategy Tips

- **Early Game**: Focus on advancing your pawn while conserving walls
- **Mid Game**: Use walls strategically to create longer paths for your opponent
- **Late Game**: Balance between advancing and defensive wall placement
- **Wall Efficiency**: Place walls to maximize your opponent's path length
- **Path Awareness**: Always ensure you maintain a clear route to your goal
