graph TD
    %% Define Styles
    classDef hardware fill:#444,stroke:#333,stroke-width:2px,color:#fff;
    classDef task fill:#0b5394,stroke:#333,stroke-width:2px,color:#fff;
    classDef buffer fill:#e69138,stroke:#333,stroke-width:2px,color:#fff;
    classDef pc fill:#274e13,stroke:#333,stroke-width:2px,color:#fff;

    %% PC / Ground Control Layer
    subgraph Ground Control Station (Python)
        FOTA_PC[fota_pusher.py]:::pc
        PLOT_PC[telemetry_plotter.py]:::pc
    end

    %% Hardware Boundaries
    BT_HW((Bluetooth Module)):::hardware
    IMU_HW((MPU6050 IMU)):::hardware
    SD_HW((SD Card / FAT32)):::hardware
    FLASH_HW((Internal Flash)):::hardware

    %% Interrupts (Top Half)
    subgraph Hardware Interrupts (ISR)
        UART_ISR[UART RX ISR]:::hardware
    end

    %% Application Layer (Bottom Half / RTOS)
    subgraph app_main.c (Super Loop / FreeRTOS)
        BT_MGR[bt_manager.c\nCommand Router]:::task
        FOTA_HND[fota_handler.c\nState Machine]:::task
        IMU_DRV[imu_driver.c\nSensor Reader]:::task
        LOG_TSK[logger_task.c\nDisk Writer]:::task
        RING_BUF[(ring_buffer.c\nLock-Free Queue)]:::buffer
    end

    %% Abstraction Layer
    subgraph Hardware Abstraction Layer (HAL)
        FLASH_IF[flash_interface.h]:::task
    end

    %% --- The FOTA Flow (Command & Update) ---
    FOTA_PC -- "1. Sends .bin chunks" --> BT_HW
    BT_HW -- "2. Triggers" --> UART_ISR
    UART_ISR -- "3. Pushes bytes" --> BT_MGR
    BT_MGR -- "4. Routes FOTA data" --> FOTA_HND
    FOTA_HND -- "5. Writes via HAL" --> FLASH_IF
    FLASH_IF -- "6. Burns to memory" --> FLASH_HW

    %% --- The Telemetry Flow (Blackbox) ---
    IMU_HW -- "A. I2C/SPI" --> IMU_DRV
    IMU_DRV -- "B. Push IMU_Data_t" --> RING_BUF
    RING_BUF -- "C. Pop IMU_Data_t" --> LOG_TSK
    LOG_TSK -- "D. f_write()" --> SD_HW
    SD_HW -. "E. Physical Card Transfer" .-> PLOT_PC

    %% Styling links for clarity
    linkStyle 0,1,2,3,4,5 stroke:#ff0000,stroke-width:2px; %% FOTA is Red
    linkStyle 6,7,8,9,10 stroke:#00ff00,stroke-width:2px; %% Telemetry is Green
