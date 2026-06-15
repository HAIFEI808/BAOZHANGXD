#include "i2s_alarm.h"
#include <math.h>
#include "stm32f4xx_ll_spi.h"

#define TONE_FREQ        1000
#define SAMPLE_RATE      16000
#define TONE_BUF_SAMPLES 256

static DMA_HandleTypeDef hdma_i2s2_tx;
static int16_t tone_buf[TONE_BUF_SAMPLES * 2];
static uint8_t is_playing = 0;

static void I2S2_GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_NOPULL;
    g.Speed     = GPIO_SPEED_FREQ_HIGH;
    g.Alternate = GPIO_AF5_SPI2;

    g.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOB, &g);
}

static void I2S2_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_i2s2_tx.Instance                 = DMA1_Stream4;
    hdma_i2s2_tx.Init.Channel             = DMA_CHANNEL_0;
    hdma_i2s2_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_i2s2_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_i2s2_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_i2s2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_i2s2_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_i2s2_tx.Init.Mode                = DMA_CIRCULAR;
    hdma_i2s2_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_i2s2_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    HAL_DMA_Init(&hdma_i2s2_tx);
}

static void I2S2_GenerateTone(void)
{
    for (int i = 0; i < TONE_BUF_SAMPLES; i++) {
        float t = (float)i / SAMPLE_RATE;
        float val = sinf(2.0f * 3.14159265f * TONE_FREQ * t);
        int16_t sample = (int16_t)(val * 16000.0f);
        tone_buf[i * 2]     = sample;
        tone_buf[i * 2 + 1] = sample;
    }
}

static void I2S2_Init(void)
{
    __HAL_RCC_SPI2_CLK_ENABLE();

    /* Disable I2S before configuration */
    SPI2->I2SCFGR &= ~SPI_I2SCFGR_I2SE;

    /* Select HSI (16MHz) as I2S clock source: CFGR I2SSRC[23:22] = 10 */
    RCC->CFGR &= ~(0x3UL << 22);
    RCC->CFGR |=  (0x2UL << 22);

    /* I2S mode, Master TX, Philips standard, 16-bit data, CPOL low */
    SPI2->I2SCFGR = SPI_I2SCFGR_I2SMOD       /* I2S mode */
                  | SPI_I2SCFGR_I2SCFG_1;     /* Master TX */

    /* Prescaler for ~16kHz audio:
     * HSI = 16MHz, bit_rate = 16000 * 32 = 512000 Hz
     * divider = 16M / 512k ≈ 31.25 → I2SDIV=15, ODD=1 → actual divider=31 */
    SPI2->I2SPR = (15 << SPI_I2SPR_I2SDIV_Pos) | SPI_I2SPR_ODD;
}

void I2S_Alarm_Init(void)
{
    I2S2_GPIO_Init();
    I2S2_DMA_Init();
    I2S2_Init();
    I2S2_GenerateTone();
}

void I2S_Alarm_Start(void)
{
    if (!is_playing) {
        LL_I2S_EnableDMAReq_TX(SPI2);
        HAL_DMA_Start(&hdma_i2s2_tx, (uint32_t)tone_buf,
                      (uint32_t)&SPI2->DR, TONE_BUF_SAMPLES * 2);
        LL_I2S_Enable(SPI2);
        is_playing = 1;
    }
}

void I2S_Alarm_Stop(void)
{
    if (is_playing) {
        LL_I2S_Disable(SPI2);
        HAL_DMA_Abort(&hdma_i2s2_tx);
        is_playing = 0;
    }
}

uint8_t I2S_Alarm_IsPlaying(void)
{
    return is_playing;
}
