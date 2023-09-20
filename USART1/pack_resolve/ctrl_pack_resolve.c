#include "ctrl_pack_resolve.h"

void handle_init(handle_t* handle);
void update(uint8_t* data, size_t length);

handle_t handle;

void handle_init(handle_t* handle)
{
	handle->joyLHori=maxJoy/2;
	handle->joyLVeri=maxJoy/2;
	handle->joyRHori=maxJoy/2;
	handle->joyRVeri=maxJoy/2;
	handle->updata=update;
}

void update(uint8_t* data, size_t length)
{
	handle.joyLHori=data[0] | data[1]<<8;
	handle.joyLVeri=data[2] | data[3]<<8;
	handle.joyRHori=data[4] | data[5]<<8;
	handle.joyRVeri=data[6] | data[7]<<8;
	handle.trigLT=data[8] | data[9]<<8;
	handle.trigRT=data[10] | data[11]<<8;
	handle.btanA=data[13] & 0x01;	//1
	handle.btanB=data[13] & 0x02;	//2
	handle.btanX=data[13] & 0x08;	//4
	handle.btanY=data[13] & 0x10;	//5
	handle.btnLB=data[13] & 0x40;	//7
	handle.btnRB=data[13] & 0x80;	//8
	handle.btnStart=data[14] & 0x04;	//3
	handle.btnSelect=data[14] & 0x08; //4
	handle.btnXbox=data[14] & 0x10;	//5
	handle.btnLS=data[14] & 0x20;	//6
	handle.btnRS=data[14] & 0x40; //7
	handle.btnShare=data[15] & 0x01;	//1
	switch(data[12])
	{
		case 0:
			handle.btnDirUp=0;
			handle.btnDirDown=0;
			handle.btnDirLeft=0;
			handle.btnDirRight=0;
			break;
		case 1:
			handle.btnDirUp=1;
			handle.btnDirDown=0;
			handle.btnDirLeft=0;
			handle.btnDirRight=0;
			break;
		case 2:
			handle.btnDirUp=1;
			handle.btnDirDown=0;
			handle.btnDirLeft=0;
			handle.btnDirRight=1;
			break;
		case 8:
			handle.btnDirUp=1;
			handle.btnDirDown=0;
			handle.btnDirLeft=1;
			handle.btnDirRight=0;
			break;
		case 5:
			handle.btnDirUp=0;
			handle.btnDirDown=1;
			handle.btnDirLeft=0;
			handle.btnDirRight=0;
			break;
		case 4:
			handle.btnDirUp=0;
			handle.btnDirDown=1;
			handle.btnDirLeft=0;
			handle.btnDirRight=1;
			break;
		case 6:
			handle.btnDirUp=0;
			handle.btnDirDown=1;
			handle.btnDirLeft=1;
			handle.btnDirRight=0;
			break;
		case 3:
			handle.btnDirUp=0;
			handle.btnDirDown=0;
			handle.btnDirLeft=0;
			handle.btnDirRight=1;
			break;
		case 7:
			handle.btnDirUp=0;
			handle.btnDirDown=0;
			handle.btnDirLeft=1;
			handle.btnDirRight=0;
			break;
	}
}
