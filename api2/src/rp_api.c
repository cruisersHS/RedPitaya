
#include "rp_api.h"

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "generate.h"
#include "la_acq.h"

/** SIGNAL ACQUISTION  */

/** Maximal digital signal sampling frequency [Hz] */
const double c_max_dig_sampling_rate = 125e6;

/** Maximal digital signal sampling frequency time interval [nS] */
const double c_max_dig_sampling_rate_time_interval_ns = 4;

rp_handle_uio_t la_acq_handle;
rp_handle_uio_t sig_gen_handle;

/*
uio9: name=scope0, version=devicetree, events=0
        map[0]: addr=0x40090000, size=65536
uio8: name=asg1, version=devicetree, events=0
        map[0]: addr=0x40080000, size=65536
uio7: name=asg0, version=devicetree, events=0
        map[0]: addr=0x40070000, size=65536
uio6: name=pwm, version=devicetree, events=0
        map[0]: addr=0x40060000, size=65536
uio5: name=pdm, version=devicetree, events=0
        map[0]: addr=0x40050000, size=65536
uio4: name=calib, version=devicetree, events=0
        map[0]: addr=0x40040000, size=65536
uio3: name=led, version=devicetree, events=0
        map[0]: addr=0x40030000, size=65536
uio2: name=gpio, version=devicetree, events=0
        map[0]: addr=0x40020000, size=65536
uio13: name=dummy, version=devicetree, events=0
        map[0]: addr=0x83C00000, size=4194304
uio12: name=la, version=devicetree, events=0
        map[0]: addr=0x400C0000, size=65536
uio11: name=lg, version=devicetree, events=0
        map[0]: addr=0x400B0000, size=65536
uio10: name=scope1, version=devicetree, events=0
        map[0]: addr=0x400A0000, size=65536
uio1: name=muxctl, version=devicetree, events=0
        map[0]: addr=0x40010000, size=65536
uio0: name=id, version=devicetree, events=0
        map[0]: addr=0x40000000, size=65536
		*/

/**
 * Open device
 */
RP_STATUS rp_OpenUnit(void)
{
    int r=RP_API_OK;

    if(rp_LaAcqOpen("/dev/uio12", &la_acq_handle)!=RP_API_OK){
        r=-1;
    }

    rp_LaAcqFpgaRegDump(&la_acq_handle);

   // if(rp_GenOpen("/dev/dummy", &sig_gen_handle)!=RP_API_OK){
    if(rp_GenOpen("/dev/uio11", &sig_gen_handle)!=RP_API_OK){
        r=-1;
    }
    return r;
}

/**
 * Close device
 */
RP_STATUS rp_CloseUnit(void)
{
    int r=RP_API_OK;

    if(rp_LaAcqClose(&la_acq_handle)!=RP_API_OK){
        r=-1;
    }

    if(rp_GenClose(&sig_gen_handle)!=RP_API_OK){
        r=-1;
    }
    return r;
}

/**
 * This function retrieves information about the specified device.
 * If the device fails to open or no device is opened, only the driver version is available.
 *
 * @param string         On exit, the information string selected specified by the info argument.
 *                         If string is NULL, only requiredSize is returned.
 * @param stringLength  On entry, the maximum number of int8_t that may be written to string.
 * @param requiredSize  On exit, the required length of the string array.
 * @param info            A number specifying what information is required. The possible values are listed in the table below.
 */
RP_STATUS rp_GetUnitInfo(int8_t * string,
                        int16_t stringLength,
                        int16_t * requiredSize,
                        RP_INFO info)
{


    return RP_API_OK;
}


/**
 * Enable digital port
 *
 * This function is used to enable the digital port and set the logic level (the voltage at
 * which the state transitions from 0 to 1).
 *
 * @param port          Identifies the port for digital data
 * @param enabled          Whether or not to enable the channel.
 * @param logiclevel     The voltage at which the state transitions between 0
 *                         and 1. Range: –32767 (–5 V) to 32767 (5 V).
 *
 */
