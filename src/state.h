#ifndef STATE_H
#define STATE_H

enum State { IDLE, RECORDING, ENTERING, COMPARING };

// Global state variable
extern State currentState;

#endif // STATE_H
