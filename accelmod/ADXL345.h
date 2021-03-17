#ifndef __ADXL345_H__
#define __ADXL345_H__

/* Bit values in BW_RATE                                                */
/* Expresed as output data rate */
#define XL345_RATE_3200       0x0f
#define XL345_RATE_1600       0x0e
#define XL345_RATE_800        0x0d
#define XL345_RATE_400        0x0c
#define XL345_RATE_200        0x0b
#define XL345_RATE_100        0x0a
#define XL345_RATE_50         0x09
#define XL345_RATE_25         0x08
#define XL345_RATE_12_5       0x07
#define XL345_RATE_6_25       0x06
#define XL345_RATE_3_125      0x05
#define XL345_RATE_1_563      0x04
#define XL345_RATE__782       0x03
#define XL345_RATE__39        0x02
#define XL345_RATE__195       0x01
#define XL345_RATE__098       0x00

/* Bit values in DATA_FORMAT                                            */

/* Register values read in DATAX0 through DATAZ1 are dependant on the 
   value specified in data format.  Customer code will need to interpret
   the data as desired.                                                 */
#define XL345_RANGE_2G             0x00
#define XL345_RANGE_4G             0x01
#define XL345_RANGE_8G             0x02
#define XL345_RANGE_16G            0x03
#define XL345_DATA_JUST_RIGHT      0x00
#define XL345_DATA_JUST_LEFT       0x04
#define XL345_10BIT                0x00
#define XL345_FULL_RESOLUTION      0x08
#define XL345_INT_LOW              0x20
#define XL345_INT_HIGH             0x00
#define XL345_SPI3WIRE             0x40
#define XL345_SPI4WIRE             0x00
#define XL345_SELFTEST             0x80

/* Bit values in INT_ENABLE, INT_MAP, and INT_SOURCE are identical
   use these bit values to read or write any of these registers.        */
#define XL345_OVERRUN              0x01
#define XL345_WATERMARK            0x02
#define XL345_FREEFALL             0x04
#define XL345_INACTIVITY           0x08
#define XL345_ACTIVITY             0x10
#define XL345_DOUBLETAP            0x20
#define XL345_SINGLETAP            0x40
#define XL345_DATAREADY            0x80

/* Bit values in POWER_CTL                                              */
#define XL345_WAKEUP_8HZ           0x00
#define XL345_WAKEUP_4HZ           0x01
#define XL345_WAKEUP_2HZ           0x02
#define XL345_WAKEUP_1HZ           0x03
#define XL345_SLEEP                0x04
#define XL345_MEASURE              0x08
#define XL345_STANDBY              0x00
#define XL345_AUTO_SLEEP           0x10
#define XL345_ACT_INACT_SERIAL     0x20
#define XL345_ACT_INACT_CONCURRENT 0x00

// ADXL345 Register List
#define ADXL345_REG_DEVID       	0x00
// BEGIN TAP Registers
#define ADXL345_REG_THRESH_TAP      0x1D
#define ADXL345_REG_DUR             0x21
#define ADXL345_REG_LATENT          0x22
#define ADXL345_REG_WINDOW          0x23
// END TAP registers
#define ADXL345_REG_POWER_CTL   	0x2D
#define ADXL345_REG_DATA_FORMAT 	0x31
#define ADXL345_REG_FIFO_CTL    	0x38
#define ADXL345_REG_BW_RATE     	0x2C
#define ADXL345_REG_INT_ENABLE  	0x2E  // default value: 0x00
#define ADXL345_REG_INT_MAP     	0x2F  // default value: 0x00
#define ADXL345_REG_INT_SOURCE  	0x30  // default value: 0x02
#define ADXL345_REG_DATAX0      	0x32  // read only
#define ADXL345_REG_DATAX1      	0x33  // read only
#define ADXL345_REG_DATAY0      	0x34  // read only
#define ADXL345_REG_DATAY1      	0x35  // read only
#define ADXL345_REG_DATAZ0      	0x36  // read only
#define ADXL345_REG_DATAZ1      	0x37  // read only
#define ADXL345_REG_OFSX        	0x1E
#define ADXL345_REG_OFSY        	0x1F
#define ADXL345_REG_OFSZ        	0x20
#define ADXL345_REG_THRESH_ACT		0x24
#define ADXL345_REG_THRESH_INACT	0x25
#define ADXL345_REG_TIME_INACT		0x26
#define ADXL345_REG_ACT_INACT_CTL	0x27
     
