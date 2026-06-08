/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "spi.h"
#include "usart.h"
#include <string.h>
#include "event_groups.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osMutexId spiMutexHandle;
osSemaphoreId txDoneSemHandle;
EventGroupHandle_t spiEventHandle;

osThreadId spiTxTaskHandle;
osThreadId spiRxTaskHandle;

#define EVT_SENT     (1 << 0)
#define EVT_RECEIVED (1 << 1)
#define EVT_ERROR    (1 << 2)

static uint8_t tx_buf[] = "Hello ESP32";
static uint8_t rx_buf[16];
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void SpiTxTask(void const *argument);
void SpiRxTask(void const *argument);
void UART_Log(const char *msg);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  osMutexDef(spiMutex);
  spiMutexHandle = osMutexCreate(osMutex(spiMutex));
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  osSemaphoreDef(txDoneSem);
  txDoneSemHandle = osSemaphoreCreate(osSemaphore(txDoneSem), 1);
  osSemaphoreWait(txDoneSemHandle, osWaitForever); // init count = 0
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(spiTxTask, SpiTxTask, osPriorityNormal, 0, 512);
  spiTxTaskHandle = osThreadCreate(osThread(spiTxTask), NULL);

  osThreadDef(spiRxTask, SpiRxTask, osPriorityNormal, 0, 512);
  spiRxTaskHandle = osThreadCreate(osThread(spiRxTask), NULL);

  spiEventHandle = xEventGroupCreate();
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void UART_Log(const char *msg)
{
    HAL_UART_Transmit(&huart5, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void SpiTxTask(void const *argument)
{
    for (;;) {
        // acquire mutex to protect SPI bus
        osMutexWait(spiMutexHandle, osWaitForever);

        memset(rx_buf, 0, sizeof(rx_buf));
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, sizeof(tx_buf), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

        UART_Log("STM32 sent: Hello ESP32\r\n");
        xEventGroupSetBits(spiEventHandle, EVT_SENT);

        osMutexRelease(spiMutexHandle);

        // signal Rx task to process
        osSemaphoreRelease(txDoneSemHandle);

        osDelay(1000);
    }
}

void SpiRxTask(void const *argument)
{
    for (;;) {
        // wait for Tx task to finish
        osSemaphoreWait(txDoneSemHandle, osWaitForever);

        if (strncmp((char*)rx_buf, "Hello", 5) == 0) {
            UART_Log("STM32 received: ");
            UART_Log((char*)rx_buf);
            UART_Log("\r\n");
            xEventGroupSetBits(spiEventHandle, EVT_RECEIVED);
        } else {
            UART_Log("STM32 RX error\r\n");
            xEventGroupSetBits(spiEventHandle, EVT_ERROR);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_1) {
        UART_Log("EXTI PB1 triggered\r\n");
    } else if (GPIO_Pin == GPIO_PIN_0) {
        UART_Log("EXTI PB0 triggered\r\n");
    }
}
/* USER CODE END Application */
