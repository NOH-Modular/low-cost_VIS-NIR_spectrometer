import machine
import time
from machine import I2C, Pin

#AS7341 address
AS7341_ADDR = 0x39

#AS7341 registers
AS7341_ENABLE = 0x80
AS7341_STATUS = 0x93
AS7341_CONFIG = 0x70
AS7341_CONTROL = 0x71
AS7341_ATIME = 0x81
AS7341_ASTEP = 0x82
AS7341_AGAIN = 0xAA
AS7341_CFG0 = 0xA9
AS7341_I2C_CFG = 0x70

#Control register bits
AS7341_EN_PON = 0x01
AS7341_EN_SP_EN = 0x02
AS7341_EN_SMUX_EN = 0x10
AS7341_EN_FLICKER_EN = 0x08

#SMUX registers
AS7341_SMUX_CMD = 0x7F
AS7341_SMUX31 = 0x00
AS7341_SMUX_STATUS = 0x73
AS7341_SMUX_STATUS_SM_VALID = 0x01

#Data registers
AS7341_DATA0 = 0x94  #F1 data
AS7341_DATA1 = 0x96  #F2 data
AS7341_DATA2 = 0x98  #F3 data
AS7341_DATA3 = 0x9A  #F4 data
AS7341_DATA4 = 0x9C  #F5 data
AS7341_DATA5 = 0x9E  #F6 data
AS7341_DATA6 = 0xA0  #F7 data
AS7341_DATA7 = 0xA2  #F8 data
AS7341_DATA8 = 0xA4  #Clear data
AS7341_DATA9 = 0xA6  #NIR data

#SMUX configuration constants
SMUX_OUT_F1 = 0x30
SMUX_OUT_F2 = 0x01
SMUX_OUT_F3 = 0x32
SMUX_OUT_F4 = 0x03
SMUX_OUT_F5 = 0x34
SMUX_OUT_F6 = 0x05
SMUX_OUT_F7 = 0x36
SMUX_OUT_F8 = 0x07
SMUX_OUT_CLEAR = 0x08
SMUX_OUT_NIR = 0x09

#Channel wavelengths in nm
WAVELENGTHS = {
    "F1": 415,  #Violet
    "F2": 445,  #Indigo
    "F3": 480,  #Blue
    "F4": 515,  #Cyan
    "F5": 555,  #Green
    "F6": 590,  #Yellow
    "F7": 630,  #Orange
    "F8": 680,  #Red
    "Clear": 0,  #Clear
    "NIR": 910,  #Near-infrared
}

