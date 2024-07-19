#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".esco_tx_node.data.bss")
#pragma data_seg(".esco_tx_node.data")
#pragma const_seg(".esco_tx_node.text.const")
#pragma code_seg(".esco_tx_node.text")
#endif
#include "jlstream.h"
#include "classic/hci_lmp.h"
#include "media/audio_base.h"
#include "app_config.h"

struct esco_tx_hdl {
    u8 start;
};


static void esco_tx_handle_frame(struct stream_iport *iport, struct stream_note *note)
{
    struct stream_frame *frame;

    while (1) {
        frame = jlstream_pull_frame(iport, note);
        if (!frame) {
            break;
        }
        /*printf("esco_tx: %d\n", frame->len);*/
        lmp_private_send_esco_packet(NULL, frame->data, frame->len);
        jlstream_free_frame(frame);
    }
}

static int esco_tx_bind(struct stream_node *node, u16 uuid)
{
    return 0;
}

static void esco_tx_open_iport(struct stream_iport *iport)
{
    iport->handle_frame = esco_tx_handle_frame;
}

static int esco_tx_ioc_fmt_nego(struct stream_iport *iport)
{
    struct stream_fmt *in_fmt = &iport->prev->fmt;

    int type = lmp_private_get_esco_packet_type();
    int media_type = type & 0xff;
    if (media_type == 0) {
        in_fmt->sample_rate = 8000;
        in_fmt->coding_type = AUDIO_CODING_CVSD;
    } else  {
        in_fmt->sample_rate = 16000;
        in_fmt->coding_type = AUDIO_CODING_MSBC;
    }
    in_fmt->channel_mode = AUDIO_CH_MIX;

    return NEGO_STA_ACCPTED;
}

static int esco_tx_ioctl(struct stream_iport *iport, int cmd, int arg)
{
    struct esco_tx_hdl *hdl = (struct esco_tx_hdl *)iport->node->private_data;

    switch (cmd) {
    case NODE_IOC_OPEN_IPORT:
        esco_tx_open_iport(iport);
        break;
    case NODE_IOC_CLOSE_IPORT:
        break;
    case NODE_IOC_NEGOTIATE:
        *(int *)arg |= esco_tx_ioc_fmt_nego(iport);
        break;
    case NODE_IOC_GET_DELAY:
        return lmp_private_get_esco_tx_packet_num() * 75;
    }

    return 0;
}

static void esco_tx_release(struct stream_node *node)
{

}


REGISTER_STREAM_NODE_ADAPTER(esco_tx_adapter) = {
    .name       = "esco_tx",
    .uuid       = NODE_UUID_ESCO_TX,
    .bind       = esco_tx_bind,
    .ioctl      = esco_tx_ioctl,
    .release    = esco_tx_release,
    .hdl_size   = sizeof(struct esco_tx_hdl),
};