RP_STATUS rp_SetDigitalPort(RP_DIGITAL_PORT port,
                           int16_t enabled,
                           int16_t logiclevel)
{
    // TODO:
    // RP_DIGITAL_PORT0
    // RP_DIGITAL_PORT1
    return RP_API_OK;
}


/**
 * Enable digital port
 *
 * This function will set the individual digital channels' trigger directions. Each trigger
 * direction consists of a channel name and a direction. If the channel is not included in
 * the array of RP_DIGITAL_CHANNEL_DIRECTIONS the driver assumes the
 * digital channel's trigger direction is RP_DIGITAL_DONT_CARE.
 *
 * @param directions      A pointer to an array of structures describing the
 *                         requested properties.
 *                         If directions is NULL, digital triggering is switched off.
 *                         A digital channel that is not included in the array will be set to RP_DIGITAL_DONT_CARE.
 * @param nDirections      The number of digital channel directions being
 *                      passed to the driver.
 *
 */
RP_STATUS rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            int16_t nDirections)
{
    // disable triggering by default
    rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_ALL_MASK);

    rp_la_trg_regset_t trg;
    memset(&trg,0,sizeof(rp_la_trg_regset_t));

    // none of triggers is enabled
    if(directions==NULL){
        return RP_API_OK;
    }

    uint32_t n;
    uint32_t edge_cnt=0;
    // set trigger pattern settings
    for(n=0; n < nDirections; n++){
        uint32_t mask = (1<<directions[n].channel);
        if(edge_cnt>1){
            // more than one pin is set to rising/falling edge
            return RP_INVALID_PARAMETER;
        }
        switch(directions[n].direction){
            case RP_DIGITAL_DONT_CARE:
                // default value is don't care
                break;
            case RP_DIGITAL_DIRECTION_LOW:
                trg.cmp_msk|=mask;
                // default val is low
                break;
            case RP_DIGITAL_DIRECTION_HIGH:
                trg.cmp_msk|=mask;
                trg.cmp_val|=mask;
                break;
            case RP_DIGITAL_DIRECTION_RISING:
                trg.edg_pos|=mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_FALLING:
                trg.edg_neg|=mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_RISING_OR_FALLING:
                trg.edg_pos|=mask;
                trg.edg_neg|=mask;
                edge_cnt++;
                break;
            default:
                return RP_INVALID_PARAMETER;
                break;
        }
    }

    // update settings
    if(edge_cnt==1){
        rp_LaAcqSetTrigSettings(&la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_LOA_PAT_MASK);//|RP_TRG_LOA_SWE_MASK);
    }

    //TEST!!! this will be removed
    //rp_LaAcqRunAcq(&la_acq_handle);

    rp_LaAcqFpgaRegDump(&la_acq_handle);

    ///
    return RP_API_OK;
}

RP_STATUS rp_IsAcquistionComplete(void){
    int i=0;
    while(i<3){
        sleep(1);
        bool status;
        rp_LaAcqFpgaRegDump(&la_acq_handle);
        rp_LaAcqAcqIsStopped(&la_acq_handle, &status);
        if(status){
            uint32_t trig_addr;
            uint32_t pst_length;
            rp_LaAcqGetCntStatus(&la_acq_handle, &trig_addr, &pst_length);
            printf("\n\r trig_addr=%d pst_length=%d\n\r", trig_addr, pst_length);
            return RP_API_OK;
        }
        else{
            i++;
        }
        //rp_LaAcqTriggerAcq(&la_acq_handle);
    }
    rp_LaAcqFpgaRegDump(&la_acq_handle);
    return RP_TRIGGER_ERROR;
}

/**
 * Enable digital port
 *
 * This function calculates the sampling rate and maximum number of samples for a
 * given timebase under the specified conditions. The result will depend on the number of
 * channels enabled by the last call to rpSetChannel().
 *
 * @param timebase        Timebase factor
 * @param noSamples      The number of samples required
 * @param timeIntervalNanoseconds     On exit, the time interval between
 *                                      readings at the selected timebase.
 * @param maxSamples    On exit, the maximum number of samples
 *                         available. The result may vary depending on the number of channels
 *                         enabled and the timebase chosen.
 *
 */