// Rounded division macro
#define ROUNDED_DIVISION(n, d) (((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d))


extern volatile unsigned int * SYSMGRVirt;
extern volatile unsigned int * I2C0Virt;


void Pinmux_Config(void) {
    // Set up pin muxing (in sysmgr) to connect ADXL345 wires to I2C0
    *(unsigned int *)(SYSMGRVirt + SYSMGR_I2C0USEFPGA) = 0;
    *(unsigned int *)(SYSMGRVirt + SYSMGR_GENERALIO7) = 1;
    *(unsigned int *)(SYSMGRVirt + SYSMGR_GENERALIO8) = 1;
}

// Initialize the I2C0 controller for use with the ADXL345 chip
void I2C0_Init(void){

    // Abort any ongoing transmits and disable I2C0.
    *(unsigned int *)(I2C0Virt + I2C0_ENABLE) = 2;
    
    // Wait until I2C0 is disabled
    while(((*(unsigned int *)(I2C0Virt + I2C0_ENABLE_STATUS))&0x1) == 1);
    
    // Configure the config reg with the desired setting (act as 
    // a master, use 7bit addressing, fast mode (400kb/s)).
    *(unsigned int *)(I2C0Virt + I2C0_CON) = 0x65;
    
    // Set target address (disable special commands, use 7bit addressing)
    *(unsigned int *)(I2C0Virt + I2C0_TAR) = 0x53;
    
    // Set SCL high/low counts (Assuming default 100MHZ clock input to I2C0 Controller).
    // The minimum SCL high period is 0.6us, and the minimum SCL low period is 1.3us,
    // However, the combined period must be 2.5us or greater, so add 0.3us to each.
    *(unsigned int *)(I2C0Virt + I2C0_FS_SCL_HCNT) = 60 + 30; // 0.6us + 0.3us
    *(unsigned int *)(I2C0Virt + I2C0_FS_SCL_LCNT) = 130 + 30; // 1.3us + 0.3us
    
    // Enable the controller
    *(unsigned int *)(I2C0Virt + I2C0_ENABLE) = 1;
    
    // Wait until controller is enabled
    while(((*(unsigned int *)(I2C0Virt + I2C0_ENABLE_STATUS))&0x1) == 0);
    
}


// Write value to internal register at address
void ADXL345_REG_WRITE(uint8_t address, uint8_t value){
    
    // Send reg address (+0x400 to send START signal)
    *(unsigned int *)(I2C0Virt+I2C0_DATA_CMD) = address + 0x400;
    
    // Send value
    *(unsigned int *)(I2C0Virt+I2C0_DATA_CMD) = value;
}

// Read value from internal register at address
void ADXL345_REG_READ(uint8_t address, uint8_t *value){

    // Send reg address (+0x400 to send START signal)
    *(unsigned int *)(I2C0Virt + I2C0_DATA_CMD) = address + 0x400;
    
    // Send read signal
    *(unsigned int *)(I2C0Virt + I2C0_DATA_CMD) = 0x100;
    
    // Read the response (first wait until RX buffer contains data)  
    while (*(unsigned int *)(I2C0Virt + I2C0_RXFLR) == 0){}
    *value = *(unsigned int *)(I2C0Virt+I2C0_DATA_CMD);
}

// Read multiple consecutive internal registers
void ADXL345_REG_MULTI_READ(uint8_t address, uint8_t values[], uint8_t len){
    int i;
    int nth_byte=0;

    // Send reg address (+0x400 to send START signal)
    *(unsigned int *)(I2C0Virt + I2C0_DATA_CMD) = address + 0x400;

    // Send read signal len times
    for (i=0;i<len;i++)
        *(unsigned int *)(I2C0Virt + I2C0_DATA_CMD) = 0x100;

    // Read the bytes
    while (len){
        if ((*(unsigned int *)(I2C0Virt +I2C0_RXFLR)) > 0){
            values[nth_byte] = *(unsigned int *)(I2C0Virt + I2C0_DATA_CMD);
            nth_byte++;
            len--;
        }
    }
}

