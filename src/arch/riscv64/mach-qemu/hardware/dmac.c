/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <types.h>
#include <dmac.h>
#include <fpioa.h>
#include <arch/plic.h>
#include <arch/memory.h>
#include <sysctl.h>
#include <utils.h>
#include <xbook/debug.h>
#include <k210_phymem.h>
#include <xbook/schedule.h>
#include <xbook/waitqueue.h>

static wait_queue_t dmac_waitqueue;

volatile dmac_t *const dmac = (dmac_t *)DMAC_V;

static int is_memory(uintptr_t address)
{
    enum
    {
        mem_len = 6 * 1024 * 1024,
        mem_no_cache_len = 8 * 1024 * 1024,
    };
    return ((address >= 0x80000000) && (address < 0x80000000 + mem_len)) || ((address >= 0x40000000) && (address < 0x40000000 + mem_no_cache_len)) || (address == 0x50450040);
}

uint64 dmac_read_id(void)
{
    return dmac->id;
}

uint64 dmac_read_version(void)
{
    return dmac->compver;
}

uint64 dmac_read_channel_id(dmac_channel_number_t channel_num)
{
    return dmac->channel[channel_num].axi_id;
}

static void dmac_enable(void)
{
    dmac_cfg_u_t dmac_cfg;

    dmac_cfg.data = readq(&dmac->cfg);
    dmac_cfg.cfg.dmac_en = 1;
    dmac_cfg.cfg.int_en = 1;
    writeq(dmac_cfg.data, &dmac->cfg);
}

void dmac_disable(void)
{
    dmac_cfg_u_t dmac_cfg;

    dmac_cfg.data = readq(&dmac->cfg);
    dmac_cfg.cfg.dmac_en = 0;
    dmac_cfg.cfg.int_en = 0;
    writeq(dmac_cfg.data, &dmac->cfg);
}

void dmac_channel_enable(dmac_channel_number_t channel_num)
{
    dmac_chen_u_t chen;

    chen.data = readq(&dmac->chen);

    switch(channel_num)
    {
        case DMAC_CHANNEL0:
            chen.dmac_chen.ch1_en = 1;
            chen.dmac_chen.ch1_en_we = 1;
            break;
        case DMAC_CHANNEL1:
            chen.dmac_chen.ch2_en = 1;
            chen.dmac_chen.ch2_en_we = 1;
            break;
        case DMAC_CHANNEL2:
            chen.dmac_chen.ch3_en = 1;
            chen.dmac_chen.ch3_en_we = 1;
            break;
        case DMAC_CHANNEL3:
            chen.dmac_chen.ch4_en = 1;
            chen.dmac_chen.ch4_en_we = 1;
            break;
        case DMAC_CHANNEL4:
            chen.dmac_chen.ch5_en = 1;
            chen.dmac_chen.ch5_en_we = 1;
            break;
        case DMAC_CHANNEL5:
            chen.dmac_chen.ch6_en = 1;
            chen.dmac_chen.ch6_en_we = 1;
            break;
        default:
            break;
    }

    writeq(chen.data, &dmac->chen);
}

void dmac_channel_disable(dmac_channel_number_t channel_num)
{
    dmac_chen_u_t chen;

    chen.data = readq(&dmac->chen);

    switch(channel_num)
    {
        case DMAC_CHANNEL0:
            chen.dmac_chen.ch1_en = 0;
            chen.dmac_chen.ch1_en_we = 1;
            break;
        case DMAC_CHANNEL1:
            chen.dmac_chen.ch2_en = 0;
            chen.dmac_chen.ch2_en_we = 1;
            break;
        case DMAC_CHANNEL2:
            chen.dmac_chen.ch3_en = 0;
            chen.dmac_chen.ch3_en_we = 1;
            break;
        case DMAC_CHANNEL3:
            chen.dmac_chen.ch4_en = 0;
            chen.dmac_chen.ch4_en_we = 1;
            break;
        case DMAC_CHANNEL4:
            chen.dmac_chen.ch5_en = 0;
            chen.dmac_chen.ch5_en_we = 1;
            break;
        case DMAC_CHANNEL5:
            chen.dmac_chen.ch6_en = 0;
            chen.dmac_chen.ch6_en_we = 1;
            break;
        default:
            break;
    }

    writeq(chen.data, &dmac->chen);
}

