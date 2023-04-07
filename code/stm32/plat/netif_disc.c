#include "netif_disc.h"
#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "ether.h"

static netif_t * netif_disc;            // 只支持这一个

net_err_t netif_disc_open (struct _netif_t* netif, void * data) {
    netif_disc_data_t * ops_data = (netif_disc_data_t *)data;

    netif->ops_data = ops_data;             // 暂存起来
    netif->type = NETIF_TYPE_ETHER;         // 以太网类型
    netif->mtu = ETH_MTU;                   // 1500字节
    netif_set_hwaddr(netif, ops_data->hwaddr, NETIF_HWADDR_SIZE);
	
    // TODO: 更改错误码
	if(ETH_Mem_Malloc()) {
        return NET_ERR_IO;
    }
    
	if(LAN8720_Init()) {
        return NET_ERR_IO;	
    }

    // 配置以太网MAC地址
	ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr.addr);

    // 初始化DMA收发缓存
	ETH_DMATxDescChainInit(DMATxDscrTab, Tx_Buff, ETH_TXBUFNB);
	ETH_DMARxDescChainInit(DMARxDscrTab, Rx_Buff, ETH_RXBUFNB);

     //开启MAC和DMA
	ETH_Start();

    netif_disc = netif;
    return NET_ERR_OK;
}

void netif_disc_close (struct _netif_t* netif) {

}

/**
 * 网卡中断处理
 * 主要处理接收中断和发送中断
 */
void ETH_IRQHandler(void) {
    // 接收中断处理
    if (ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) {
	    while(ETH_GetRxPktSize(DMARxDescToGet) != 0) {
            FrameTypeDef frame = ETH_Get_Received_Frame_interrupt();

            // 分配数据包之后填充进去
            pktbuf_t * pktbuf = pktbuf_alloc(frame.length);
            if (pktbuf) {
                pktbuf_write(pktbuf, (uint8_t *)frame.buffer, frame.length);

                // 写入网络接口的接收队列
                net_err_t err = netif_put_in(netif_disc, pktbuf, -1);
                if (err < 0) {
                    pktbuf_free(pktbuf);
                }
            }

            __IO ETH_DMADESCTypeDef *DMARxNextDesc;
            if (DMA_RX_FRAME_infos->Seg_Count > 1) {
                DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
            } else {
                DMARxNextDesc = frame.descriptor;
            }
            
            /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
            for (int i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++) {  
                DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
                DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
            }
            
            /* Clear Segment_Count */
            DMA_RX_FRAME_infos->Seg_Count =0;     

            // 已经使用完毕，所以这里可以释放掉了
            frame.descriptor->Status=ETH_DMARxDesc_OWN; 
            if((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET) {
                ETH->DMASR=ETH_DMASR_RBUS;
                ETH->DMARPDR=0;
            }
        }
        
        // 清除接收中断
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    }

    // 发送中断处理
    if (ETH_GetDMAFlagStatus(ETH_DMA_FLAG_T) == SET) {
        // 每次尽可能取出所有数据包之后填满
        while((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) == (u32)RESET) {    
            // 取发送缓存和可发送的包
            uint8_t * buffer=(uint8_t *)ETH_GetCurrentTxBuffer(); 
            pktbuf_t* pktbuf = netif_get_out(netif_disc, -1);
            if (pktbuf == (pktbuf_t *)0) {
                break;
            }

            // 数据拷贝到发送缓存中
            int size = pktbuf_total(pktbuf);
            pktbuf_reset_acc(pktbuf);
            pktbuf_read(pktbuf, buffer, size);
            pktbuf_free(pktbuf);

            // 启动发送过程
            ETH_Tx_Packet(size);
        }

        // 清除发送中断
        ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
    }
    
    // 清除普通中断
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}  

net_err_t netif_disc_xmit (struct _netif_t* netif) {
    // 当硬件正在发送时，不用再启动发送。
    if (ETH_GetTransmitProcessState() != ETH_DMA_TransmitProcess_Stopped) {
        return NET_ERR_OK;
    }

    // 取发送缓存和可发送的包
    uint8_t * buffer=(uint8_t *)ETH_GetCurrentTxBuffer(); 
    pktbuf_t* pktbuf = netif_get_out(netif, -1);
    if (pktbuf) {
        // 数据拷贝到发送缓存中
        int size = pktbuf_total(pktbuf);
        pktbuf_reset_acc(pktbuf);
        pktbuf_read(pktbuf, buffer, size);
        pktbuf_free(pktbuf);

        // 启动发送过程
        ETH_Tx_Packet(size);
    }

	return NET_ERR_OK;
}

const netif_ops_t netif_disc_ops = {
    .open = netif_disc_open,
    .close = netif_disc_close,
    .xmit = netif_disc_xmit,
};