class AS7341:
    def __init__(self, i2c, addr=AS7341_ADDR):
        self.i2c = i2c
        self.addr = addr
        
        #Check if device is present
        devices = i2c.scan()
        if addr not in devices:
            raise RuntimeError(f"AS7341 not found at address 0x{addr:02x}")
        
        self.debug_enabled = True
        self.reset()
        self.setup()
    
    def debug(self, msg):
        """Print debug message if debug is enabled"""
        if self.debug_enabled:
            print(f"DEBUG: {msg}")
    
    def write_reg(self, reg, value):
        """Write byte to register"""
        try:
            self.i2c.writeto_mem(self.addr, reg, bytes([value]))
            if self.debug_enabled:
                self.debug(f"Write reg 0x{reg:02x} = 0x{value:02x}")
        except Exception as e:
            print(f"Error writing to reg 0x{reg:02x}: {e}")
            raise
    
    def read_reg(self, reg):
        """Read byte from register"""
        try:
            result = self.i2c.readfrom_mem(self.addr, reg, 1)[0]
            if self.debug_enabled:
                self.debug(f"Read reg 0x{reg:02x} = 0x{result:02x}")
            return result
        except Exception as e:
            print(f"Error reading from reg 0x{reg:02x}: {e}")
            raise
    
    def reset(self):
        """Reset the device"""
        try:
            #Soft reset
            self.debug("Performing soft reset")
            self.write_reg(AS7341_ENABLE, 0x00)  #Disable everything
            time.sleep_ms(10)
            self.write_reg(AS7341_ENABLE, 0x01)  #Power on
            time.sleep_ms(10)
        except Exception as e:
            print(f"Reset failed: {e}")
    
    def setup(self):
        """initialise the AS7341 sensor"""
        try:
            self.debug("Setting up AS7341")
            
            #Enable PON (power on) in ENABLE register
            self.write_reg(AS7341_ENABLE, AS7341_EN_PON)
            time.sleep_ms(10)
            
            #Set ATIME (integration time) - higher value = longer integration time
            self.write_reg(AS7341_ATIME, 0x10)  #~50ms
            
            #Set ASTEP (integration time step size)
            self.write_reg(AS7341_ASTEP, 0xFF)  #Max step size
            
            #Set gain to 64x (0x07)
            self.set_gain(3)  #Using a higher gain for better sensitivity
            
            #Configure I2C and init control registers
            self.write_reg(AS7341_CFG0, 0x00)  #Set default configuration
            
            self.debug("Setup complete")
        except Exception as e:
            print(f"Setup failed: {e}")
            raise
    
    def set_gain(self, gain):
        """Set the analog gain (0-10)"""
        #Gain values: 0=0.5x, 1=1x, 2=2x, 3=4x, 4=8x, 5=16x, 6=32x, 7=64x, 8=128x, 9=256x, 10=512x
        if gain < 0 or gain > 10:
            gain = 5  #Default to 16x gain
        
        #Get actual gain value for display
        real_gain = 0.5 * (2 ** gain) if gain > 0 else 0.5
        self.debug(f"Setting gain to {real_gain}x (register value: {gain})")
        
        #Write to the AGAIN register (correct address)
        self.write_reg(AS7341_AGAIN, gain)
        
    def write_smux_config(self):
        """Write SMUX configuration for standard F1-F8, Clear, NIR readings"""
        try:
            #Set SMUX command register to write mode, PON=1
            self.write_reg(AS7341_ENABLE, AS7341_EN_PON)
            
            #Enable Config state
            self.write_reg(AS7341_CONFIG, 0x01)
            
            #Send SMUX command for RAM access
            self.write_reg(AS7341_SMUX_CMD, 0x10)
            
            #Write SMUX configuration
            ram_loc = 0  #Start at register 0x00
            
            #F1 to ADC0
            self.write_reg(ram_loc, SMUX_OUT_F1)
            ram_loc += 1
            
            #F2 to ADC1
            self.write_reg(ram_loc, SMUX_OUT_F2)
            ram_loc += 1
            
            #F3 to ADC2
            self.write_reg(ram_loc, SMUX_OUT_F3)
            ram_loc += 1
            
            #F4 to ADC3
            self.write_reg(ram_loc, SMUX_OUT_F4)
            ram_loc += 1
            
            #F5 to ADC4
            self.write_reg(ram_loc, SMUX_OUT_F5)
            ram_loc += 1
            
            #F6 to ADC5
            self.write_reg(ram_loc, SMUX_OUT_F6)
            ram_loc += 1
            
            #F7 to ADC6
            self.write_reg(ram_loc, SMUX_OUT_F7)
            ram_loc += 1
            
            #F8 to ADC7
            self.write_reg(ram_loc, SMUX_OUT_F8)
            ram_loc += 1
            
            #Clear to ADC8
            self.write_reg(ram_loc, SMUX_OUT_CLEAR)
            ram_loc += 1
            
            #NIR to ADC9
            self.write_reg(ram_loc, SMUX_OUT_NIR)
            
            #Enable SMUX (writing 0x10 tells the chip to apply the config)
            self.write_reg(AS7341_SMUX_CMD, 0x10)
            time.sleep_ms(50)  #Wait for SMUX to be applied
            
            #Check SMUX status
            status = self.read_reg(AS7341_SMUX_STATUS)
            if status & AS7341_SMUX_STATUS_SM_VALID:
                self.debug("SMUX configuration valid")
            else:
                self.debug("SMUX configuration NOT valid!")
                
            #Set CONFIG back to 0
            self.write_reg(AS7341_CONFIG, 0x00)
            
        except Exception as e:
            print(f"SMUX configuration failed: {e}")
            raise
    
    def read_channels(self):
        """Read all spectral channels and return values"""
        try:
            self.debug("Reading channels...")
            
            #Configure SMUX
            self.write_smux_config()
            
            #Enable spectral measurement
            self.write_reg(AS7341_ENABLE, AS7341_EN_PON | AS7341_EN_SP_EN)
            time.sleep_ms(10)  #Give it a moment to start
            
            #Wait for data to be ready (with a longer timeout)
            timeout = 200
            status = 0
            while timeout > 0:
                status = self.read_reg(AS7341_STATUS)
                self.debug(f"Status: 0x{status:02x}")
                if status & 0x08:  #Check if data is ready (AVALID bit)
                    self.debug("Data is ready!")
                    break
                time.sleep_ms(10)
                timeout -= 1
            
            if timeout <= 0:
                self.debug(f"Timeout waiting for data! Final status: 0x{status:02x}")
                return {k: 0 for k in WAVELENGTHS.keys()}
            
            #Read all data registers
            channels = {}
            
            #Read F1-F8 channels
            for i, name in enumerate(["F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"]):
                #Each channel data is in two registers (low and high bytes)
                reg_low = AS7341_DATA0 + (i * 2)
                reg_high = reg_low + 1
                
                low = self.read_reg(reg_low)
                high = self.read_reg(reg_high)
                
                value = (high << 8) | low
                channels[name] = value
                self.debug(f"{name}: {value}")
            
            #Read Clear and NIR
            clear_low = self.read_reg(AS7341_DATA8)
            clear_high = self.read_reg(AS7341_DATA8 + 1)
            channels["Clear"] = (clear_high << 8) | clear_low
            self.debug(f"Clear: {channels['Clear']}")
            
            nir_low = self.read_reg(AS7341_DATA9)
            nir_high = self.read_reg(AS7341_DATA9 + 1)
            channels["NIR"] = (nir_high << 8) | nir_low
            self.debug(f"NIR: {channels['NIR']}")
            
            #Disable spectral engine to save power
            self.write_reg(AS7341_ENABLE, AS7341_EN_PON)
            
            return channels
        
        except Exception as e:
            print(f"Error reading channels: {e}")
            return {k: 0 for k in WAVELENGTHS.keys()}
    
    def identify_colour(self, channels):
        """Identify colour based on channel readings"""
        #Check if we have valid readings
        if sum(channels.values()) == 0:
            return "No light detected"
        
        #normalise readings
        total = sum(v for k, v in channels.items() if k not in ["Clear", "NIR"])
        if total == 0:
            return "No visible light detected"
            
        normalised = {k: v/total for k, v in channels.items() if k not in ["Clear", "NIR"]}
        
        #Find dominant channel
        dominant = max(normalised.items(), key=lambda x: x[1])
        
        #Colour dentification based on dominant wavelength
        wavelength = WAVELENGTHS[dominant[0]]
        if wavelength < 450:
            return "Violet"
        elif wavelength < 495:
            return "Blue"
        elif wavelength < 570:
            return "Green"
        elif wavelength < 590:
            return "Yellow"
        elif wavelength < 620:
            return "Orange"
        else:
            return "Red"
        
    def run_diagnostics(self):
        """Run diagnostics to check sensor functionality"""
        print("\n===== AS7341 DIAGNOSTICS =====")
        
        #1. Check power-on status
        enable = self.read_reg(AS7341_ENABLE)
        print(f"ENABLE register: 0x{enable:02x} (PON: {'ON' if enable & 0x01 else 'OFF'})")
        
        #2. Check device ID register (if available)
        try:
            device_id = self.read_reg(0x92)  #ID register
            print(f"Device ID: 0x{device_id:02x}")
        except:
            print("Device ID not available")
        
        #3. Read STATUS
        status = self.read_reg(AS7341_STATUS)
        print(f"STATUS register: 0x{status:02x}")
        
        #4. Check gain setting
        gain = self.read_reg(AS7341_AGAIN)
        print(f"AGAIN register: 0x{gain:02x}")
        
        #5. Read and report ATIME (integration time)
        atime = self.read_reg(AS7341_ATIME)
        print(f"ATIME register: 0x{atime:02x}")
        
        #6. Test SMUX access
        print("Testing SMUX access...")
        self.write_reg(AS7341_CONFIG, 0x01)  #Enable config mode
        self.write_reg(AS7341_SMUX_CMD, 0x10)  #Set to NORMAL mode
        smux_status = self.read_reg(AS7341_SMUX_STATUS)
        print(f"SMUX_STATUS register: 0x{smux_status:02x} (Valid: {'Yes' if smux_status & 0x01 else 'No'})")
        self.write_reg(AS7341_CONFIG, 0x00)  #Disable config mode
        
        #7. Test reading from data registers
        print("Reading sample data registers:")
        data0_low = self.read_reg(AS7341_DATA0)
        data0_high = self.read_reg(AS7341_DATA0 + 1)
        print(f"DATA0 registers: 0x{data0_low:02x}, 0x{data0_high:02x} (Value: {(data0_high << 8) | data0_low})")
        
        #8. Try a simple test measurement
        self.write_smux_config()
        self.write_reg(AS7341_ENABLE, AS7341_EN_PON | AS7341_EN_SP_EN)
        time.sleep_ms(100)  #Wait for measurement
        status = self.read_reg(AS7341_STATUS)
        print(f"After measurement, STATUS: 0x{status:02x} (AVALID: {'Yes' if status & 0x08 else 'No'})")
        
        #9. Verify I2C communication speed (reading multiple registers quickly)
        print("Verifying I2C performance...")
        start = time.ticks_ms()
        for i in range(10):
            self.read_reg(AS7341_ENABLE)
        elapsed = time.ticks_ms() - start
        print(f"10 register reads took {elapsed}ms ({elapsed/10}ms per read)")
        
        print("===== DIAGNOSTICS COMPLETE =====\n")

