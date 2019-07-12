
#include <string.h>
#include <stdbool.h>


#include "sysinit/sysinit.h"
#include "os/os.h"

#include <inttypes.h>
#include <mcu/mcu.h>

#include "console/console.h"

#include "os/mynewt.h"

#include "adc.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_rcc.h"
#include "stm32l1xx_hal.h"


#define ADC_MAX_VALUE                               4095
#define ADC_VREF_BANDGAP                            1224 // mV

static Adc_t Adc;


ADC_HandleTypeDef AdcHandle;


/**
  * @brief  Stop ADC conversion and disable the selected ADC
  * @note   Prerequisite condition to use this function: ADC conversions must be
  *         stopped to disable the ADC.
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef ADC_ConversionStop_Disable(ADC_HandleTypeDef* hadc)
{
  uint32_t tickstart = 0;
  
  /* Verification if ADC is not already disabled */
  if (ADC_IS_ENABLE(hadc) != RESET)
  {
    /* Disable the ADC peripheral */
    __HAL_ADC_DISABLE(hadc);
     
    /* Get tick count */
    tickstart = HAL_GetTick();
    
    /* Wait for ADC effectively disabled */    
    while(ADC_IS_ENABLE(hadc) != RESET)
    {
      if((HAL_GetTick() - tickstart ) > ADC_DISABLE_TIMEOUT)
      {
        /* Update ADC state machine to error */
        SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);
      
        /* Set ADC error code to ADC IP internal error */
        SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
        
        return HAL_ERROR;
      }
    }
  }
  
  /* Return HAL status */
  return HAL_OK;
}


/**
  * @brief  Initializes the ADC MSP.
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_MspInit must be implemented in the user file.
   */ 
}

/**
  * @brief  DeInitializes the ADC MSP.
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_MspDeInit must be implemented in the user file.
   */ 
} 


