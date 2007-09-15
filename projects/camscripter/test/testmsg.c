#include <stdlib.h>
#include "csmsg.h"
#include "slip.h"
#include "stdint.h"

void testBytesToMsg( void ) {
	uint_least8_t test[] = {0x00, 0x0A, 2, 0x00, 0x00, 1, '4', 0x00, 0x00, 2, 'A', 'B'};
	ScripterMsg* msg = bytes_to_message( test, 12 );
	if (msg->id != 0x0A) {
		printf("ID: Expteced <%X>, received<%X>\n",0x0A,msg->id);
	}
	if (msg->param_count!=2) {
		printf("P: Expteced <%d>, received<%d>\n",2,msg->param_count);
	}
	if (msg->parameters->size != 1) {
		printf("P1.size: Expteced <%d>, received<%d>\n",1,msg->parameters->size);
	}
	if (*(msg->parameters->data) != '4') {
		printf("P1.data: Expteced <%c>, received<%c>\n",'4',*(msg->parameters->data));
	}
	
	if (msg->parameters->next_param->size != 2) {
		printf("P2.size: Expteced <%d>, received<%d>\n",2,msg->parameters->next_param->size);
	}
	if (*(msg->parameters->next_param->data) != 'A') {
		printf("P2.data[0]: Expteced <%c>, received<%c>\n",'A',*(msg->parameters->next_param->data));
	}
	if (*(msg->parameters->next_param->data+1) != 'B') {
		printf("P2.data[1]: Expteced <%c>, received<%c>\n",'B',*(msg->parameters->next_param->data+1));
	}
}

void testMsgToBytes( void ) {
	ScripterMsg msg;
//	// I want to get back the following message (bits in hexa)
//	// 00 0A 02 00 00 01 04 00 00 02 AA BB
	msg.id = 10;
	msg.param_count = 2;
	MsgParam* param = malloc(sizeof(MsgParam));
	param->size = 1;
	param->data = malloc(1);
	*(param->data) = 0x04;
	msg.parameters = param;
	param = malloc(sizeof(MsgParam));
	param->size = 2;
	param->data = malloc(2);
	param->data[0] = 0xAA;
	param->data[1] = 0xBB;
	msg.parameters->next_param = param;
	uint_least8_t* msg_bytes;
	int i;
	int msg_size = message_to_bytes(msg,&msg_bytes);	
	printf("size<%d>\n",msg_size);
	uint_least8_t expected[] = {0x00, 0x0A, 0x02, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x02, 0xAA, 0xBB};
	for( i=0; i<msg_size; i++ ) {
		if (expected[i]!=msg_bytes[i]) {
			printf("expected <%X> received <%X> position <%d>.",expected[i],msg_bytes[i],i);
		}
	}
}

static int idx = 0;
char* chargeter(char* text) {
	if (text[idx] != '\0') {
		return &text[idx++];
	}
	return NULL;
}

void testSlipMsg() {
	char* test = "Hola. Test!";	
	stream_packet(&chargeter,test);
}

int main(void) {
//	testMsgToBytes();
//	testBytesToMsg();
	testSlipMsg();
//	uint_least8_t sample[] = {0x00, 0x00, 0x01};
//	int idx=0;
//	printf("[%d]\n",(sample[0]<<16 | sample[1]<<8 | sample[2]));
	return 0;
}