def print_readings(channels):
    """Print colour spectrum readings in a nicely formatted way"""
    if sum(channels.values()) == 0:
        print("WARNING: All channels reading zero - check sensor connection/configuration")
        return
    
    print("\n" + "=" * 50)
    print("Spectral Readings".center(50))
    print("=" * 50)
    print(f"{'Channel':<10}{'Wavelength':<15}{'Reading':<15}{'%':<10}")
    print("-" * 50)
    
    #Calculate percentages for visualisation
    total = sum(channels.values())
    
    #Show F1-F8 in wavelength order
    for channel in ["F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"]:
        value = channels[channel]
        percentage = (value / total * 100) if total > 0 else 0
        wavelength = f"{WAVELENGTHS[channel]}nm"
        
        #Bar Chart
        bar_length = int(percentage / 2)  #Scale to fit in console
        bar = "█" * bar_length
        
        print(f"{channel:<10}{wavelength:<15}{value:<15}{percentage:6.2f}% {bar}")
    
    #Show Clear and NIR
    print("-" * 50)
    print(f"{'Clear':<10}{'N/A':<15}{channels['Clear']:<15}{((channels['Clear']/total*100) if total > 0 else 0):6.2f}%")
    print(f"{'NIR':<10}{WAVELENGTHS['NIR']}nm{'':<5}{channels['NIR']:<15}{((channels['NIR']/total*100) if total > 0 else 0):6.2f}%")
    
    print("=" * 50)

