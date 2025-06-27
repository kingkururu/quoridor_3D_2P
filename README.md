
## 2Player 3D Quoridor Simulation

To compile and run, download the zip file from release.
locate program through terminal via
cd /User/ … (type in the path)
cd 3Dquoridor
make install
brew install sfml@2
make test

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