/**
  * @brief  Deinitialize the ADC peripheral registers to its default reset values.
  * @note   To not impact other ADCs, reset of common ADC registers have been
  *         left commented below.
  *         If needed, the example code can be copied and uncommented into
  *         function HAL_ADC_MspDeInit().
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check ADC handle */
  if(hadc == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Set ADC state */
  SET_BIT(hadc->State, HAL_ADC_STATE_BUSY_INTERNAL);
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  
  /* Configuration of ADC parameters if previous preliminary actions are      */ 
  /* correctly completed.                                                     */
  if (tmp_hal_status == HAL_OK)
  {
    /* ========== Reset ADC registers ========== */
    /* Reset register SR */
    __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_AWD | ADC_FLAG_JEOC | ADC_FLAG_EOC |
                                ADC_FLAG_JSTRT | ADC_FLAG_STRT));
                         
    /* Reset register CR1 */
    CLEAR_BIT(hadc->Instance->CR1, (ADC_CR1_OVRIE   | ADC_CR1_RES     | ADC_CR1_AWDEN  |
                                    ADC_CR1_JAWDEN  | ADC_CR1_PDI     | ADC_CR1_PDD    |
                                    ADC_CR1_DISCNUM | ADC_CR1_JDISCEN | ADC_CR1_DISCEN |
                                    ADC_CR1_JAUTO   | ADC_CR1_AWDSGL  | ADC_CR1_SCAN   |
                                    ADC_CR1_JEOCIE  | ADC_CR1_AWDIE   | ADC_CR1_EOCIE  |
                                    ADC_CR1_AWDCH                                       ));
    
    /* Reset register CR2 */
    ADC_CR2_CLEAR(hadc);
    
    /* Reset register SMPR0 */
    ADC_SMPR0_CLEAR(hadc);
    
    /* Reset register SMPR1 */
    ADC_SMPR1_CLEAR(hadc);
    
    /* Reset register SMPR2 */
    CLEAR_BIT(hadc->Instance->SMPR2, (ADC_SMPR2_SMP19 | ADC_SMPR2_SMP18 | ADC_SMPR2_SMP17 | 
                                      ADC_SMPR2_SMP16 | ADC_SMPR2_SMP15 | ADC_SMPR2_SMP14 | 
                                      ADC_SMPR2_SMP13 | ADC_SMPR2_SMP12 | ADC_SMPR2_SMP11 |
                                      ADC_SMPR2_SMP10                                      ));
    
    /* Reset register SMPR3 */
    CLEAR_BIT(hadc->Instance->SMPR3, (ADC_SMPR3_SMP9 | ADC_SMPR3_SMP8 | ADC_SMPR3_SMP7 | 
                                      ADC_SMPR3_SMP6 | ADC_SMPR3_SMP5 | ADC_SMPR3_SMP4 | 
                                      ADC_SMPR3_SMP3 | ADC_SMPR3_SMP2 | ADC_SMPR3_SMP1 |
                                      ADC_SMPR3_SMP0                                    ));
    
    /* Reset register JOFR1 */
    CLEAR_BIT(hadc->Instance->JOFR1, ADC_JOFR1_JOFFSET1);
    /* Reset register JOFR2 */
    CLEAR_BIT(hadc->Instance->JOFR2, ADC_JOFR2_JOFFSET2);
    /* Reset register JOFR3 */
    CLEAR_BIT(hadc->Instance->JOFR3, ADC_JOFR3_JOFFSET3);
    /* Reset register JOFR4 */
    CLEAR_BIT(hadc->Instance->JOFR4, ADC_JOFR4_JOFFSET4);
    
    /* Reset register HTR */
    CLEAR_BIT(hadc->Instance->HTR, ADC_HTR_HT);
    /* Reset register LTR */
    CLEAR_BIT(hadc->Instance->LTR, ADC_LTR_LT);
    
    /* Reset register SQR1 */
    CLEAR_BIT(hadc->Instance->SQR1, (ADC_SQR1_L | __ADC_SQR1_SQXX));
    
    /* Reset register SQR2 */
    CLEAR_BIT(hadc->Instance->SQR2, (ADC_SQR2_SQ24 | ADC_SQR2_SQ23 | ADC_SQR2_SQ22 | 
                                     ADC_SQR2_SQ21 | ADC_SQR2_SQ20 | ADC_SQR2_SQ19  ));
    
    /* Reset register SQR3 */
    CLEAR_BIT(hadc->Instance->SQR3, (ADC_SQR3_SQ18 | ADC_SQR3_SQ17 | ADC_SQR3_SQ16 | 
                                     ADC_SQR3_SQ15 | ADC_SQR3_SQ14 | ADC_SQR3_SQ13  ));
    
    /* Reset register SQR4 */
    CLEAR_BIT(hadc->Instance->SQR4, (ADC_SQR4_SQ12 | ADC_SQR4_SQ11 | ADC_SQR4_SQ10 | 
                                     ADC_SQR4_SQ9  | ADC_SQR4_SQ8  | ADC_SQR4_SQ7   ));
    
    /* Reset register SQR5 */
    CLEAR_BIT(hadc->Instance->SQR5, (ADC_SQR5_SQ6 | ADC_SQR5_SQ5 | ADC_SQR5_SQ4 | 
                                     ADC_SQR5_SQ3 | ADC_SQR5_SQ2 | ADC_SQR5_SQ1  ));
    
    
    /* Reset register JSQR */
    CLEAR_BIT(hadc->Instance->JSQR, (ADC_JSQR_JL |
                                     ADC_JSQR_JSQ4 | ADC_JSQR_JSQ3 | 
                                     ADC_JSQR_JSQ2 | ADC_JSQR_JSQ1  ));
    
    /* Reset register DR */
    /* bits in access mode read only, no direct reset applicable*/
    
    /* Reset registers JDR1, JDR2, JDR3, JDR4 */
    /* bits in access mode read only, no direct reset applicable*/
    
    /* Reset register CCR */
    CLEAR_BIT(ADC->CCR, ADC_CCR_TSVREFE);   
    
    /* ========== Hard reset ADC peripheral ========== */
    /* Performs a global reset of the entire ADC peripheral: ADC state is     */
    /* forced to a similar state after device power-on.                       */
    /* If needed, copy-paste and uncomment the following reset code into      */
    /* function "void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)":              */
    /*                                                                        */
    /*  __HAL_RCC_ADC1_FORCE_RESET()                                          */
    /*  __HAL_RCC_ADC1_RELEASE_RESET()                                        */
    
    /* DeInit the low level hardware */
    HAL_ADC_MspDeInit(hadc);
    
    /* Set ADC error code to none */
    ADC_CLEAR_ERRORCODE(hadc);
    
    /* Set ADC state */
    hadc->State = HAL_ADC_STATE_RESET;
    
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}


/**
  * @brief  Enable the selected ADC.
  * @note   Prerequisite condition to use this function: ADC must be disabled
  *         and voltage regulator must be enabled (done into HAL_ADC_Init()).
  * @note   If low power mode AutoPowerOff is enabled, power-on/off phases are
  *         performed automatically by hardware.
  *         In this mode, this function is useless and must not be called because 
  *         flag ADC_FLAG_RDY is not usable.
  *         Therefore, this function must be called under condition of
  *         "if (hadc->Init.LowPowerAutoPowerOff != ENABLE)".
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef ADC_Enable(ADC_HandleTypeDef* hadc)
{
  uint32_t tickstart = 0;
  __IO uint32_t wait_loop_index = 0;
  
  /* ADC enable and wait for ADC ready (in case of ADC is disabled or         */
  /* enabling phase not yet completed: flag ADC ready not yet set).           */
  /* Timeout implemented to not be stuck if ADC cannot be enabled (possible   */
  /* causes: ADC clock not running, ...).                                     */
  if (ADC_IS_ENABLE(hadc) == RESET)
  {
    /* Enable the Peripheral */
    __HAL_ADC_ENABLE(hadc);
    
    /* Delay for ADC stabilization time */
    /* Compute number of CPU cycles to wait for */
    wait_loop_index = (ADC_STAB_DELAY_US * (SystemCoreClock / 1000000));
    while(wait_loop_index != 0)
    {
      wait_loop_index--;
    }
    
    /* Get tick count */
    tickstart = HAL_GetTick();    

    /* Wait for ADC effectively enabled */
    while(ADC_IS_ENABLE(hadc) == RESET)
    {
      if((HAL_GetTick() - tickstart ) > ADC_ENABLE_TIMEOUT)
      {
        /* Update ADC state machine to error */
        SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);
      
        /* Set ADC error code to ADC IP internal error */
        SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
        
        /* Process unlocked */
        __HAL_UNLOCK(hadc);
      
        return HAL_ERROR;
      }
    }
  }
   
  /* Return HAL status */
  return HAL_OK;
}

