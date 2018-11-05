#include "testmethod.hpp"

//for test method API interfaces
#include "mapi.hpp"
#include "rdi.hpp"
#include <cmath>

#include "../UtilityMaps/InitUtilitySateRDI.h"
#include "../UtilityMaps/MyUtilReadAndPrintSetUtils.h"
#include "../lib/Info.h"

#include "../COMM/communication.h"
#include "../OptimalSettings/OptimalSettings_class.h"

#include "../RegCheck/I2C_REGCHECK.cpp"

using namespace std;
using namespace V93kLimits;

/*
 * 0: use Measured Data
 * 1: use Data from array
 */
#define Is_MeasuredData	0

/**
 * Test method class.
 *
 * For each test suite using this test method, one object of this
 * class is created.
 */
class Bandgap: public testmethod::TestMethod {

protected:
  /**
   *Initialize the parameter interface to the testflow.
   *This method is called just once after a testsuite is created.
   */

  virtual void initialize()
  {
    //Add your initialization code here
    //Note: Test Method API should not be used in this method!
  }

  /**
   *This test is invoked per site.
   */
  virtual void run()
  {
    RDI_INIT();
	REGISTER_GLOBAL_INIT();

	/*
	 * TRIGGER example
	 */
	// rdi.port("Port_AllwoBCK").smartVec().pin("TRIGGER").fillVec("1",1,0).defaultBit(0).vecLength(2).execute();

	/*
	 * Measure 17 times on TEST2
	 */
	ON_FIRST_INVOCATION_BEGIN();

		OptimalSettings_class TFA9894_OptimalSettings;

		rdi.hiddenUpload(TA::ALL);
		RDI_BEGIN(TA::BURST);
			// High speed mode I2C
			registers.i2cMap.hs_mode = 0x1; // I2C high speed mode control; 0b = I2C filter enable [default]; 1b = I2C filter bypassed for HS mode

			// Optimal settings
			registers.i2cMap.calibr_temp_gain = 3; // <<== collabnet artf237847: Compensate temperature sensor gain error, and mail Roland 20170322: TFA9894/74 callibration of temperature
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
		  	TFA9894_OptimalSettings.OptimalSettings();
			// End Optimal settings


			// Define initial setting before power-up (start with Outputs disabled, FollowerMode DCDC, Bypass protections, Normal end-stage, only BCK clock available all others stable, Low power mode is off)
			registers.i2cMap.bypass_ocp = 1;
			registers.i2cMap.bypass_ovp = 1;
			registers.i2cMap.bypass_uvp = 1;
			registers.i2cMap.bypass_otp = 1;
			registers.i2cMap.bypass_lost_clk = 1; // Bypass lost clock detector; 0b = lost clock detector active [default]; 1b = lost clock detector bypassed
			registers.i2cMap.disable_low_power_mode = 1; // Low power mode1 detector control; 0b = enabled [default]; 1b = disabled
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);