// Return true if there was activity since the last read (checks ACTIVITY bit).
bool ADXL345_WasActivityUpdated(void){
    bool bReady = false;
    uint8_t data8;
    
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    if (data8 & XL345_ACTIVITY)
        bReady = true;
    
    return bReady;
}

// Return true if there is new data (checks DATA_READY bit).
bool ADXL345_IsDataReady(void){
    bool bReady = false;
    uint8_t data8;
    
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    if (data8 & XL345_DATAREADY)
        bReady = true;
    
    return bReady;
}

uint8_t ADXL345_WhichInterrupts(void) {
    uint8_t data8;
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    return data8;
} 

bool ADXL345_WasSingleTapped(void) {
    bool wasTapped = false;
    uint8_t data8;
    
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    if (data8 & XL345_SINGLETAP)
        wasTapped = true;

    return wasTapped;    

} 

bool ADXL345_WasDoubleTapped(void) {
    bool wasTapped = false;
    uint8_t data8;
    
    ADXL345_REG_READ(ADXL345_REG_INT_SOURCE,&data8);
    if (data8 & XL345_DOUBLETAP)
        wasTapped = true;

    return wasTapped;        
}

// Read acceleration data of all three axes
void ADXL345_XYZ_Read(int16_t szData16[3]){
    uint8_t szData8[6];
    ADXL345_REG_MULTI_READ(0x32, (uint8_t *)&szData8, sizeof(szData8));

    szData16[0] = (szData8[1] << 8) | szData8[0]; 
    szData16[1] = (szData8[3] << 8) | szData8[2];
    szData16[2] = (szData8[5] << 8) | szData8[4];
}

// Read the ID register
void ADXL345_IdRead(uint8_t *pId){
    ADXL345_REG_READ(ADXL345_REG_DEVID, pId);
}


void ADXL345_SetFreq(uint16_t Freq) {
    switch(Freq) {
        case 3200:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_3200);
            break;
        case 1600:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_1600);
            break;
        case 800:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_800);
            break;
        case 400:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_400);
            break;
        case 200:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_200);
            break;
        case 100:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_100);
            break;
        case 50:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_50);
            break;
        case 25:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_25);
            break;
        case 12:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_12_5);
            break;
        case 6:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_6_25);
            break;
        case 3:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_3_125);
            break;
        case 1:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_1_563);
            break;
        default:
            ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_12_5);
    }
}

void ADXL345_SetG(bool FullRes, uint16_t G, uint16_t * Scale) {
    uint8_t GSet;
    switch(G) {
        case 2:
            GSet = XL345_RANGE_2G;
            *Scale = ROUNDED_DIVISION(2*1000, 512);
            break;
        case 4:
            GSet = XL345_RANGE_4G;
            *Scale = ROUNDED_DIVISION(4*1000, 512);
            break;
        case 8:
            GSet = XL345_RANGE_8G;
            *Scale = ROUNDED_DIVISION(8*1000, 512);
            break;
        case 16:
            GSet = XL345_RANGE_16G;
            *Scale = ROUNDED_DIVISION(16*1000, 512);
            break;
        default:
            GSet = XL345_RANGE_16G;
            *Scale = ROUNDED_DIVISION(16*1000, 512);
    }
    if(FullRes) {
        GSet |= XL345_FULL_RESOLUTION; 
        *Scale = 4;
    }
    ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, GSet);
}

