import qbs

Project {
    property string device: "stm32f429"
    property bool build_CMSIS_DSP: false

    references: [
        "../../Drivers/CMSIS/cmsis.qbs",
        "../../Drivers/STM32F4xx_HAL_Driver"
    ]

    Product {
        name: "configurations"

        Export {
            Depends {name: "cpp"}
            cpp.includePaths: ["Inc"]

            Properties {
                condition: qbs.buildVariant == "release"
                cpp.optimization: "small"
            }
        }

        files: [
            "Inc/stm32f4xx_hal_conf.h",
        ]
    }

    CppApplication {
        Depends {name: "CMSIS"}
        Depends {name: "stm32f4-HAL"}
        Depends {name: "configurations"}
        cpp.linkerScripts: ["STM32F429ZI_FLASH.ld"]
        cpp.includePaths: ["../inc"]

        files: [
            "STM32F429ZI_FLASH.ld",
            "../inc/instructions.h",
            "Inc/adc.h",
            "Inc/ctrl.h",
            "Inc/dac.h",
            "Inc/handles.h",
            "Inc/info.h",
            "Inc/stm32f4xx_it.h",
            "Inc/timer.h",
            "Inc/uart.h",
            "Src/adc.c",
            "Src/ctrl.c",
            "Src/dac.c",
            "Src/main.c",
            "Src/newlib_stubs.c",
            "Src/stm32f4xx_hal_msp.c",
            "Src/stm32f4xx_it.c",
            "Src/timer.c",
            "Src/uart.c",
        ]
    }
}