			// VS is connected to TEST3 and TEST4 (Parallel to amux. This path must be disabled.
			// Is done using enbl_vs_inp1 = 0; and enbl_vs_inn1 = 0;
 			registers.i2cMap.enbl_vs_inp1 = 0; // Enable connection of voltage sense positive1; 0b = disable; 1b = enable [default]
			registers.i2cMap.enbl_vs_inp2 = 0; // Enable connection of voltage sense positive2; 0b = disable [default]; 1b = enable
			registers.i2cMap.enbl_vs_inn1 = 0; // Enable connection of voltage sense negative1; 0b = disable; 1b = enable [default]
			registers.i2cMap.enbl_vs_inn2 = 0; // Enable connection of voltage sense negative2; 0b = disable [default]; 1b = enable
			registers.i2cMap.vs_inp_short = 1; // Short voltage sense positive to common mode; 0b = not connected [default]; 1b = connected
			registers.i2cMap.vs_inn_short = 1; // Short voltage sense negative to common mode; 0b = not connected [default]; 1b = connected
			registers.i2cMap.enbl_vs_adc = 0;  // Enable voltage sense ADC; 0b = disable [default]; 1b = enable
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);

			// Do not use low power mode
			registers.i2cMap.disable_low_power_mode = 1; // Low power mode1 detector control; 0b = enabled [default]; 1b = disabled
			registers.i2cMap.lpm1_mode = 1; // Low power mode control; 00b = auto [default]; 01b = low power mode disabled; 10b = low power mode disabled; 11b = low power mode forced active
			registers.i2cMap.lownoisegain_mode = 2; // Low noise gain mode control; 00b = auto [default]; 01b = fixed to high gain; 10b = fixed to high gain; 11b = fixed to low gain
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);

			// make sure the endstage is not switching
			registers.i2cMap.enbl_amplifier = 0; // Enable Amplifier; 0b = tristate [default]; 1b = switching/active
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);

			// Wait4SrcSettings to Operating
			registers.i2cMap.src_set_configured = 1; // Device I2C settings configured; 0b = HW I2C not loaded [default]; 1b = HW I2C loaded
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").wait(2.2 ms);

			/*
			 * In Operating state
			 */

			// Read manager state (to check if operating state is reached)
			// If expected value is found then the expect generates a "1".
			registers.i2cMap.man_state.markForExpect ( 0x9 );
			registers.expect ( CM::I2C, "man_state0", CM::BURST );

		RDI_END();

		/*
		 * Prepare source to discharge DC lines of the TEST2 pin
		 * Needed to prevent spikes when switching on the BADC of TEST2
		 */
		TASK_LIST Discharge_DC_TEST2;
		// Close DC of GNDP pin
		// Close Isol relay of GNDP pin (and with that also for TEST2)
		FLEX_RELAY Discharge_ISOL_TEST2_GNDP; Discharge_ISOL_TEST2_GNDP.pin("GNDP").set("DC","1XXXXX"); // DC relay with ISOL
		// Open Isol relay of GNDP and TEST2 group
		FLEX_RELAY Disconnect_ISOL_TEST2; Disconnect_ISOL_TEST2.pin("GNDP").set("IDLE","0XXXXX"); // DC relay with ISOL
		// Open DC relay GNDP pin.
		Discharge_DC_TEST2.add(Discharge_ISOL_TEST2_GNDP).add(Disconnect_ISOL_TEST2).execute();

		rdi.hiddenUpload(TA::ALL);
		RDI_BEGIN(TA::BURST);
		/*
		  	 * Prepare source to force level TEST2 pin = 0V
		  	 * -1- PPMU TEST2  = 0.0V
		  	 * -2- Connect PPMU, DC and AMUX_BADC to get 0V on TEST2 channel (on interface)
		  	 * -3- Connect channel to DUT pin TEST2
		  	 * -4- Disconnect the PPMU of TEST2 leaving the DC and AMUX_BADC active
		  	 * -5- Activate the amux of the device.
		  	 */
			rdi.port("Port_AllwoBCK").dc().pin("TEST2" ).vForce(0.0 V).iRange(100 uA).clamp(100 uA, 100 uA).execute(); // PPMU
			rdi.port("Port_AllwoBCK").hwRelay().pin("TEST2").setOff("AC").setOn("DC,PPMU,AMUX_BADC").execute();
			// Initialize the utility driver inside RDI_BEGIN (prevent test time implications)
			InitUtilitySate_RDI Bandgap_UtilityMap = InitUtilitySate_RDI();
			Bandgap_UtilityMap.utilMap.setPort("Port_AllwoBCK");
			// Connect TEST2 to channel
			Bandgap_UtilityMap.utilMap.ChangeUtilityState("TEST1_2_Ch", "on");

			Bandgap_UtilityMap.utilMap.ExecuteUtilSetOn(); // The combination above creates a sort of set for the supplyPins
			rdi.port("Port_AllwoBCK").wait(1000 us); // Ton time using IF = 5mA gives 1ms max (ASSR1510)
			// Open TEST2 (AC relay) and connect the BADC
			rdi.port("Port_AllwoBCK").hwRelay().pin("TEST2").setOff("AC,PPMU").setOn("DC,AMUX_BADC").execute();
			/*
			 * End discharge block
			 */

			/*
			 * Generic: The average of 64 is used to have a CP > 12.3 on the delta test.
			 */

			// Set the appropriate I2C settings (gnda_sense)
			registers.i2cMap.enbl_anamux2 = 1;
			registers.i2cMap.anamux2 = 0;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("BandgapMeasured").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x8)
			registers.i2cMap.anamux2 = 12; // vbg
			registers.i2cMap.calibr_vbg_trim = 0x8;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x9)
			registers.i2cMap.calibr_vbg_trim = 0x9;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xA)
			registers.i2cMap.calibr_vbg_trim = 0xA;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xB)
			registers.i2cMap.calibr_vbg_trim = 0xB;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xC)
			registers.i2cMap.calibr_vbg_trim = 0xC;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xD)
			registers.i2cMap.calibr_vbg_trim = 0xD;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xE)
			registers.i2cMap.calibr_vbg_trim = 0xE;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0xF)
			registers.i2cMap.calibr_vbg_trim = 0xF;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x0)
			registers.i2cMap.calibr_vbg_trim = 0x0;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x1)
			registers.i2cMap.calibr_vbg_trim = 0x1;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x2)
			registers.i2cMap.calibr_vbg_trim = 0x2;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x3)
			registers.i2cMap.calibr_vbg_trim = 0x3;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x4)
			registers.i2cMap.calibr_vbg_trim = 0x4;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x5)
			registers.i2cMap.calibr_vbg_trim = 0x5;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x6)
			registers.i2cMap.calibr_vbg_trim = 0x6;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Set path to measure (V_bandgap_0x7)
			registers.i2cMap.calibr_vbg_trim = 0x7;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").dc("").pin("TEST2",TA::BADC).vMeas().measWait(1.0 ms).limit(-2000 mV, 5750 mV).amux(false).relay(TA::badcRly_NO).average(64).execute(); // pure BADC, VMeas only.

			// Disconnect TEST2 from channel
			Bandgap_UtilityMap.utilMap.ChangeUtilityState("TEST1_2_Ch", "off");
			Bandgap_UtilityMap.utilMap.ExecuteUtilSetOn();

			// TEST2 back to default = AC relay closed
			rdi.port("Port_AllwoBCK").hwRelay().pin("TEST2").setOff("PPMU,DC").setOn("AC").execute();

			// Reset anamux
			registers.i2cMap.enbl_anamux2 = 0x0;
			registers.i2cMap.anamux2 = 0x0;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);

			// Operating to PowerDown
			registers.i2cMap.ctrl_on2off_criterion = 0x000; // Amplifier on-off criteria for shutdown; others = minimum number of consecutive zeroes as output by noise shaper before amplifier goes into tristate to shut down
			registers.i2cMap.sel_enbl_amplifier = 0x0; // CoolFlux control over amplifier; 0b = no control; 1b = control [default]
			registers.i2cMap.enbl_amplifier = 0x0; // Enable Amplifier; 0b = tristate [default]; 1b = switching/active
			registers.i2cMap.src_set_configured = 0;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			registers.i2cMap.powerdown = 0x1; // Powerdown control; 0b = Operating; 1b = Powerdown [default]
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);
			rdi.port("Port_AllwoBCK").wait(1 ms);

			/*
			 * In Powerdown
			 */

			registers.i2cMap.hs_mode = 0x0;
			registers.write(CM::I2C, CM::STATIC_WRITE, CM::BURST);


			RDI_END();