RP_STATUS rp_GetTimebase(uint32_t timebase,
                        int32_t noSamples,
                        double * timeIntervalNanoseconds,
                        //int16_t oversample,
                        uint32_t * maxSamples
                        //uint32_t segmentIndex
                        )
{
    *timeIntervalNanoseconds=timebase*c_max_dig_sampling_rate_time_interval_ns;
    return RP_API_OK;
};


/**
 * Set data buffer
 *
 * This function tells the driver where to store the data.
 *
 * @param channel    The channel you want to use with the buffer.
 * @param buffer      The location of the buffer
 * @param bufferLth The size of the buffer array
 *
 */
RP_STATUS rp_SetDataBuffer(RP_DIGITAL_PORT channel,
                                 int16_t * buffer,
                                 int32_t bufferLth,
                                // uint32_t segmentIndex,
                                RP_RATIO_MODE mode)
{


    return 0;
}


/**
 *
 * This function tells the driver where to store the data.
 *
 * @param noOfPreTriggerSamples,    The number of samples to return before the trigger event.
 *                                     If no trigger has been set then this argument is ignored and
 *                                     noOfPostTriggerSamples specifies the maximum number of samples to collect.
 * @param noOfPostTriggerSamples      the number of samples to be taken after a trigger event.
 *                                     If no trigger event has been set then this specifies the maximum number of
 *                                     samples to be taken.
 *                                     If a trigger condition has been set, this specifies the number of
 *                                     samples to be taken after a trigger has fired, and the number of samples
 *                                     to be collected is then: noOfPreTriggerSamples + noOfPostTriggerSamples
 * @param timebase                     Timebase
 * @param timeIndisposedS            On exit, the time, in milliseconds, that the scope will spend collecting samples.
 * @param rpReady                     A pointer to the rpBlockReady() callback function that the driver will
 *                                     call when the data has been collected.
 *
 * @param pParameter                A void pointer that is passed to the rpBlockReady() callback function.
 */

RP_STATUS rp_RunBlock(uint32_t noOfPreTriggerSamples,
                     uint32_t noOfPostTriggerSamples,
                     uint32_t timebase,
                    // int16_t oversample,
                     double * timeIndisposedMs,
                     //uint32_t segmentIndex,
                     rpBlockReady rpReady,
                     void * pParameter
)
{
    // reset

    double timeIntervalNanoseconds;
    uint32_t maxSamples;
    RP_STATUS status = rp_GetTimebase(timebase,0,&timeIntervalNanoseconds,&maxSamples);
    if(status!=RP_API_OK) return status;

    if((noOfPreTriggerSamples+noOfPostTriggerSamples)>maxSamples){
        return RP_INVALID_PARAMETER;
    }

    *timeIndisposedMs=(noOfPreTriggerSamples+noOfPostTriggerSamples)*timeIntervalNanoseconds/10e6;

    // configure FPGA to start block mode
   // rp_la_decimation_regset_t dec;
   // dec.dec=timebase;
   // rp_LaAcqSetDecimation(&la_acq_handle, dec);

    rp_la_cfg_regset_t cfg;
    cfg.pre=noOfPreTriggerSamples;
    cfg.pst=noOfPostTriggerSamples;
    rp_LaAcqSetCntConfig(&la_acq_handle, cfg);

    printf("\r\nrp_LaAcqRunAcq");
    rp_LaAcqRunAcq(&la_acq_handle);

    rp_LaAcqBlockingRead(&la_acq_handle);

    // how to check if acq. was triggered?
    if(rp_IsAcquistionComplete()!=RP_API_OK){
        return RP_OPERATION_FAILED;
    }

    // acquisition is completed -> callback
    (*rpReady)(status,pParameter);

    printf("\r\nrp_LaAcqStopAcq");
    //rp_LaAcqStopAcq(&la_acq_handle);

    return RP_API_OK;
}


