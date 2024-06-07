#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(F(x))
#define debug_(x) Serial.print(x)
#define debugln(x) Serial.println(F(x))
#define debugln_(x) Serial.println(x)
#define debugf(x,y) Serial.printf(x,y)
#else
#define debug(x)
#define debugln(x)
#define debug_(x)
#define debugln_(x)
#define debugf(x,y)
#endif