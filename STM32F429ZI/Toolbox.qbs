import qbs

Project {
    property string device: "STM32F429xx"
    property bool build_CMSIS_DSP: false

    references: [
        "../../embedded-drivers/CMSIS/cmsis.qbs",
        "../../embedded-drivers/STM32F4xx_HAL_Driver"
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
        Depends {name: "STM32-HAL"}
        Depends {name: "configurations"}
        cpp.linkerScripts: ["STM32F429ZI_FLASH.ld"]
        cpp.includePaths: ["../inc"]

        files: [
            "../inc/instructions.h",
            "Inc/*",
            "Src/*",
        ]
    }
}