/**
 *
 * Start collecting data in streaming mode.
 * When data has been collected from the device it is down-sampled if necessary and then
 * delivered to the user. Call rpGetStreamingLatestValues() to retrieve the data.
 *
 * When a trigger is set, the total number of samples stored in the driver is the sum of maxPreTriggerSamples and maxPostTriggerSamples.
 *
 * @param sampleInterval             On entry, the requested time interval between samples;
 *                                     on exit, the actual time interval used.
 * @param sampleIntervalTimeUnits   The unit of time used for sampleInterval.
 *
 * @param maxPreTriggerSamples        The maximum number of raw samples before a trigger event for each enabled channel.
 *                                     If no trigger has been set then this argument is ignored and
 *                                     maxPostTriggerSamples specifies the maximum number of samples to collect.
 *
 * @param maxPostTriggerSamples     The maximum number of raw samples after a trigger event for each enabled channel.
 *                                     If no trigger condition is set, this argument states the maximum number of samples to be stored.
 *
 * @param autoStop                     A flag that specifies if the streaming should stop when all of maxSamples have been captured.
 *
 * @param downSampleRatio             See rpGetValues()
 *
 * @param downSampleRatioMode         See rpGetValues()
 *
 * @param overviewBufferSize         The size of the overview buffers. These are temporary buffers used for storing the data
 *                                     before returning it to the application.
 *                                     The size is the same as the bufferLth value passed to rpSetDataBuffer().
 */
RP_STATUS rp_RunStreaming(uint32_t * sampleInterval,
                        RP_TIME_UNITS sampleIntervalTimeUnits,
                        uint32_t maxPreTriggerSamples,
                        uint32_t maxPostTriggerSamples,
                        int16_t autoStop,
                        uint32_t downSampleRatio,
                        RP_RATIO_MODE downSampleRatioMode,
                        uint32_t overviewBufferSize)
{

    // configure FPGA to start block mode
    return RP_API_OK;
};


/**
 * This function returns block-mode data, with or without down-sampling, starting at the
 * specified sample number. It is used to get the stored data from the driver after data
 * collection has stopped.
 *
 * This function tells the driver where to store the data.
 *
 * @param startIndex        A zero-based index that indicates the start point for data collection.
 *                             It is measured in sample intervals from the start of the buffer.
 * @param noOfSamples          On entry, the number of samples required. On exit, the actual number retrieved.
 *                          The number of samples retrieved will not be more than the number requested,
 *                          and the data retrieved starts at startIndex.
 * @param downSampleRatio     The down-sampling factor that will be applied to the raw data.
 * @param downSampleRatioMode Which down-sampling mode to use.
 * @param overflow             On exit, a set of flags that indicate whether an over-voltage has occurred
 *                             on any of the channels. It is a bit field with bit 0 denoting Channel A.
 *
 */
RP_STATUS rp_GetValues(uint32_t startIndex,
                      uint32_t * noOfSamples,
                      uint32_t downSampleRatio,
                      RP_RATIO_MODE downSampleRatioMode,
                      //uint32_t segmentIndex,
                      int16_t * overflow){


    return RP_API_OK;
};

/**
 * This function returns data either with or without down-sampling, starting at the
 * specified sample number. It is used to get the stored data from the scope after data
 * collection has stopped. It returns the data using a callback.
 *
 * @param startIndex             See rpGetValues()
 * @param noOfSamples             See rpGetValues()
 * @param downSampleRatio         See rpGetValues()
 * @param downSampleRatioMode     See rpGetValues()
 * @param lpDataReady             A pointer to the user-supplied function that will be called when the data is ready.
 *                                 This will be rpDataReady() for block-mode data or ps3000aStreamingReady() for streaming mode data.
 * @param pParameter             A void pointer that will be passed to the callback function.
 *                                 the data type is determined by the application.
 */
RP_STATUS rp_GetValuesAsync(
    uint32_t startIndex,
    uint32_t noOfSamples,
    uint32_t downSampleRatio,
    RP_RATIO_MODE downSampleRatioMode,
    //uint32_t segmentIndex
    void * lpDataReady,
    void * pParameter)
{

    return RP_API_OK;
}

