// mcp4162.h - MCP4162 Digital Potentiometer Driver for ESPHome (ESP-IDF)
// Single 5kΩ Non-Volatile Rheostat für Ni1000 Raumfühler-Simulation
// 
// Vorteil MCP4162: Behält Wert bei Stromausfall (EEPROM)!
//
// Pinbelegung MCP4162-502E/P (DIP-8):
// Pin 1: CS   → GPIO5
// Pin 2: SCK  → GPIO18
// Pin 3: SDI  → GPIO23
// Pin 4: VSS  → GND
// Pin 5: P0W  → Schleifer (über 500Ω||220Ω parallel zu P0B, dann 1kΩ zu Klemme B5)
// Pin 6: P0B  → Klemme M (Heizung)
// Pin 7: SDO  → nicht verwendet
// Pin 8: VDD  → 5V (WICHTIG: nicht 3.3V, da Heizung 5V auf Sensorleitung!)

#pragma once
#include "esphome.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define MCP4162_WIPER0_ADDR  0x00

class MCP4162 {
  private:
    gpio_num_t cs_pin;
    gpio_num_t clk_pin;
    gpio_num_t mosi_pin;
    bool initialized;
    spi_device_handle_t spi_handle;
    
  public:
    MCP4162(int cs = 5, int clk = 18, int mosi = 23) 
      : cs_pin((gpio_num_t)cs), 
        clk_pin((gpio_num_t)clk), 
        mosi_pin((gpio_num_t)mosi),
        initialized(false),
        spi_handle(nullptr) {}
    
    void begin() {
      if (initialized) return;
      
      // SPI Bus Konfiguration
      spi_bus_config_t bus_config = {};
      bus_config.mosi_io_num = mosi_pin;
      bus_config.miso_io_num = -1;  // Nicht benötigt für Write-Only
      bus_config.sclk_io_num = clk_pin;
      bus_config.quadwp_io_num = -1;
      bus_config.quadhd_io_num = -1;
      bus_config.max_transfer_sz = 4;
      
      // SPI Device Konfiguration
      spi_device_interface_config_t dev_config = {};
      dev_config.command_bits = 0;
      dev_config.address_bits = 0;
      dev_config.dummy_bits = 0;
      dev_config.mode = 0;  // SPI Mode 0
      dev_config.duty_cycle_pos = 128;
      dev_config.cs_ena_pretrans = 0;
      dev_config.cs_ena_posttrans = 0;
      dev_config.clock_speed_hz = 1000000;  // 1 MHz
      dev_config.input_delay_ns = 0;
      dev_config.spics_io_num = cs_pin;
      dev_config.flags = 0;
      dev_config.queue_size = 1;
      dev_config.pre_cb = nullptr;
      dev_config.post_cb = nullptr;
      
      // SPI Bus initialisieren
      esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_DISABLED);
      if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE("mcp4162", "SPI Bus Init fehlgeschlagen: %d", ret);
        return;
      }
      
      // Device hinzufügen
      ret = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_handle);
      if (ret != ESP_OK) {
        ESP_LOGE("mcp4162", "SPI Device Init fehlgeschlagen: %d", ret);
        return;
      }
      
      initialized = true;
      ESP_LOGI("mcp4162", "MCP4162 initialisiert (CS=%d, CLK=%d, MOSI=%d)", 
               cs_pin, clk_pin, mosi_pin);
    }
    
    void setWiper(uint16_t value) {
      if (!initialized) {
        begin();
        if (!initialized) return;
      }
      
      // Begrenzen auf 0-256 (257 Stufen)
      if (value > 256) value = 256;
      
      // SPI Daten vorbereiten
      // Format: [Addr(4bit)][Cmd(2bit)][Data(10bit)]
      // Addr = 0x00 (Wiper 0), Cmd = 00 (Write)
      uint8_t tx_data[2];
      tx_data[0] = (MCP4162_WIPER0_ADDR << 4) | ((value >> 8) & 0x01);
      tx_data[1] = value & 0xFF;
      
      spi_transaction_t trans = {};
      trans.length = 16;  // 2 Bytes = 16 Bits
      trans.tx_buffer = tx_data;
      
      esp_err_t ret = spi_device_transmit(spi_handle, &trans);
      if (ret != ESP_OK) {
        ESP_LOGE("mcp4162", "SPI Transfer fehlgeschlagen: %d", ret);
        return;
      }
      
      ESP_LOGV("mcp4162", "Wiper = %d (0x%02X 0x%02X)", value, tx_data[0], tx_data[1]);
    }
};

// Globale Instanz (CS=GPIO5, CLK=GPIO18, MOSI=GPIO23)
static MCP4162 mcp4162(5, 18, 23);
