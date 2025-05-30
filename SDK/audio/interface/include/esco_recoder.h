#ifndef ESCO_RECODER_H
#define ESCO_RECODER_H


#define COMMON_SCO   	  0    //普通SCO
#define JL_DOGLE_ACL  	  1    //dongle   ACL链路



int esco_recoder_open(u8 link_type, void *bt_addr);

void esco_recoder_close();

int esco_recoder_switch(u8 en);

int esco_recoder_reset(void);

int esco_recoder_running();

void esco_recoder_set_ai_tx_node_func(int (*func)(u8 *, u32));

int audio_sidetone_open(void);
int audio_sidetone_close(void);
int get_audio_sidetone_state();


#endif