/**
 * Set data buffer
 *
 * This function instructs the driver to return the next block of values to your
 * rpStreamingReady() callback. You must have previously called
 * rpRunStreaming() beforehand to set up streaming.
 *
 * @param rpReady          A pointer to your rpStreamingReady() callback.
 * @param pParameter    A void pointer that will be passed to the rpStreamingReady() callback.
 *                         The callback may optionally use this pointer to return information to the application.
 *
 */
RP_STATUS rp_GetStreamingLatestValues(rpStreamingReady rpReady,
                                     void * pParameter)
{
    // block read
    int32_t noOfSamples;
    uint32_t startIndex;
    int16_t overflow;
    uint32_t triggerAt;
    int16_t triggered;
    int16_t autoStop;

    noOfSamples=0;
    startIndex=0;
    overflow=0;
    triggerAt=0;
    triggered=0;
    autoStop=0;

    // acquisition is completed -> callback
    (*rpReady)(noOfSamples,
               startIndex,
               overflow,
               triggerAt,
               triggered,
               autoStop,
               pParameter);

    return RP_API_OK;
}

/**
 * Stops the scope device from sampling data. If this function is called
 * before a trigger event occurs, the oscilloscope may not contain valid data.
 * Always call this function after the end of a capture to ensure that the scope is ready for the next capture.
 */


RP_STATUS rp_Stop(void){
    return rp_LaAcqStopAcq(&la_acq_handle);
}

/** SIGNAL GENERATION  */
/**
 * This function causes a trigger event, or starts and stops gating.
 * It is used when the signal generator is set to SIGGEN_SOFT_TRIG.
 *
 * @param state, sets the trigger gate high or low when the trigger type is
 * set to either SIGGEN_GATE_HIGH or SIGGEN_GATE_LOW. Ignored for other trigger types.
 */

RP_STATUS rp_SigGenSoftwareControl(int16_t state){
    rp_GenTrigger(&sig_gen_handle);
    return RP_API_OK;
}

/**
 * This function sets up the signal generator to produce a signal from a list of built-in
 * waveforms. If different start and stop frequencies are specified, the device will sweep
 * either up, down or up and down.
 *
 * @param offsetVoltage     The voltage offset, in microvolts, to be applied to the waveform
 * @param pkToPk             The peak-to-peak voltage, in microvolts, of the waveform signal.
 *                             Note that if the signal voltages described by the combination of offsetVoltage and pkToPk
 *                             extend outside the voltage range of the signal generator, the output waveform will be clipped.
 * @param waveType            The type of waveform to be generated.
 * @param startFrequency    The frequency that the signal generator will initially produce.
 *                             For allowable values see RP_SINE_MAX_FREQUENCY and related values.
 * @param stopFrequency        The frequency at which the sweep reverses direction or returns to the initial frequency.
 * @param increment            The amount of frequency increase or decrease in sweep mode.
 * @param dwellTime            The time for which the sweep stays at each frequency in seconds.
 * @param sweepType         Whether the frequency will sweep from startFrequency to stopFrequency, or in the opposite direction,
 *                             or repeatedly reverse direction.
 * @param operation            The type of extra waveform to be produced.
 * @param shots                0: Sweep the frequency as specified by sweeps
 *                             1...RP_MAX_SWEEPS_SHOTS: the number of cycles of the waveform to be produced after a trigger event.
 *                             Sweeps must be zero.
 *                             RP_SHOT_SWEEP_TRIGGER_CONTINUOUS_RUN: start and run continuously after trigger occurs
 * @param sweeps            0: produce number of cycles specified by shots
 *                             1..RP_MAX_SWEEPS_SHOTS: the number of times to sweep the frequency after a trigger event, according to sweepType.
 *                             shots must be zero.
 *                             RP_SHOT_SWEEP_TRIGGER_CONTINUOUS_RUN: start a sweep and continue after trigger occurs.
 * @param triggerType        The type of trigger that will be applied to the signal generator.
 * @param triggerSource        The source that will trigger the signal generator
 * @param extInThreshold    Used to set trigger level for external trigger.
 */