/**
  * @brief  Initializes the ADC peripheral and regular group according to  
  *         parameters specified in structure "ADC_InitTypeDef".
  * @note   As prerequisite, ADC clock must be configured at RCC top level
  *         (clock source APB2).
  *         See commented example code below that can be copied and uncommented 
  *         into HAL_ADC_MspInit().
  * @note   Possibility to update parameters on the fly:
  *         This function initializes the ADC MSP (HAL_ADC_MspInit()) only when
  *         coming from ADC state reset. Following calls to this function can
  *         be used to reconfigure some parameters of ADC_InitTypeDef  
  *         structure on the fly, without modifying MSP configuration. If ADC  
  *         MSP has to be modified again, HAL_ADC_DeInit() must be called
  *         before HAL_ADC_Init().
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure 
  *         "ADC_InitTypeDef".
  * @note   This function configures the ADC within 2 scopes: scope of entire 
  *         ADC and scope of regular group. For parameters details, see comments 
  *         of structure "ADC_InitTypeDef".
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  uint32_t tmp_cr1 = 0;
  uint32_t tmp_cr2 = 0;
  
  /* Check ADC handle */
  if(hadc == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_CLOCKPRESCALER(hadc->Init.ClockPrescaler));
  assert_param(IS_ADC_RESOLUTION(hadc->Init.Resolution));
  assert_param(IS_ADC_DATA_ALIGN(hadc->Init.DataAlign)); 
  assert_param(IS_ADC_SCAN_MODE(hadc->Init.ScanConvMode));
  assert_param(IS_ADC_EOC_SELECTION(hadc->Init.EOCSelection));
  assert_param(IS_ADC_AUTOWAIT(hadc->Init.LowPowerAutoWait));
  assert_param(IS_ADC_AUTOPOWEROFF(hadc->Init.LowPowerAutoPowerOff));
  assert_param(IS_ADC_CHANNELSBANK(hadc->Init.ChannelsBank));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_EXTTRIG(hadc->Init.ExternalTrigConv));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DMAContinuousRequests));
  
  if(hadc->Init.ScanConvMode != ADC_SCAN_DISABLE)
  {
    assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));
    assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DiscontinuousConvMode));
    if(hadc->Init.DiscontinuousConvMode != DISABLE)
    {
      assert_param(IS_ADC_REGULAR_DISCONT_NUMBER(hadc->Init.NbrOfDiscConversion));
    }
  }
      
  if(hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START)
  {
    assert_param(IS_ADC_EXTTRIG_EDGE(hadc->Init.ExternalTrigConvEdge));
  }
  
  
  /* As prerequisite, into HAL_ADC_MspInit(), ADC clock must be configured    */
  /* at RCC top level.                                                        */
  /* Refer to header of this file for more details on clock enabling          */
  /* procedure.                                                               */

  /* Actions performed only if ADC is coming from state reset:                */
  /* - Initialization of ADC MSP                                              */
  if (hadc->State == HAL_ADC_STATE_RESET)
  {
    /* Initialize ADC error code */
    ADC_CLEAR_ERRORCODE(hadc);
    
    /* Allocate lock resource and initialize it */
    hadc->Lock = HAL_UNLOCKED;
    
    /* Enable SYSCFG clock to control the routing Interface (RI) */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    
    /* Init the low level hardware */
    HAL_ADC_MspInit(hadc);
  }
  
  /* Configuration of ADC parameters if previous preliminary actions are      */ 
  /* correctly completed.                                                     */
  if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL))
  {
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_INJ_BUSY,
                      HAL_ADC_STATE_BUSY_INTERNAL);
    
    /* Set ADC parameters */
    
    /* Configuration of common ADC clock: clock source HSI with selectable    */
    /* prescaler                                                              */
    MODIFY_REG(ADC->CCR                 ,
               ADC_CCR_ADCPRE           ,
               hadc->Init.ClockPrescaler );

    /* Configuration of ADC:                                                  */
    /*  - external trigger polarity                                           */
    /*  - End of conversion selection                                         */
    /*  - DMA continuous request                                              */
    /*  - Channels bank (Banks availability depends on devices categories)    */
    /*  - continuous conversion mode                                          */
    tmp_cr2 |= (hadc->Init.DataAlign                                 |
                hadc->Init.EOCSelection                              |
                ADC_CR2_DMACONTREQ(hadc->Init.DMAContinuousRequests) |
                hadc->Init.ChannelsBank                              |
                ADC_CR2_CONTINUOUS(hadc->Init.ContinuousConvMode)     );

    /* Enable external trigger if trigger selection is different of software  */
    /* start.                                                                 */
    /* Note: This configuration keeps the hardware feature of parameter       */
    /*       ExternalTrigConvEdge "trigger edge none" equivalent to           */
    /*       software start.                                                  */
    if (hadc->Init.ExternalTrigConv != ADC_SOFTWARE_START)
    {
      tmp_cr2 |= ( hadc->Init.ExternalTrigConv    |
                  hadc->Init.ExternalTrigConvEdge );
    }
    
    /* Parameters update conditioned to ADC state:                            */
    /* Parameters that can be updated only when ADC is disabled:              */
    /*  - delay selection (LowPowerAutoWait mode)                             */
    /*  - resolution                                                          */
    /*  - auto power off (LowPowerAutoPowerOff mode)                          */
    /*  - scan mode                                                           */
    /*  - discontinuous mode disable/enable                                   */
    /*  - discontinuous mode number of conversions                            */
    if ((ADC_IS_ENABLE(hadc) == RESET))
    {
      tmp_cr2 |= hadc->Init.LowPowerAutoWait;
      
      tmp_cr1 |= (hadc->Init.Resolution                     |
                  hadc->Init.LowPowerAutoPowerOff           |
                  ADC_CR1_SCAN_SET(hadc->Init.ScanConvMode)  );
      
      /* Enable discontinuous mode only if continuous mode is disabled */
      /* Note: If parameter "Init.ScanConvMode" is set to disable, parameter  */
      /*       discontinuous is set anyway, but has no effect on ADC HW.      */
      if (hadc->Init.DiscontinuousConvMode == ENABLE)
      {
        if (hadc->Init.ContinuousConvMode == DISABLE)
        {
          /* Enable the selected ADC regular discontinuous mode */
          /* Set the number of channels to be converted in discontinuous mode */
          SET_BIT(tmp_cr1, ADC_CR1_DISCEN                                            |
                           ADC_CR1_DISCONTINUOUS_NUM(hadc->Init.NbrOfDiscConversion)  );
        }
        else
        {
          /* ADC regular group settings continuous and sequencer discontinuous*/
          /* cannot be enabled simultaneously.                                */
          
          /* Update ADC state machine to error */
          SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_CONFIG);
          
          /* Set ADC error code to ADC IP internal error */
          SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
        }
      }
      
      /* Update ADC configuration register CR1 with previous settings */
        MODIFY_REG(hadc->Instance->CR1,
                   ADC_CR1_RES     |
                   ADC_CR1_PDI     |
                   ADC_CR1_PDD     |
                   ADC_CR1_DISCNUM |
                   ADC_CR1_DISCEN  |
                   ADC_CR1_SCAN     ,
                   tmp_cr1           );
    }
    
    /* Update ADC configuration register CR2 with previous settings */
    MODIFY_REG(hadc->Instance->CR2    ,
               ADC_CR2_MASK_ADCINIT() ,
               tmp_cr2                 );
    
    /* Configuration of regular group sequencer:                              */
    /* - if scan mode is disabled, regular channels sequence length is set to */
    /*   0x00: 1 channel converted (channel on regular rank 1)                */
    /*   Parameter "NbrOfConversion" is discarded.                            */
    /*   Note: Scan mode is present by hardware on this device and, if        */
    /*   disabled, discards automatically nb of conversions. Anyway, nb of    */
    /*   conversions is forced to 0x00 for alignment over all STM32 devices.  */
    /* - if scan mode is enabled, regular channels sequence length is set to  */
    /*   parameter "NbrOfConversion"                                          */
    if (ADC_CR1_SCAN_SET(hadc->Init.ScanConvMode) == ADC_SCAN_ENABLE)
    {
      MODIFY_REG(hadc->Instance->SQR1                         ,
                 ADC_SQR1_L                                   ,
                 ADC_SQR1_L_SHIFT(hadc->Init.NbrOfConversion)  );
    }
    else
    {
      MODIFY_REG(hadc->Instance->SQR1,
                 ADC_SQR1_L          ,
                 0x00000000           );
    }
    
    /* Check back that ADC registers have effectively been configured to      */
    /* ensure of no potential problem of ADC core IP clocking.                */
    /* Check through register CR2 (excluding execution control bits ADON,     */
    /* JSWSTART, SWSTART and injected trigger bits JEXTEN and JEXTSEL).       */
    if ((READ_REG(hadc->Instance->CR2) & ~(ADC_CR2_ADON |
                                           ADC_CR2_SWSTART | ADC_CR2_JSWSTART |
                                           ADC_CR2_JEXTEN  | ADC_CR2_JEXTSEL   ))
         == tmp_cr2)
    {
      /* Set ADC error code to none */
      ADC_CLEAR_ERRORCODE(hadc);
      
      /* Set the ADC state */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_BUSY_INTERNAL,
                        HAL_ADC_STATE_READY);
    }
    else
    {
      /* Update ADC state machine to error */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_BUSY_INTERNAL,
                        HAL_ADC_STATE_ERROR_INTERNAL);
      
      /* Set ADC error code to ADC IP internal error */
      SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
      
      tmp_hal_status = HAL_ERROR;
    }
    
  }
  else
  {
    tmp_hal_status = HAL_ERROR;
  }
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Configures the the selected channel to be linked to the regular
  *         group.
  * @note   In case of usage of internal measurement channels:
  *         Vbat/VrefInt/TempSensor.
  *         These internal paths can be be disabled using function 
  *         HAL_ADC_DeInit().
  * @note   Possibility to update parameters on the fly:
  *         This function initializes channel into regular group, following  
  *         calls to this function can be used to reconfigure some parameters 
  *         of structure "ADC_ChannelConfTypeDef" on the fly, without reseting 
  *         the ADC.
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure 
  *         "ADC_ChannelConfTypeDef".
  * @param  hadc: ADC handle
  * @param  sConfig: Structure of ADC channel for regular group.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  __IO uint32_t wait_loop_index = 0;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_CHANNEL(sConfig->Channel));
  assert_param(IS_ADC_REGULAR_RANK(sConfig->Rank));
  assert_param(IS_ADC_SAMPLE_TIME(sConfig->SamplingTime));
  
  /* Process locked */
  __HAL_LOCK(hadc);
  
   
  /* Regular sequence configuration */
  /* For Rank 1 to 6 */
  if (sConfig->Rank < 7)
  {
    MODIFY_REG(hadc->Instance->SQR5,
               ADC_SQR5_RK(ADC_SQR5_SQ1, sConfig->Rank),
               ADC_SQR5_RK(sConfig->Channel, sConfig->Rank) );
  }
  /* For Rank 7 to 12 */
  else if (sConfig->Rank < 13)
  {
    MODIFY_REG(hadc->Instance->SQR4,
               ADC_SQR4_RK(ADC_SQR4_SQ7, sConfig->Rank),
               ADC_SQR4_RK(sConfig->Channel, sConfig->Rank) );
  }
  /* For Rank 13 to 18 */
  else if (sConfig->Rank < 19)
  {
    MODIFY_REG(hadc->Instance->SQR3,
               ADC_SQR3_RK(ADC_SQR3_SQ13, sConfig->Rank),
               ADC_SQR3_RK(sConfig->Channel, sConfig->Rank) );
  }
  /* For Rank 19 to 24 */
  else if (sConfig->Rank < 25)
  {
    MODIFY_REG(hadc->Instance->SQR2,
               ADC_SQR2_RK(ADC_SQR2_SQ19, sConfig->Rank),
               ADC_SQR2_RK(sConfig->Channel, sConfig->Rank) );
  }
  /* For Rank 25 to 28 */
  else
  {
    MODIFY_REG(hadc->Instance->SQR1,
               ADC_SQR1_RK(ADC_SQR1_SQ25, sConfig->Rank),
               ADC_SQR1_RK(sConfig->Channel, sConfig->Rank) );
  }
  
  
  /* Channel sampling time configuration */
  /* For channels 0 to 9 */
  if (sConfig->Channel < ADC_CHANNEL_10)
  {
    MODIFY_REG(hadc->Instance->SMPR3,
               ADC_SMPR3(ADC_SMPR3_SMP0, sConfig->Channel),
               ADC_SMPR3(sConfig->SamplingTime, sConfig->Channel) );
  }
  /* For channels 10 to 19 */
  else if (sConfig->Channel < ADC_CHANNEL_20)
  {
    MODIFY_REG(hadc->Instance->SMPR2,
               ADC_SMPR2(ADC_SMPR2_SMP10, sConfig->Channel),
               ADC_SMPR2(sConfig->SamplingTime, sConfig->Channel) );
  }
  /* For channels 20 to 26 for devices Cat.1, Cat.2, Cat.3 */
  /* For channels 20 to 29 for devices Cat4, Cat.5 */
  else if (sConfig->Channel <= ADC_SMPR1_CHANNEL_MAX)
  {
    MODIFY_REG(hadc->Instance->SMPR1,
               ADC_SMPR1(ADC_SMPR1_SMP20, sConfig->Channel),
               ADC_SMPR1(sConfig->SamplingTime, sConfig->Channel) );
  }
  /* For channels 30 to 31 for devices Cat4, Cat.5 */
  else
  {
    ADC_SMPR0_CHANNEL_SET(hadc, sConfig->SamplingTime, sConfig->Channel);
  }

  /* If ADC1 Channel_16 or Channel_17 is selected, enable Temperature sensor  */
  /* and VREFINT measurement path.                                            */
  if ((sConfig->Channel == ADC_CHANNEL_TEMPSENSOR) ||
      (sConfig->Channel == ADC_CHANNEL_VREFINT)      )
  {
      if (READ_BIT(ADC->CCR, ADC_CCR_TSVREFE) == RESET)
      {
        SET_BIT(ADC->CCR, ADC_CCR_TSVREFE);
        
        if ((sConfig->Channel == ADC_CHANNEL_TEMPSENSOR))
        {
          /* Delay for temperature sensor stabilization time */
          /* Compute number of CPU cycles to wait for */
          wait_loop_index = (ADC_TEMPSENSOR_DELAY_US * (SystemCoreClock / 1000000));
          while(wait_loop_index != 0)
          {
            wait_loop_index--;
          }
        }
    }
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}


