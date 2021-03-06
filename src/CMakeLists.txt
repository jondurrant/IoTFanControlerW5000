add_executable(${NAME}
		main.cpp
		FanState.cpp
		RGBLEDMgr.cpp
		FanController.cpp
		OledDisplay.cpp
		DisplayAgent.cpp
		RotEncAgent.cpp
		RotEncListener.cpp
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${NAME} 
	pico_stdlib 
	hardware_spi
	hardware_dma
	hardware_i2c
	pico_unique_id
	pico_ssd1306
	ETHERNET_FILES
	DHCP_FILES
	DNS_FILES
	SNTP_FILES
	TIMER_FILES
	W5100S_FILES
	FreeRTOS 
	twinThingRP2040W5x00 
	json_maker 
	tiny_json
	hardware_rtc
	RTOSComponents
	IOLIBRARY_FILES
	coreMQTT 
	coreMQTTAgent
	ds18b20pio
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})