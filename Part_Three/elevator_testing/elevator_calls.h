#ifndef __ELEVATOR_CALLS_H
#define __ELEVATOR_CALLS_H

int start_elevator();
int issue_request(int type, int start, int dest);
int stop_elevator();

#endif /*__ELEVATOR_CALLS_H*/