void dmac_enable_common_interrupt_status(void)
{
    dmac_commonreg_intstatus_enable_u_t intstatus;

    intstatus.data = readq(&dmac->com_intstatus_en);
    intstatus.intstatus_enable.enable_slvif_dec_err_intstat = 1;
    intstatus.intstatus_enable.enable_slvif_wr2ro_err_intstat = 1;
    intstatus.intstatus_enable.enable_slvif_rd2wo_err_intstat = 1;
    intstatus.intstatus_enable.enable_slvif_wronhold_err_intstat = 1;
    intstatus.intstatus_enable.enable_slvif_undefinedreg_dec_err_intstat = 1;

    writeq(intstatus.data, &dmac->com_intstatus_en);
}

void dmac_enable_common_interrupt_signal(void)
{
    dmac_commonreg_intsignal_enable_u_t intsignal;

    intsignal.data = readq(&dmac->com_intsignal_en);
    intsignal.intsignal_enable.enable_slvif_dec_err_intsignal = 1;
    intsignal.intsignal_enable.enable_slvif_wr2ro_err_intsignal = 1;
    intsignal.intsignal_enable.enable_slvif_rd2wo_err_intsignal = 1;
    intsignal.intsignal_enable.enable_slvif_wronhold_err_intsignal = 1;
    intsignal.intsignal_enable.enable_slvif_undefinedreg_dec_err_intsignal = 1;

    writeq(intsignal.data, &dmac->com_intsignal_en);
}

static void dmac_enable_channel_interrupt(dmac_channel_number_t channel_num)
{
    writeq(0xffffffff, &dmac->channel[channel_num].intclear);
    writeq(0x2, &dmac->channel[channel_num].intstatus_en);
}

void dmac_disable_channel_interrupt(dmac_channel_number_t channel_num)
{
    writeq(0, &dmac->channel[channel_num].intstatus_en);
}

static void dmac_chanel_interrupt_clear(dmac_channel_number_t channel_num)
{
    writeq(0xffffffff, &dmac->channel[channel_num].intclear);
}

int dmac_set_channel_param(dmac_channel_number_t channel_num,
                           const void *src, void *dest, dmac_address_increment_t src_inc, dmac_address_increment_t dest_inc,
                           dmac_burst_trans_length_t dmac_burst_size,
                           dmac_transfer_width_t dmac_trans_width,
                           uint32 blockSize)
{
    dmac_ch_ctl_u_t ctl;
    dmac_ch_cfg_u_t cfg_u;

    int mem_type_src = is_memory((uintptr_t)src), mem_type_dest = is_memory((uintptr_t)dest);
    dmac_transfer_flow_t flow_control;
    if(mem_type_src == 0 && mem_type_dest == 0)
    {
        flow_control = DMAC_PRF2PRF_DMA;
    } else if(mem_type_src == 1 && mem_type_dest == 0)
        flow_control = DMAC_MEM2PRF_DMA;
    else if(mem_type_src == 0 && mem_type_dest == 1)
        flow_control = DMAC_PRF2MEM_DMA;
    else
        flow_control = DMAC_MEM2MEM_DMA;

    /**
     * cfg register must configure before ts_block and
     * sar dar register
     */
    cfg_u.data = readq(&dmac->channel[channel_num].cfg);

    cfg_u.ch_cfg.tt_fc = flow_control;
    cfg_u.ch_cfg.hs_sel_src = mem_type_src ? DMAC_HS_SOFTWARE : DMAC_HS_HARDWARE;
    cfg_u.ch_cfg.hs_sel_dst = mem_type_dest ? DMAC_HS_SOFTWARE : DMAC_HS_HARDWARE;
    cfg_u.ch_cfg.src_per = channel_num;
    cfg_u.ch_cfg.dst_per = channel_num;
    cfg_u.ch_cfg.src_multblk_type = 0;
    cfg_u.ch_cfg.dst_multblk_type = 0;

    writeq(cfg_u.data, &dmac->channel[channel_num].cfg);

    dmac->channel[channel_num].sar = (uint64)src;
    dmac->channel[channel_num].dar = (uint64)dest;

    ctl.data = readq(&dmac->channel[channel_num].ctl);
    ctl.ch_ctl.sms = DMAC_MASTER1;
    ctl.ch_ctl.dms = DMAC_MASTER2;
    /* master select */
    ctl.ch_ctl.sinc = src_inc;
    ctl.ch_ctl.dinc = dest_inc;
    /* address incrememt */
    ctl.ch_ctl.src_tr_width = dmac_trans_width;
    ctl.ch_ctl.dst_tr_width = dmac_trans_width;
    /* transfer width */
    ctl.ch_ctl.src_msize = dmac_burst_size;
    ctl.ch_ctl.dst_msize = dmac_burst_size;

    writeq(ctl.data, &dmac->channel[channel_num].ctl);

    writeq(blockSize - 1, &dmac->channel[channel_num].block_ts);
    /*the number of (blcok_ts +1) data of width SRC_TR_WIDTF to be */
    /* transferred in a dma block transfer */
    return 0;
}



