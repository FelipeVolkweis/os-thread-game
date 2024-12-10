#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>

#define WIDTH 800
#define HEIGHT 600
#define INVALID -1
#define SCORE_INITIAL 0
#define PACKAGE_SIZE 50
#define PACKAGE_SPEED_BASE 150.0f        
#define PACKAGE_SPEED_INCREMENT 20.0f    
#define PACKAGE_COLOR sf::Color::Green 
#define SCORE_THRESHOLD 5                
#define THREADMILL_HEIGHT 80
#define THREADMILL_Y_POS_CENTER 250
#define THREADMILL_Y_POS_TOP (THREADMILL_Y_POS_CENTER - THREADMILL_HEIGHT - 100)
#define THREADMILL_Y_POS_BOTTOM (THREADMILL_Y_POS_CENTER + THREADMILL_HEIGHT + 100)
#define THREADMILL_COLOR sf::Color::Red
#define SCORE_TEXT_SIZE 24
#define SCORE_TEXT_POS_X 10
#define SCORE_TEXT_POS_Y 10
#define LIVES_TEXT_POS_X 10
#define LIVES_TEXT_POS_Y 40
#define FONT_PATH "assets/04B_30__.ttf"
#define THREADMILL_WIDTH WIDTH
#define PACKAGE_START_X 0.0f
#define PLAYER_SIZE 50
#define PLAYER_SPEED 200.0f
#define PLAYER_COLOR sf::Color::Blue
#define MAX_LANE 2
#define MIN_LANE 0
#define PLAYER_OFFSET_Y 10.0f
#define PACKAGE_SPAWN_INTERVAL_BASE 2.0f     
#define PACKAGE_SPAWN_INTERVAL_DECREMENT 0.2f  
#define PACKAGE_SPAWN_INTERVAL_MIN 0.5f        
#define MAX_LIVES 3

#endif // CONSTANTS_H