RP_STATUS rp_SetSigGenBuiltIn(int32_t offsetVoltage,
                             uint32_t pkToPk,
                             RP_WAVE_TYPE waveType,
                             float startFrequency,
                             float stopFrequency,
                             float increment,
                             float dwellTime,
                             RP_SWEEP_TYPE sweepType,
                             RP_EXTRA_OPERATIONS operation,
                             uint32_t shots,
                             uint32_t sweeps,
                             RP_SIGGEN_TRIG_TYPE triggerType,
                             RP_SIGGEN_TRIG_SOURCE triggerSource,
                             int16_t extInThreshold){

    //rp_GenSetAmp(&sig_gen_handle,1.0);
    //rp_GenSetOffset(&sig_gen_handle, 1.0);


    // triggerType
    switch(triggerSource){
        case RP_SIGGEN_NONE:
            break;
        case RP_SIGGEN_SCOPE_TRIG:
            break;
        case RP_SIGGEN_EXT_IN:
            break;
        case RP_SIGGEN_SOFT_TRIG:
            break;
        case RP_SIGGEN_TRIGGER_RAW:
            break;
    }

    switch(waveType){
        case RP_SG_SINE:
            break;
        case RP_SG_SQUARE:
            break;
        case RP_SG_TRIANGLE:
            break;
        case RP_SG_DC_VOLTAGE:
            break;
        case RP_SG_RAMP_UP:
            break;
        case RP_SG_RAMP_DOWN:
            break;
        case RP_SG_SINC:
            break;
        case RP_SG_GAUSSIAN:
            break;
        case PR_SG_HALF_SINE:
            break;
    }

    return RP_API_OK;
}

/** DIGITAL SIGNAL GENERATION  */

RP_STATUS rp_DigSigGenOuput(bool enable)
{
    if(enable){
        rp_GenOutputEnable(&sig_gen_handle, RP_GEN_OUT_PORT0_MASK);
    }
    else{
        rp_GenOutputDisable(&sig_gen_handle, RP_GEN_OUT_PORT0_MASK);
    }
    return RP_API_OK;
}

RP_STATUS rp_DigSigGenSoftwareControl(int16_t state)
{
	rp_GenTrigger(&sig_gen_handle);
   // rp_GenFpgaRegDump(&sig_gen_handle,0);
    return RP_API_OK;
}

RP_STATUS rp_SetDigSigGenBuiltIn(RP_DIG_SIGGEN_PAT_TYPE patternType,
                                double * sample_rate,
                                uint32_t shots,
                                uint32_t delay_between_shots,
                                uint32_t triggerSourceMask)
{
    rp_GenStop(&sig_gen_handle);

    // set burst mode - dig. sig. gen will always operate in this mode!
    rp_GenSetMode(&sig_gen_handle, RP_GEN_MODE_BURST);

    // set waveform
    uint32_t len = 256;
    switch(patternType){
        case RP_DIG_SIGGEN_PAT_UP_COUNT_8BIT_SEQ_256:
            rp_GenSetWaveformUpCountSeq(&sig_gen_handle,len);
            rp_GenSetBurstModeDataLen(&sig_gen_handle,len);
            rp_GenSetBurstModePeriodLen(&sig_gen_handle,len);
            break;
    }

    // repetitions
    rp_GenSetBurstModeRepetitions(&sig_gen_handle, shots);

    // no idle
    // rp_GenSetBurstModeIdle(&sig_gen_handle, delay_between_shots);

    // sample rate
    rp_GenSetWaveformSampleRate(&sig_gen_handle,sample_rate);

    // trigger
    rp_GenGlobalTrigSet(&sig_gen_handle, triggerSourceMask);

    rp_GenRun(&sig_gen_handle);

    rp_GenFpgaRegDump(&sig_gen_handle,len);

    return RP_API_OK;
}

// TODO: add function that will generate protocol from file
