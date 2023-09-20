#ifndef _CTRL_PACK_RESOLVE_H_
#define _CTRL_PACK_RESOLVE_H_
#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#define maxJoy 0XFFFF

typedef struct{
	bool btanA;
	bool btanB;
	bool btanX;
	bool btanY;
	
	bool btnShare;
	bool btnStart;
	bool btnSelect;
	bool btnXbox;
	
	bool btnLB;
	bool btnRB;
	
	bool btnLS;
	bool btnRS;
	
	bool btnDirUp;
	bool btnDirLeft;
	bool btnDirRight;
	bool btnDirDown;
	
	uint16_t joyLHori;
	uint16_t joyLVeri;
	uint16_t joyRHori;
	uint16_t joyRVeri;
	
	uint16_t trigLT;
	uint16_t trigRT;
	void (*updata)(uint8_t* data, size_t length);
}handle_t;

void handle_init(handle_t* handle);

void update(uint8_t* data, size_t length);


extern handle_t handle;


#endif
