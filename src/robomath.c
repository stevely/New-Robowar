/*
 * robomath.c
 *
 * By Steven Smith
 */

#include <math.h>

int square( int n ) {
    return n*n;
}

int robo_atan2_raw( int y, int x ) {
    return (int)(360/(2*M_PI) * atan2(y,x));
}

int robo_atan2( int y, int x ) {
    return (450 + robo_atan2_raw(y,x)) % 360;
}

int robo_sin( int n, int angle ) {
    return (int)((double)n * sin(((double)angle * 2 * M_PI) / 360));
}

int robo_cos( int n, int angle ) {
    return (int)((double)n * -cos(((double)angle * 2 * M_PI) / 360));
}

int robo_tan( int n, int angle ) {
    double result;
    result = ((double)n * tan(90 - ((double)angle * 2 * M_PI) / 360));
    if( result > 19999 ) {
        result = 19999;
    }
    else if( result < -19999 ) {
        result = -19999;
    }
    return (int)result;
}

int robo_asin( int x, int y ) {
    return (int)(asin((((double)x / (double)y) * 2 * M_PI) / 360));
}

int robo_acos( int x, int y ) {
    return (int)(acos((((double)x / (double)y) * 2 * M_PI) / 360));
}
