
## Raycast test application

This test game is made using my game template with SFML and C++. The makefile is set to download all the necessary stuff like homebrew, spdlog, and sfml. 

To compile and run, download the zip file from release.
locate program through terminal via
cd /User/ … (type in the path)
cd maze_3D-1.0.3
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

## Game Play Screenshots: 

<img width="641" alt="Image" src="https://github.com/user-attachments/assets/681bd3e9-5e92-47ae-bc73-865dbb595454" />
<img width="884" alt="Image" src="https://github.com/user-attachments/assets/4856a85c-618c-4cc3-9e42-bb8f56957bd5" />
<img width="950" alt="Image" src="https://github.com/user-attachments/assets/ae51ba5a-fd90-45b8-8330-e17a46d82088" />
