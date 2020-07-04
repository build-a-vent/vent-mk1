#ifndef VENT_SFE_FAKE_SENSOR_H
#define VENT_SFE_FAKE_SENSOR_H

class SFE_BMP180 {
  public:
    inline int32_t startTemperature(void)                    { return 100; } // millisecs to process
    inline int32_t startPressure(int i)                      { return 20; } // millisecs to process
    inline int32_t getError(void)                            { return 0; } // returncode
    inline int32_t getTemperature(double &dTemp)             { dTemp=20.0; return 1; }
    inline int32_t getPressure(double &dP, double &dTemp)    { dP=1013.0; return 1; }
    inline int begin(void)                                   { return 1; }
};


#endif // VENT_SFE_FAKE_SENSOR_H
