#include <stdio.h>
#include <math.h>
#include <float.h>

double equation(double KG, double Kd, double Kp, double Trc) {
    double KG2 = KG * KG;
    double Kd2 = Kd * Kd;
    double delta = (Kd2 * Kd2) / KG2 - 4.0 * (Kp * Kd2 / KG2 - 1.0);

    if (delta < 0) {
        return 1e6;
    }

    double arg = 0.5 * ((-Kd2) / KG + sqrt(delta));
    
    if (arg < -1.0) arg = -1.0;
    if (arg > 1.0) arg = 1.0;

    double theta = acos(arg);
    
    double denom = KG * sqrt(1.0 - cos(theta) * cos(theta));
    
    if (fabs(denom) < 1e-12) return 1e6; // Avoid division by zero

    return (Kd * theta / denom) - Trc;
}

double bisection(double low, double high, double Kd, double Kp, double Trc) {
    double mid;
    for (int i = 0; i < 100; i++) { // Max 100 iterations
        mid = (low + high) / 2.0;
        double f_mid = equation(mid, Kd, Kp, Trc);
        
        if (fabs(f_mid) < 1e-7 || (high - low) / 2.0 < 1e-7) {
            return mid;
        }

        if ((equation(low, Kd, Kp, Trc) > 0) == (f_mid > 0)) {
            low = mid;
        } else {
            high = mid;
        }
    }
    return mid;
}

int main() {
    double Kp = 1.0;
    double Kd = 1.0;
    double Trc = 0.5;
    double G = 1.0; // Assuming G is defined somewhere in your MATLAB workspace

    double root_KG = bisection(0.1, 500.0, Kd, Kp, Trc);
    double Kc = root_KG / G;

    printf("Result Kc: %f\n", Kc);
    return 0;
}