def main():
    #Set up LED for status indication
    led = Pin(25, Pin.OUT)
    led.value(1)  #Turn on LED to show program is running
    
    print("initialising AS7341 Spectral Sensor Test")
    print("----------------------------------------")
    
    #initialise I2C with the correct pins
    try:
        #Use pins 1 (SDA) and 2 (SCL)
        print("initialising I2C with SDA=Pin(1), SCL=Pin(2)...")
        i2c = I2C(0, sda=Pin(0), scl=Pin(1), freq=100000) 
        
        #Scan for devices
        devices = i2c.scan()
        print(f"I2C devices found: {[hex(d) for d in devices]}")
        
        if AS7341_ADDR not in devices:
            print(f"AS7341 (0x{AS7341_ADDR:02x}) not found on the I2C bus!")
            print("Please check your wiring and try again.")
            return
        
        #initialise the sensor
        sensor = AS7341(i2c)
        print("AS7341 sensor initialised successfully!")
        
        #Run initial diagnostics
        sensor.run_diagnostics()
        
    except Exception as e:
        print(f"initialisation error: {e}")
        led.value(0)
        return
    
    #Main menu loop
    while True:
        print("\nAS7341 SPECTRAL SENSOR TEST")
        print("---------------------------")
        print("1. Test Sensor (Single Reading)")
        print("2. Continuous Readings")
        print("3. Run Diagnostics")
        print("4. Set Gain")
        print("q. Quit")
        
        choice = input("\nSelect option: ").strip().lower()
        
        if choice == 'q':
            break
            
        elif choice == '1':
            print("\nTaking spectral measurement...")
            led.value(1)
            
            #Take measurement
            start = time.ticks_ms()
            channels = sensor.read_channels()
            elapsed = time.ticks_ms() - start
            
            #Print results
            print_readings(channels)
            
            #Identify colour
            colour = sensor.identify_colour(channels)
            print(f"Detected colour: {colour}")
            print(f"Measurement time: {elapsed}ms")
            
            led.value(0)
            
        elif choice == '2':
            print("\nContinuous reading mode (Press Ctrl+C to stop)")
            count = 0
            
            try:
                while True:
                    #Blink LED
                    led.value(count % 2)
                    
                    #Take measurement
                    channels = sensor.read_channels()
                    
                    #Simple output
                    colour = sensor.identify_colour(channels)
                    dominant = max([(k, v) for k, v in channels.items() if k not in ["Clear", "NIR"]], 
                                  key=lambda x: x[1], default=("None", 0))
                    
                    print(f"\rReading #{count}: {colour:<10} - Dominant: {dominant[0]} ({WAVELENGTHS[dominant[0]]}nm): {dominant[1]}", end="")
                    
                    count += 1
                    time.sleep_ms(500)
                    
            except KeyboardInterrupt:
                print("\nContinuous mode stopped")
                
            led.value(0)
            
        elif choice == '3':
            led.value(1)
            sensor.run_diagnostics()
            led.value(0)
            
        elif choice == '4':
            gain = input("Enter gain (0-10, 0=0.5x, 5=16x, 10=512x): ")
            try:
                gain_val = int(gain)
                if 0 <= gain_val <= 10:
                    sensor.set_gain(gain_val)
                    print(f"Gain set to {0.5 * (2 ** gain_val) if gain_val > 0 else 0.5}x")
                else:
                    print("Invalid gain value. Please enter a number between 0 and 10.")
            except ValueError:
                print("Invalid input. Please enter a number.")
        
        else:
            print("Invalid choice, please try again.")
    
    #Clean up
    led.value(0)
    print("Exiting AS7341 test program.")

if __name__ == "__main__":
    main()