//		I2C_REGCHECK MyCheck;
//		MyCheck.registerCheck();

		ARRAY_I activeSite;
		GET_ACTIVE_SITES(activeSite);
		SMC_ARM(activeSite);

		// Only run TM once (site1) and still get all data per site in background. Only possible using Common Threat
		setStopSiteLoop(true);

		FW_TASK("UPTD DPS, 1 \n");
		FW_TASK("UPTD LEV, 1 \n");

	ON_FIRST_INVOCATION_END();

    return;
  }

public:
	void SMC_backgroundProcessing(ARRAY_I sites)
	{

		REGISTER_GLOBAL_INIT();
		for ( int j=0; j < sites.size();j++)
		{
			bool iMan_state			 = registers.getPassFailSMC_CT(CM::I2C,"man_state0","BANDGAP",sites[j]);

			ARRAY_D dBandgap_All  = rdi.site(sites[j]).id( "BandgapMeasured" ).getBurstValue("TEST2");

			SMC_TEST(sites[j],"SDA" , "Bandgap_Operating_Check", tmLimits, (double) iMan_state , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x8", tmLimits, dBandgap_All[1] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x9_0x8", tmLimits, 1000.0 * (dBandgap_All[2] - dBandgap_All[1]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x9", tmLimits, dBandgap_All[2] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xA_0x9", tmLimits, 1000.0 * (dBandgap_All[3] - dBandgap_All[2]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xA", tmLimits, dBandgap_All[3] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xB_0xA", tmLimits, 1000.0 * (dBandgap_All[4] - dBandgap_All[3]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xB", tmLimits, dBandgap_All[4] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xC_0xB", tmLimits, 1000.0 * (dBandgap_All[5] - dBandgap_All[4]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xC", tmLimits, dBandgap_All[5] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xD_0xC", tmLimits, 1000.0 * (dBandgap_All[6] - dBandgap_All[5]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xD", tmLimits, dBandgap_All[6] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xE_0xD", tmLimits, 1000.0 * (dBandgap_All[7] - dBandgap_All[6]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xE", tmLimits, dBandgap_All[7] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0xF_0xE", tmLimits, 1000.0 * (dBandgap_All[8] - dBandgap_All[7]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0xF", tmLimits, dBandgap_All[8] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x0_0xF", tmLimits, 1000.0 * (dBandgap_All[9] - dBandgap_All[8]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x0", tmLimits, dBandgap_All[9] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x1_0x0", tmLimits, 1000.0 * (dBandgap_All[10] - dBandgap_All[9]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x1", tmLimits, dBandgap_All[10] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x2_0x1", tmLimits, 1000.0 * (dBandgap_All[11] - dBandgap_All[10]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x2", tmLimits, dBandgap_All[11] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x3_0x2", tmLimits, 1000.0 * (dBandgap_All[12] - dBandgap_All[11]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x3", tmLimits, dBandgap_All[12] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x4_0x3", tmLimits, 1000.0 * (dBandgap_All[13] - dBandgap_All[12]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x4", tmLimits, dBandgap_All[13] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x5_0x4", tmLimits, 1000.0 * (dBandgap_All[14] - dBandgap_All[13]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x5", tmLimits, dBandgap_All[14] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x6_0x5", tmLimits, 1000.0 * (dBandgap_All[15] - dBandgap_All[14]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x6", tmLimits, dBandgap_All[15] - dBandgap_All[0] /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "Delta_0x7_0x6", tmLimits, 1000.0 * (dBandgap_All[16] - dBandgap_All[15]) /*[mV]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_0x7", tmLimits, dBandgap_All[16] - dBandgap_All[0] /*[V]*/ , TRUE);

			/**
			 * ===================================================================================================
			 * Determine measured bandgap voltage closest to 1.2507V
			 */
			double dDeltaBandgap = abs(1.2507 - (dBandgap_All[1] - dBandgap_All[0]));
			int iVoltageFound = 1;
			for (int i=2; i<17; i++) {
				if (dDeltaBandgap > abs(1.2507 - (dBandgap_All[i] - dBandgap_All[0]))) {
					dDeltaBandgap = abs(1.2507 - (dBandgap_All[i] - dBandgap_All[0]));
					iVoltageFound = i;
				};
			}

			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_BestFit", tmLimits, (dBandgap_All[iVoltageFound] - dBandgap_All[ 0]) /*[V]*/ , TRUE);
			SMC_TEST(sites[j],"TEST2" , "V_bandgap_seq", tmLimits, double(iVoltageFound), TRUE);

			/**
			 * ===================================================================================================
			 * Translate into the valid Trim setting
			 *
			 * Determine iTrim value belonging to the closest value found
			 * The relation between the iVoltageFound and the iTrim to be used is:
			 * iVoltage found	iTrim
			 *  	0			0x0		(can never occur, is offset voltage level wrt ground)
			 * 		1			0x8		Minimum value
			 * 		2			0x9
			 * 		3			0xA
			 * 		4			0xB
			 * 		5			0xC
			 * 		6			0xD
			 * 		7			0xE
			 * 		8			0xF
			 * 		9			0x0		Typical value
			 * 		10			0x1
			 * 		11			0x2
			 * 		12			0x3
			 * 		13			0x4
			 * 		14			0x5
			 * 		15			0x6
			 * 		16			0x7		Maximum value
			 */

			const int iSearchTrim[17] = {0x0,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7}; // So from 0..16
			int iMTP_Bandgap_global = iSearchTrim[iVoltageFound];

			SMC_TEST(sites[j],"TEST2" , "V_Bandgap_Hex", tmLimits, (double) iMTP_Bandgap_global ,TRUE);

			/**
			 * ===================================================================================================
			 * Store unsigned integer into OTP class to enable writing into MTP
			 */

			OtpData OtpBandgap;	// for MTP programming
			if (OtpBandgap.getComp2Flag("Bandgap_calibr_vbg_trim") == 0) { // Unsigned
				OtpBandgap.setCalValueSMC("Bandgap_calibr_vbg_trim",iMTP_Bandgap_global, sites[j]);
			} else { // Signed
				OtpBandgap.setCalValueSMC("Bandgap_calibr_vbg_trim",OtpBandgap.ConvComp2(iMTP_Bandgap_global,OtpBandgap.getBitCount("Bandgap_calibr_vbg_trim")), sites[j]);
			}

		}
	}


  /**
   *This function will be invoked once the specified parameter's value is changed.
   *@param parameterIdentifier
   */
  virtual void postParameterChange(const string& parameterIdentifier)
  {
    //Add your code
    //Note: Test Method API should not be used in this method!
    return;
  }

  /**
   *This function will be invoked once the Select Test Method Dialog is opened.
   */
  virtual const string getComment() const 
  {
    string comment = " please add your comment for this method.";
    return comment;
  }
};
REGISTER_TESTMETHOD("src.Bandgap.Bandgap", Bandgap);