/**
  * @brief  Enables ADC, starts conversion of regular group.
  *         Interruptions enabled in this function: None.
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Enable the ADC peripheral */
  tmp_hal_status = ADC_Enable(hadc);
  
  /* Start conversion if ADC is effectively enabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Set ADC state                                                          */
    /* - Clear state bitfield related to regular group conversion results     */
    /* - Set state bitfield related to regular group operation                */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR,
                      HAL_ADC_STATE_REG_BUSY);
    
    /* If conversions on group regular are also triggering group injected,    */
    /* update ADC state.                                                      */
    if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
    {
      ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);  
    }
    
    /* State machine update: Check if an injected conversion is ongoing */
    if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    {
      /* Reset ADC error code fields related to conversions on group regular */
      CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));         
    }
    else
    {
      /* Reset ADC all error code fields */
      ADC_CLEAR_ERRORCODE(hadc);
    }
    
    /* Process unlocked */
    /* Unlock before starting ADC conversions: in case of potential           */
    /* interruption, to let the process to ADC IRQ Handler.                   */
    __HAL_UNLOCK(hadc);
    
    /* Clear regular group conversion flag and overrun flag */
    /* (To ensure of no unknown state from potential previous ADC operations) */
    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC | ADC_FLAG_OVR);
    
    /* Enable conversion of regular group.                                    */
    /* If software start has been selected, conversion starts immediately.    */
    /* If external trigger has been selected, conversion will start at next   */
    /* trigger event.                                                         */
    if (ADC_IS_SOFTWARE_START_REGULAR(hadc))
    {
      /* Start ADC conversion on regular group */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_SWSTART);
    }
  }
  
  /* Return function status */
  return tmp_hal_status;
}


