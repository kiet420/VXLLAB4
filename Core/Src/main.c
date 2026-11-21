/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler.h"
#include <stdio.h>
#include <string.h>
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
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
//#define DEMO_MODE 1
//#define DEMO_MODE 2
//#define DEMO_MODE 3
//#define DEMO_MODE 4
#define DEMO_MODE 5

static uint32_t id_to_delete = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void Task1(void);  void Task2(void);  void Task3(void);
void Task4(void);  void Task5(void);
void Task_Print10ms(void);
void Task_OneShot(void);
void Task_100ms(void);
void Task_500ms(void);
void Task_A(void);  void Task_B(void);  void Task_C(void);
void Task_Victim(void);
void Task_Killer(void);
static void print_time(const char *name);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  SCH_Init();

#if DEMO_MODE == 1
  // Demo 1: tick 10 ms, callback mỗi 10 ms in get_time()
  SCH_Add_Task(Task_Print10ms, 10, 10);

#elif DEMO_MODE == 2
  // Demo 2: nhiều timeout đồng thời
  SCH_Add_Task(Task_Print10ms, 10, 10);
  SCH_Add_Task(Task_100ms,     100, 100);
  SCH_Add_Task(Task_500ms,     500, 500);

#elif DEMO_MODE == 3
  // Demo 3: add lộn xộn thứ tự, kết quả phải đúng thứ tự thời gian
  SCH_Add_Task(Task_A, 3000, 0);
  SCH_Add_Task(Task_B, 1000, 0);
  SCH_Add_Task(Task_C, 2000, 0);

#elif DEMO_MODE == 4
  // Demo 4: xóa task
  id_to_delete = SCH_Add_Task(Task_Victim, 100, 100);
  SCH_Add_Task(Task_Killer, 1000, 0);

#elif DEMO_MODE == 5
  // Demo 5: bài chính
  SCH_Add_Task(Task1, 0,  500);
  SCH_Add_Task(Task2, 20, 1000);
  SCH_Add_Task(Task3, 40, 1500);
  SCH_Add_Task(Task4, 60, 2000);
  SCH_Add_Task(Task5, 80, 2500);
  SCH_Add_Task(Task_OneShot, 3000, 0);
#endif

  HAL_TIM_Base_Start_IT(&htim2);   // bật tick SAU khi đã add task
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    SCH_Dispatch_Tasks();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin
                          |LED5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin LED4_Pin
                           LED5_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin|LED4_Pin
                          |LED5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) {
    SCH_Update();
  }
}

static void print_time(const char *name)
{
  char buf[64];
  int n = snprintf(buf, sizeof(buf), "%s t=%lu ms\r\n",
                   name, (unsigned long)get_time());
  if (n > (int)sizeof(buf)) n = sizeof(buf);
  HAL_UART_Transmit(&huart1, (uint8_t*)buf, n, 50);
}

/* ---- Demo 5: bài chính ---- */
void Task1(void) { HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); print_time("T1 0.5s"); }
void Task2(void) { HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); print_time("T2 1.0s"); }
void Task3(void) { HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); print_time("T3 1.5s"); }
void Task4(void) { HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); print_time("T4 2.0s"); }
void Task5(void) { HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin); print_time("T5 2.5s"); }
void Task_OneShot(void) { print_time("ONE-SHOT"); }

/* ---- Demo 1, 2 ---- */
void Task_Print10ms(void) { print_time("10ms"); }
void Task_100ms(void)     { print_time("   100ms"); }
void Task_500ms(void)     { print_time("      500ms"); }

/* ---- Demo 3 ---- */
void Task_A(void) { print_time("A (add dau tien, delay 3000)"); }
void Task_B(void) { print_time("B (add thu hai,  delay 1000)"); }
void Task_C(void) { print_time("C (add thu ba,   delay 2000)"); }

/* ---- Demo 4 ---- */
void Task_Victim(void) { print_time("Victim"); }
void Task_Killer(void) {
  uint8_t r1 = SCH_Delete_Task(id_to_delete);   // lần 1: xóa thành công (=1)
  uint8_t r2 = SCH_Delete_Task(id_to_delete);   // lần 2: ID không còn (=0)
  char buf[64];
  int n = snprintf(buf, sizeof(buf), "DELETE: lan1=%u lan2=%u t=%lu\r\n",
                   r1, r2, (unsigned long)get_time());
  if (n > (int)sizeof(buf)) n = sizeof(buf);
  HAL_UART_Transmit(&huart1, (uint8_t*)buf, n, 50);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
