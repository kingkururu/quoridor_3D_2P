
## 2Player 3D Quoridor Simulation
Quoridor is a two-player strategic board game where each player’s objective is to reach the opposite side of the board before the opponent. Players can either move their piece one square per turn or place a wall to block their opponent — but a path to the goal must always remain open.

This version brings Quoridor into a 3D visual experience using SFML and custom-built graphics and logic components.

## Game Play Screenshots
<img width="1365" alt="Image" src="https://github.com/user-attachments/assets/ef6666c5-f0dc-4a39-9f36-463fabf2eb87" />

<img width="1362" alt="Image" src="https://github.com/user-attachments/assets/946d1da7-77c0-4e1d-a79e-bdbab224b09c" />

Music is by https://pixabay.com/music/video-games-game-176807/ and sprites and backgrounds are drawn by @kingkururu (me)

## How to Compile and Run
To compile and run, download the zip file from release.
locate program through terminal via
1. cd /User/ … (type in the path)
2. cd 3Dquoridor
3. make install (if this doesn't work, download homebrew, and then install sfml@2, spdlog, fmt, and yaml-cpp seperately)
5. make test

설정 -> 개인정보 보호 및 보안
다음에서 응용 프로그램 허용 -> 그래도 허용
settings -> privacy & security
allow applications downloaded from ... enable it

nano ~/.zshrc (for zshrc)
nano ~/.bash_profile (for bash)

export PATH="/opt/homebrew/bin:$PATH"
export PATH="/opt/homebrew/opt/sfml/bin:$PATH"
export PATH="/opt/homebrew/opt/yaml-cpp/bin:$PATH"

Ctrl O, Ctrl X
source ~/.zshrc (for zshrc)
source ~/. bash_profile (for bash)