/**
  * @brief  Wait for regular group conversion to be completed.
  * @note   ADC conversion flags EOS (end of sequence) and EOC (end of
  *         conversion) are cleared by this function, with an exception:
  *         if low power feature "LowPowerAutoWait" is enabled, flags are 
  *         not cleared to not interfere with this feature until data register
  *         is read using function HAL_ADC_GetValue().
  * @note   This function cannot be used in a particular setup: ADC configured 
  *         in DMA mode and polling for end of each conversion (ADC init
  *         parameter "EOCSelection" set to ADC_EOC_SINGLE_CONV).
  *         In this case, DMA resets the flag EOC and polling cannot be
  *         performed on each conversion. Nevertheless, polling can still 
  *         be performed on the complete sequence (ADC init
  *         parameter "EOCSelection" set to ADC_EOC_SEQ_CONV).
  * @param  hadc: ADC handle
  * @param  Timeout: Timeout value in millisecond.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout)
{
  uint32_t tickstart = 0;
 
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Verification that ADC configuration is compliant with polling for      */
  /* each conversion:                                                       */
  /* Particular case is ADC configured in DMA mode and ADC sequencer with   */
  /* several ranks and polling for end of each conversion.                  */
  /* For code simplicity sake, this particular case is generalized to       */
  /* ADC configured in DMA mode and and polling for end of each conversion. */
  if (HAL_IS_BIT_SET(hadc->Instance->CR2, ADC_CR2_EOCS) &&
      HAL_IS_BIT_SET(hadc->Instance->CR2, ADC_CR2_DMA)    )
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_CONFIG);
    
    /* Process unlocked */
    __HAL_UNLOCK(hadc);
    
    return HAL_ERROR;
  }

  /* Get tick count */
  tickstart = HAL_GetTick();
  
  /* Wait until End of Conversion flag is raised */
  while(HAL_IS_BIT_CLR(hadc->Instance->SR, ADC_FLAG_EOC))
  {
    /* Check if timeout is disabled (set to infinite wait) */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0) || ((HAL_GetTick() - tickstart ) > Timeout))
      {
        /* Update ADC state machine to timeout */
        SET_BIT(hadc->State, HAL_ADC_STATE_TIMEOUT);
        
        /* Process unlocked */
        __HAL_UNLOCK(hadc);
        
        return HAL_TIMEOUT;
      }
    }
  }
  
  /* Clear end of conversion flag of regular group if low power feature     */
  /* "Auto Wait" is disabled, to not interfere with this feature until data */
  /* register is read using function HAL_ADC_GetValue().                    */
  if (hadc->Init.LowPowerAutoWait == DISABLE)
  {
    /* Clear regular group conversion flag */
    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_STRT | ADC_FLAG_EOC);
  }
  
  /* Update ADC state machine */
  SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);
  
  /* Determine whether any further conversion upcoming on group regular       */
  /* by external trigger, continuous mode or scan sequence on going.          */
  /* Note: On STM32L1, there is no independent flag of end of sequence.       */
  /*       The test of scan sequence on going is done either with scan        */
  /*       sequence disabled or with end of conversion flag set to            */
  /*       of end of sequence.                                                */
  if(ADC_IS_SOFTWARE_START_REGULAR(hadc)                   &&
     (hadc->Init.ContinuousConvMode == DISABLE)            &&
     (HAL_IS_BIT_CLR(hadc->Instance->SQR1, ADC_SQR1_L) ||
      HAL_IS_BIT_CLR(hadc->Instance->CR2, ADC_CR2_EOCS)  )   )
  {
    /* Set ADC state */
    CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);   
    
    if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    { 
      SET_BIT(hadc->State, HAL_ADC_STATE_READY);
    }
  }
  
  /* Return ADC state */
  return HAL_OK;
}