void dmac_init(void)
{
    uint64 tmp;
    dmac_commonreg_intclear_u_t intclear;
    dmac_cfg_u_t dmac_cfg;
    dmac_reset_u_t dmac_reset;

    sysctl_clock_enable(SYSCTL_CLOCK_DMA);
    // printf("[dmac_init] dma clk=%d\n", sysctl_clock_get_freq(SYSCTL_CLOCK_DMA));

    wait_queue_init(&dmac_waitqueue);

    dmac_reset.data = readq(&dmac->reset);
    dmac_reset.reset.rst = 1;
    writeq(dmac_reset.data, &dmac->reset);
    while(dmac_reset.reset.rst)
        dmac_reset.data = readq(&dmac->reset);

    /*reset dmac */

    intclear.data = readq(&dmac->com_intclear);
    intclear.com_intclear.clear_slvif_dec_err_intstat = 1;
    intclear.com_intclear.clear_slvif_wr2ro_err_intstat = 1;
    intclear.com_intclear.clear_slvif_rd2wo_err_intstat = 1;
    intclear.com_intclear.clear_slvif_wronhold_err_intstat = 1;
    intclear.com_intclear.clear_slvif_undefinedreg_dec_err_intstat = 1;
    writeq(intclear.data, &dmac->com_intclear);
    /* clear common register interrupt */

    dmac_cfg.data = readq(&dmac->cfg);
    dmac_cfg.cfg.dmac_en = 0;
    dmac_cfg.cfg.int_en = 0;
    writeq(dmac_cfg.data, &dmac->cfg);
    /* disable dmac and disable interrupt */

    while(readq(&dmac->cfg))
        ;
    tmp = readq(&dmac->chen);
    tmp &= ~0xf;
    writeq(tmp, &dmac->chen);
    /* disable all channel before configure */
    dmac_enable();
}

void dmac_set_single_mode(dmac_channel_number_t channel_num,
                          const void *src, void *dest, dmac_address_increment_t src_inc,
                          dmac_address_increment_t dest_inc,
                          dmac_burst_trans_length_t dmac_burst_size,
                          dmac_transfer_width_t dmac_trans_width,
                          uint64 block_size)
{
    dmac_chanel_interrupt_clear(channel_num);
    dmac_channel_disable(channel_num);
    dmac_wait_idle(channel_num);
    dmac_set_channel_param(channel_num, src, dest, src_inc, dest_inc,
                           dmac_burst_size, dmac_trans_width, block_size);
    dmac_enable();
    dmac_enable_channel_interrupt(channel_num);
    dmac_channel_enable(channel_num);
}

int dmac_is_done(dmac_channel_number_t channel_num)
{
    if(readq(&dmac->channel[channel_num].intstatus) & 0x2)
        return 1;
    else
        return 0;
}

void dmac_wait_done(dmac_channel_number_t channel_num)
{
    dmac_wait_idle(channel_num);
}

int dmac_is_idle(dmac_channel_number_t channel_num)
{
    dmac_chen_u_t chen;
    chen.data = readq(&dmac->chen);
    if((chen.data >> channel_num) & 0x1UL)
        return 0;
    else
        return 1;
}

void dmac_wait_idle(dmac_channel_number_t channel_num)
{
    while(!dmac_is_idle(channel_num)) {
        wait_queue_sleepon(&dmac_waitqueue);
    }
}

void dmac_intr(dmac_channel_number_t channel_num)
{
    dmac_chanel_interrupt_clear(channel_num);
    wait_queue_wakeup(&dmac_waitqueue);
    //dbgprintln("dmac_intr");
}