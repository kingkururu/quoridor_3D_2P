
#include "game/core/game.hpp"
#include "network.hpp"

// int main(){
//     Constants::initialize(); 
    
//     GameManager game1; 
//     game1.runGame();
// }

// Add this to your main function
int main(int argc, char* argv[]) {

    Constants::initialize(); 

    GameManager game1; 
    game1.runGame();
    
    // Auto-start networking based on command line args
    if (argc > 1) {
        if (strcmp(argv[1], "host") == 0) {
            std::cout << "Auto-starting as host..." << std::endl;
            // You'd need to add a method to trigger hosting automatically
        } else if (strcmp(argv[1], "client") == 0) {
            std::cout << "Auto-starting as client..." << std::endl;
            // You'd need to add a method to trigger client automatically
        }
    }
    
    game1.runGame();
    return 0;
}