/**
  * @brief  Get ADC regular group conversion result.
  * @note   Reading register DR automatically clears ADC flag EOC
  *         (ADC group regular end of unitary conversion).
  * @note   This function does not clear ADC flag EOS 
  *         (ADC group regular end of sequence conversion).
  *         Occurrence of flag EOS rising:
  *          - If sequencer is composed of 1 rank, flag EOS is equivalent
  *            to flag EOC.
  *          - If sequencer is composed of several ranks, during the scan
  *            sequence flag EOC only is raised, at the end of the scan sequence
  *            both flags EOC and EOS are raised.
  *         To clear this flag, either use function: 
  *         in programming model IT: @ref HAL_ADC_IRQHandler(), in programming
  *         model polling: @ref HAL_ADC_PollForConversion() 
  *         or @ref __HAL_ADC_CLEAR_FLAG(&hadc, ADC_FLAG_EOS).
  * @param  hadc: ADC handle
  * @retval ADC group regular conversion data
  */
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef* hadc)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));

  /* Note: EOC flag is not cleared here by software because automatically     */
  /*       cleared by hardware when reading register DR.                      */
  
  /* Return ADC converted value */ 
  return hadc->Instance->DR;
}

/**
  * @brief  Handles ADC interrupt request  
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_IRQHandler(ADC_HandleTypeDef* hadc)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));

  
  /* ========== Check End of Conversion flag for regular group ========== */
  if(__HAL_ADC_GET_IT_SOURCE(hadc, ADC_IT_EOC))
  {
    if(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC) )
    {
      /* Update state machine on conversion status if not in error state */
      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL))
      {
        /* Set ADC state */
        SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC); 
      }

      /* Determine whether any further conversion upcoming on group regular   */
      /* by external trigger, continuous mode or scan sequence on going.      */
      /* Note: On STM32L1, there is no independent flag of end of sequence.   */
      /*       The test of scan sequence on going is done either with scan    */
      /*       sequence disabled or with end of conversion flag set to        */
      /*       of end of sequence.                                            */
      if(ADC_IS_SOFTWARE_START_REGULAR(hadc)                   &&
         (hadc->Init.ContinuousConvMode == DISABLE)            &&
         (HAL_IS_BIT_CLR(hadc->Instance->SQR1, ADC_SQR1_L) || 
          HAL_IS_BIT_CLR(hadc->Instance->CR2, ADC_CR2_EOCS)  )   )
      {
        /* Disable ADC end of single conversion interrupt on group regular */
        /* Note: Overrun interrupt was enabled with EOC interrupt in          */
        /* HAL_ADC_Start_IT(), but is not disabled here because can be used   */
        /* by overrun IRQ process below.                                      */
        __HAL_ADC_DISABLE_IT(hadc, ADC_IT_EOC);
        
        /* Set ADC state */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);
        
        if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
        {
          SET_BIT(hadc->State, HAL_ADC_STATE_READY);
        }
      }

      /* Conversion complete callback */
      HAL_ADC_ConvCpltCallback(hadc);
      
      /* Clear regular group conversion flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_STRT | ADC_FLAG_EOC);
    }
  }

  /* ========== Check End of Conversion flag for injected group ========== */
  if(__HAL_ADC_GET_IT_SOURCE(hadc, ADC_IT_JEOC))
  {
    if(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_JEOC))
    {
      /* Update state machine on conversion status if not in error state */
      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL))
      {
        /* Set ADC state */
        SET_BIT(hadc->State, HAL_ADC_STATE_INJ_EOC);
      }

      /* Determine whether any further conversion upcoming on group injected  */
      /* by external trigger, scan sequence on going or by automatic injected */
      /* conversion from group regular (same conditions as group regular      */
      /* interruption disabling above).                                       */
      if(ADC_IS_SOFTWARE_START_INJECTED(hadc)                    &&
         (HAL_IS_BIT_CLR(hadc->Instance->JSQR, ADC_JSQR_JL)  ||
          HAL_IS_BIT_CLR(hadc->Instance->CR2, ADC_CR2_EOCS)    ) &&
         (HAL_IS_BIT_CLR(hadc->Instance->CR1, ADC_CR1_JAUTO) &&
          (ADC_IS_SOFTWARE_START_REGULAR(hadc)       &&
          (hadc->Init.ContinuousConvMode == DISABLE)   )       )   )
      {
        /* Disable ADC end of single conversion interrupt on group injected */
        __HAL_ADC_DISABLE_IT(hadc, ADC_IT_JEOC);
        
        /* Set ADC state */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_INJ_BUSY);   

        if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_REG_BUSY))
        { 
          SET_BIT(hadc->State, HAL_ADC_STATE_READY);
        }
      }

      /* Conversion complete callback */ 
      HAL_ADCEx_InjectedConvCpltCallback(hadc);
      
      /* Clear injected group conversion flag */
      __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_JSTRT | ADC_FLAG_JEOC));
    }
  }
   
  /* ========== Check Analog watchdog flags ========== */
  if(__HAL_ADC_GET_IT_SOURCE(hadc, ADC_IT_AWD))
  {
    if(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_AWD))
    {
      /* Set ADC state */
      SET_BIT(hadc->State, HAL_ADC_STATE_AWD1);
      
      /* Level out of window callback */ 
      HAL_ADC_LevelOutOfWindowCallback(hadc);
      
      /* Clear the ADC analog watchdog flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_AWD);
    }
  }
  
  /* ========== Check Overrun flag ========== */
  if(__HAL_ADC_GET_IT_SOURCE(hadc, ADC_IT_OVR))
  {
    if(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_OVR))
    {
      /* Note: On STM32L1, ADC overrun can be set through other parameters    */
      /*       refer to description of parameter "EOCSelection" for more      */
      /*       details.                                                       */
      
      /* Set ADC error code to overrun */
      SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_OVR);
      
      /* Clear ADC overrun flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_OVR);
      
      /* Error callback */ 
      HAL_ADC_ErrorCallback(hadc);
      
      /* Clear the Overrun flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_OVR);
    }
  }
  
}


static void AdcMcuInit( Adc_t *obj0, PinNames adcInput0 )
{
    AdcHandle.Instance = ( ADC_TypeDef* )ADC1_BASE;

    __HAL_RCC_ADC1_CLK_ENABLE( );

    HAL_ADC_DeInit( &AdcHandle );

    if( adcInput0 != NC )
    {
        GpioInit( &obj0->AdcInput, adcInput0, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
}

static void AdcMcuConfig( void )
{
    // Configure ADC
    AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.ContinuousConvMode    = DISABLE;
    AdcHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T6_TRGO;
    AdcHandle.Init.DMAContinuousRequests = DISABLE;
    AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    AdcHandle.Init.NbrOfConversion       = 1;
    AdcHandle.Init.LowPowerAutoWait      = DISABLE;
    AdcHandle.Init.LowPowerAutoPowerOff  = DISABLE;
    HAL_ADC_Init( &AdcHandle );
}

static uint16_t AdcMcuReadChannel( Adc_t *obj, uint32_t channel )
{
  ADC_ChannelConfTypeDef adcConf = { 0 };
  uint16_t adcData = 0;

  // Enable HSI
  __HAL_RCC_HSI_ENABLE( );

  // Wait till HSI is ready
  while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSIRDY ) == RESET )
  {
  }

  __HAL_RCC_ADC1_CLK_ENABLE( );

  adcConf.Channel = channel;
  adcConf.Rank = ADC_REGULAR_RANK_1;
  adcConf.SamplingTime = ADC_SAMPLETIME_192CYCLES;

  HAL_ADC_ConfigChannel( &AdcHandle, &adcConf );

  // Enable ADC1
  __HAL_ADC_ENABLE( &AdcHandle );

  // Start ADC Software Conversion
  HAL_ADC_Start( &AdcHandle );

  HAL_ADC_PollForConversion( &AdcHandle, HAL_MAX_DELAY );

  adcData = HAL_ADC_GetValue( &AdcHandle );

  __HAL_ADC_DISABLE( &AdcHandle );

  if( ( adcConf.Channel == ADC_CHANNEL_TEMPSENSOR ) || ( adcConf.Channel == ADC_CHANNEL_VREFINT ) )
  {
      HAL_ADC_DeInit( &AdcHandle );
  }
  __HAL_RCC_ADC1_CLK_DISABLE( );

  // Disable HSI
  __HAL_RCC_HSI_DISABLE( );

  return adcData;
}
/*!
 * Flag to indicates if the ADC is initialized
 */

static bool AdcInitialized = false;

static void AdcInit( Adc_t *obj, PinNames adcInput )
{
    if( AdcInitialized == false )
    {
    AdcInitialized = true;

    AdcMcuInit( obj, adcInput );
    AdcMcuConfig( );
    }
}

void AdcDeInit( Adc_t *obj )
{
    AdcInitialized = false;
}

static uint16_t AdcReadChannel( Adc_t *obj, uint32_t channel )
{
    if( AdcInitialized == true )
    {
      return AdcMcuReadChannel( obj, channel );
    }
    else
    {
      return 0;
    }
}




uint16_t BoardBatteryMeasureVolage(void)
{
   uint16_t vref = 0;
   uint32_t batteryVoltage = 0;
   // Init
   
   AdcInit(&Adc, NC);
   // Read the current Voltage
   vref = AdcReadChannel( &Adc , ADC_CHANNEL_17 );

   // We don't use the VREF from calibValues here.
   // calculate the Voltage in millivolt
   batteryVoltage = ( uint32_t )ADC_VREF_BANDGAP * ( uint32_t )ADC_MAX_VALUE;
   batteryVoltage = batteryVoltage / ( uint32_t )vref;
   
   return batteryVoltage;
}
