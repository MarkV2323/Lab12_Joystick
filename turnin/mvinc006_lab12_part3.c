/*	Author: Mark Alan Vincent II
 *  Partner(s) Name: NA
 *	Lab Section: A01
 *	Assignment: Lab #12  Exercise #3
 *	Youtube:  https://youtu.be/nxx2VnlezRM
 *  Exercise Description:
 *
 *  Spawn a 1 block character on the LED matrix and allow it move left and right
 *  Now I need to increment a speed for the LED
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#endif

// Method for handling writing to register.
void transmit_data(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
   	 // Sets SRCLR to 1 allowing data to be set
   	 // Also clears SRCLK in preparation of sending data
   	 PORTC = 0x08;
   	 // set SER = next bit of data to be sent.
   	 PORTC |= ((data >> i) & 0x01);
   	 // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
   	 PORTC |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTC |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTC = 0x00;
}

// Does A2D conversion
void A2D_init() {
      ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}

// move counters
unsigned char moveLeft = 0;
unsigned char moveRight = 0;

// globals (shared varibles for the SMs)
unsigned char tmpA = 0x00;
unsigned char tmpB = 0x00; // Output for B
unsigned char tmpD = 0x00; // Output for D

// Position of character
unsigned char row_val = 0x00; // (x)
unsigned char row_sel = 0x00; // (y)

// function for shifting
// 0 - right, 1 - left, 2 - up, 3 - down
void shiftFunc(int direction) {
  switch (direction) {
    case 0:
        // right
        if ((row_val >> 1) == 0x00) {
          // Rolls over to the other side of matrix.
          row_val = 0x80;
        } else {
            // Move right
            row_val = row_val >> 1;
        }
        break;
    case 1:
        // Left
        if ((row_val << 1) == 0x00) {
          // Rolls over to the other side of matrix.
          row_val = 0x01;
        } else {
            // Move Left
            row_val = row_val << 1;
        }
        break;
    case 2:
        // up
        break;
    case 3:
        // down
        break;
    default:
        break;
  }
}

// Our concurrent synchSMs
// enum SM1_States { SM1_Start, SM1_Dis } sm1_state;
// int TickFct_Display(int state) {
//
//     // Gather input from joystick
//     unsigned short input = ADC; // Saves input from ADC
//
//     // Transitions
//     switch(state) {
//
//         case SM1_Start:
//             // init Output
//             tmpB = 0x00;
//             tmpD = 0x00;
//             state = SM1_Dis;
//             break;
//         case SM1_Dis:
//             state = SM1_Dis;
//             break;
//         default:
//             // Default
//             state = SM1_Start;
//             break;
//
//     }
//
//     // Actions
//     switch(state) {
//
//         case SM1_Start:
//             break;
//
//         case SM1_Dis:
//             // Break up bits from input
//             tmpB = (char) input;
//             tmpD = (char) (input >> 8);
//             tmpD = (tmpD & 0x03);
//
//             // Write outputs
//             PORTB = tmpB;
//             // PORTD = tmpD;
//             break;
//
//         default:
//             break;
//
//     }
//
//     // End of Tick Function SM1
//     return state;
// }

enum SM2_States { SM2_Start, SM2_Gather } sm2_state;
int TickFct_GatherInput(int state) {

    // Gather input
    unsigned short input = ADC; // Saves input from ADC

    // Transitions
    switch(state) {

        case SM2_Start:
            // init vars (start player at far left)
            row_val = 0x80;
            row_sel = 0x1E;

            // Go to draw state
            state = SM2_Gather;
            break;

        case SM2_Gather:
            // Stay gathering g
            state = SM2_Gather;
            break;

        default:
            // Default
            state = SM2_Start;
            break;

    }

    // Actions
    switch(state) {

        case SM2_Start:
            break;

        case SM2_Gather:

            // Flags current input
            if (input < 30) {
                // 100 ms move left
                moveLeft = moveLeft + 10;
            } else if (input < 100) {
                // 250 ms move left
                moveLeft = moveLeft + 4;
            } else  if (input < 200){
                // 500 ms move left
                moveLeft = moveLeft + 2;
            } else if (input < 350) {
                // 1000 ms move left
                moveLeft = moveLeft + 1;
            } else if (input < 700) {
                // no move, joystick not being touched.
                //moveLeft = 0;
                //moveRight = 0;
            } else if (input < 750) {
                // 1000 ms move right
                moveRight = moveRight + 1;
            } else if (input < 900) {
                // 500 ms move right
                moveRight = moveRight + 2;
            } else if (input < 950) {
                // 250 ms Move right
                moveRight = moveRight + 4;
            } else if (input < 1050) {
                // 100 ms Move right
                moveRight = moveRight + 10;
            } else {
                // nothing
            }

            // Checks if current input counters require a shift.
            if (moveLeft >= 20) {
              // Perform left move operation
              // Left
              if ((row_val << 1) == 0x00) {
                // Rolls over to the other side of matrix.
                row_val = 0x01;
              } else {
                  // Move Left
                  row_val = row_val << 1;
              }
              // reset moveLeft
              moveLeft = 0;

            }

            // Checks if current input counters require a shift.
            if (moveRight >= 20) {
              // Perform right move operation
              // right
              if ((row_val >> 1) == 0x00) {
                // Rolls over to the other side of matrix.
                row_val = 0x80;
              } else {
                  // Move right
                  row_val = row_val >> 1;
              }
              // reset moveRight
              moveRight = 0;

            }

            break;

        default:
            break;

    }

    // End of Tick Function SM3
    return state;
}

enum SM4_States { SM4_Start, SM4_Draw } sm4_state;
int TickFct_Draw(int state) {

    // counter for current frame to draw (row in this case).
    static unsigned char drawFrame = 0;

    // Transitions
    switch(state) {

        case SM4_Start:
            // init vars
            drawFrame = 0;
            // Go to draw state
            state = SM4_Draw;
            break;

        case SM4_Draw:
            // Stay drawing g
            state = SM4_Draw;
            break;

        default:
            // Default
            state = SM4_Start;
            break;

    }

    // Actions
    switch(state) {

        case SM4_Start:
            break;

        case SM4_Draw:
            if (drawFrame == 4) {
              // draw last frame and reset
              PORTC = row_val;
              PORTD = row_sel;
              drawFrame = 0;
            } else {
              // draw and increment frame
              PORTC = row_val;
              PORTD = row_sel;
              drawFrame = drawFrame + 1;
            }
            break;

        default:
            break;

    }

    // End of Tick Function SM3
    return state;
}

// Main Funcntion
int main(void) {

    // PORTS
    //DDRA = 0x00; PORTA = 0xFF; // PortA as input
    DDRB = 0xFF; PORTB = 0x00; // PortB as output
    DDRC = 0xFF; PORTC = 0x00; // PortC as output
    DDRD = 0xFF; PORTD = 0x00; // PortD as output

    // Start A2D conversion
    A2D_init();

    // Set Matrix values
    // PORTC = 0x80; // (x)
    // PORTD = 0x1E; // (y)

    // Setup Task List
    unsigned char i = 0;

    // SM1
    tasks[i].state = SM2_Start;
	  tasks[i].period = 50;
	  tasks[i].elapsedTime = tasks[i].period;
	  tasks[i].TickFct = &TickFct_GatherInput;
    i++;

    tasks[i].state = SM4_Start;
	  tasks[i].period = 10;
	  tasks[i].elapsedTime = tasks[i].period;
	  tasks[i].TickFct = &TickFct_Draw;

    // Setup System Period & Timer to ON.
    TimerSet(tasksGCD);
    TimerOn();
    while (1) {}
    return 1;
}
