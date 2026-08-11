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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "modbus_rtu.h"
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

/* USER CODE BEGIN PV */
uint32_t last_modbus_poll = 0;
uint8_t current_slave_address = 1; // Default PLC slave address for Modbus RTU

// Test variables for Virtual I/O via Modbus
uint8_t test_state = 0; 
uint16_t virtual_inputs[2] = {0}; // 32 bits max (for 18 inputs)
uint16_t virtual_outputs[2] = {0}; // 32 bits max (for 18 outputs)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  Modbus_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // Hàm này phải được gọi liên tục để kiểm tra xem đã nhận xong 1 frame Modbus chưa
    // Nếu nhận xong (timeout > 5ms), nó sẽ tự động phân tích dữ liệu và cập nhật vào plc_registers
    Modbus_Process();

    // Khối lệnh Test Sequence (Chạy luân phiên mỗi 500ms)
    // Dùng để tạo ra chu kỳ Test: Ép Input ảo -> Đọc Output ảo
    if (HAL_GetTick() - last_modbus_poll >= 500)
    {
        last_modbus_poll = HAL_GetTick();
        
        if (test_state == 0)
        {
            // [TRẠNG THÁI 0: GHI TÍN HIỆU INPUT ẢO XUỐNG PLC]
            // Đọc trạng thái 8 nút bấm vật lý trên bo STM32 và ghép vào virtual_inputs[0]
            virtual_inputs[0] = 0; // Xóa dữ liệu cũ
            if (HAL_GPIO_ReadPin(I1_GPIO_Port, I1_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 0);
            if (HAL_GPIO_ReadPin(I2_GPIO_Port, I2_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 1);
            if (HAL_GPIO_ReadPin(I3_GPIO_Port, I3_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 2);
            if (HAL_GPIO_ReadPin(I4_GPIO_Port, I4_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 3);
            if (HAL_GPIO_ReadPin(I5_GPIO_Port, I5_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 4);
            if (HAL_GPIO_ReadPin(I6_GPIO_Port, I6_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 5);
            if (HAL_GPIO_ReadPin(I7_GPIO_Port, I7_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 6);
            if (HAL_GPIO_ReadPin(I8_GPIO_Port, I8_Pin) == GPIO_PIN_SET) virtual_inputs[0] |= (1 << 7);
            
            // Đặt các bit ảo khác nếu cần (Ví dụ PLC có 18 Input, ta chỉ có 8 nút thật)
            virtual_inputs[1] = 0x0000; 
            
            // Gửi lệnh Modbus (Mã 0x10) ghi 2 thanh ghi (32 bit) vào địa chỉ D0 của PLC
            Modbus_WriteMultipleRegisters(current_slave_address, 0x0000, 2, virtual_inputs);
            
            // Chuyển sang trạng thái đọc ở chu kỳ 500ms tiếp theo
            test_state = 1; 
        }
        else if (test_state == 1)
        {
            // [TRẠNG THÁI 1: ĐỌC KẾT QUẢ OUTPUT TỪ PLC VỀ KIỂM TRA]
            // Gửi lệnh Modbus (Mã 0x03) yêu cầu PLC trả về giá trị của 2 thanh ghi bắt đầu từ D10
            // D10 và D11 trên PLC sẽ do chương trình Ladder ghi trạng thái của 18 tín hiệu Output vào
            Modbus_ReadHoldingRegisters(current_slave_address, 0x000A, 2);
            
            // Dữ liệu PLC trả về sẽ được Modbus_Process() ngầm xử lý và bật cờ modbus_rx_ready = 1
            if (modbus_rx_ready)
            {
                // Lấy kết quả lưu vào mảng của STM32
                virtual_outputs[0] = plc_registers[0]; // Output 1-16 (từ D10)
                virtual_outputs[1] = plc_registers[1]; // Output 17-18 (từ D11)
                
                // Cập nhật trạng thái 8 LED/Relay vật lý trên STM32 tương ứng với 8 Output ảo đầu tiên
                HAL_GPIO_WritePin(O1_GPIO_Port, O1_Pin, (virtual_outputs[0] & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O2_GPIO_Port, O2_Pin, (virtual_outputs[0] & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O3_GPIO_Port, O3_Pin, (virtual_outputs[0] & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O4_GPIO_Port, O4_Pin, (virtual_outputs[0] & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O5_GPIO_Port, O5_Pin, (virtual_outputs[0] & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O6_GPIO_Port, O6_Pin, (virtual_outputs[0] & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O7_GPIO_Port, O7_Pin, (virtual_outputs[0] & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(O8_GPIO_Port, O8_Pin, (virtual_outputs[0] & (1 << 7)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

                modbus_rx_ready = 0; // Xóa cờ báo hiệu đã đọc xong
            }
            
            // Quay lại trạng thái 0 để thực hiện bài test (Input) tiếp theo
            test_state = 0; 
        }
    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
