deps_config := \
	/Users/R-Zaveri/esp/esp-idf/components/app_trace/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/bt/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/driver/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/esp32/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/ethernet/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/fatfs/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/freertos/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/heap/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/http_server/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/libsodium/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/log/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/lwip/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/mdns/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/openssl/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/pthread/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/spiffs/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/vfs/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/R-Zaveri/esp/esp-idf/Kconfig.compiler \
	/Users/R-Zaveri/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/R-Zaveri/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/R-Zaveri/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/R-Zaveri/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