// Initialize the ADXL345 chip
void ADXL345_Init(void){

    // Stop Measuring.
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);

    // 16g range, Fixed resolution (10-bits).
    ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, XL345_RANGE_16G);
    
    // Output Data Rate: 12.5Hz
    ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_12_5);

    // NOTE: The DATA_READY bit is not reliable. It is updated at a much higher rate than the Data Rate
    // Use the Activity and Inactivity interrupts.
    //----- Enabling interrupts -----//
    ADXL345_REG_WRITE(ADXL345_REG_THRESH_ACT, 0x04);    //activity threshold
    ADXL345_REG_WRITE(ADXL345_REG_THRESH_INACT, 0x02);  //inactivity threshold
    ADXL345_REG_WRITE(ADXL345_REG_TIME_INACT, 0x02);    //time for inactivity
    ADXL345_REG_WRITE(ADXL345_REG_ACT_INACT_CTL, 0xFF); //Enables AC coupling for thresholds

    // Tap Threshold = 3G
    // 3000mg/62.5mg/LSB == 48 (base 10) 
    ADXL345_REG_WRITE(ADXL345_REG_THRESH_TAP, 48);

    // Tap Duration = 0.02s
    // 20000 us / 625 us/LSB == 32 (base 10)
    ADXL345_REG_WRITE(ADXL345_REG_DUR, 32);

    // Tap Latency = 0.02s
    // 20 ms / 1.25ms/LSB == 16 (base 10)
    ADXL345_REG_WRITE(ADXL345_REG_LATENT, 16);

    // Double Tap Window = 0.3s
    // 300/ 1.25ms/LSB == 240 (base 10)
    ADXL345_REG_WRITE(ADXL345_REG_WINDOW, 240);


    ADXL345_REG_WRITE(ADXL345_REG_INT_ENABLE, XL345_SINGLETAP | XL345_SINGLETAP | XL345_ACTIVITY | XL345_INACTIVITY );  //enable interrupts
    //-------------------------------//


    // start measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
}

// Calibrate the ADXL345. The DE1-SoC should be placed on a flat
// surface, and must remain stationary for the duration of the calibration.
void ADXL345_Calibrate(void){
    
    int average_x = 0;
    int average_y = 0;
    int average_z = 0;
    int16_t XYZ[3];
    int8_t offset_x;
    int8_t offset_y;
    int8_t offset_z;

    int i = 0;
    uint8_t saved_bw;
    uint8_t saved_dataformat;
    
    // stop measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
    
    // Get current offsets
    ADXL345_REG_READ(ADXL345_REG_OFSX, (uint8_t *)&offset_x);
    ADXL345_REG_READ(ADXL345_REG_OFSY, (uint8_t *)&offset_y);
    ADXL345_REG_READ(ADXL345_REG_OFSZ, (uint8_t *)&offset_z);
    
    // Use 100 hz rate for calibration. Save the current rate.
    ADXL345_REG_READ(ADXL345_REG_BW_RATE, &saved_bw);
    ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_100);
    
    // Use 16g range, full resolution. Save the current format.
    ADXL345_REG_READ(ADXL345_REG_DATA_FORMAT, &saved_dataformat);
    ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, XL345_RANGE_16G | XL345_FULL_RESOLUTION);
    
    // start measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
    
    // Get the average x,y,z accelerations over 32 samples (LSB 3.9 mg)
    while (i < 32){
        // Note: use DATA_READY here, can't use ACTIVITY because board is stationary.
        if (ADXL345_IsDataReady()){
            ADXL345_XYZ_Read(XYZ);
            average_x += XYZ[0];
            average_y += XYZ[1];
            average_z += XYZ[2];
            i++;
        }
    }
    average_x = ROUNDED_DIVISION(average_x, 32);
    average_y = ROUNDED_DIVISION(average_y, 32);
    average_z = ROUNDED_DIVISION(average_z, 32);
    
    // stop measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
    
    // printf("Average X=%d, Y=%d, Z=%d\n", average_x, average_y, average_z);
    
    // Calculate the offsets (LSB 15.6 mg)
    offset_x += ROUNDED_DIVISION(0-average_x, 4);
    offset_y += ROUNDED_DIVISION(0-average_y, 4);
    offset_z += ROUNDED_DIVISION(256-average_z, 4);
    
    // printf("Calibration: offset_x: %d, offset_y: %d, offset_z: %d (LSB: 15.6 mg)\n",offset_x,offset_y,offset_z);
    
    // Set the offset registers
    ADXL345_REG_WRITE(ADXL345_REG_OFSX, offset_x);
    ADXL345_REG_WRITE(ADXL345_REG_OFSY, offset_y);
    ADXL345_REG_WRITE(ADXL345_REG_OFSZ, offset_z);
    
    // Restore original bw rate
    ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, saved_bw);
    
    // Restore original data format
    ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, saved_dataformat);
    
    // start measure
    ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
}



#endif /*ACCELEROMETER_ADXL345_SPI_H_*/
