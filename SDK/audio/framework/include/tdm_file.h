#ifndef _TDM_FILE_H_
#define _TDM_FILE_H_

#include "generic/typedef.h"
#include "media/includes.h"
#include "app_config.h"

void audio_tdm_file_init(void);
void tdm_sample_output_handler(s16 *data, int len);

#endif // #ifndef _FM_FILE_H_
